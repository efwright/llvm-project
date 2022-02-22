//===----RTLs/cuda/src/rtl.cpp - Target RTLs Implementation ------- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// RTL for CUDA machine
//
//===----------------------------------------------------------------------===//

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cuda.h>
#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Debug.h"
#include "DeviceEnvironment.h"
#include "omptargetplugin.h"

#define TARGET_NAME CUDA
#define DEBUG_PREFIX "Target " GETNAME(TARGET_NAME) " RTL"

#include "MemoryManager.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/CodeGen/CommandFlags.h"
#include "llvm/Frontend/OpenMP/OMPConstants.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/LTO/legacy/LTOCodeGenerator.h"
#include "llvm/LTO/legacy/LTOModule.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Transforms/Utils/Cloning.h"

using llvm::cast;
using llvm::ConstantInt;
using llvm::dyn_cast;
using llvm::Function;
using llvm::GlobalVariable;
using llvm::LLVMContext;
using llvm::LTOModule;
using llvm::MemoryBuffer;
using llvm::Type;
using llvm::Value;

// Utility for retrieving and printing CUDA error string.
#ifdef OMPTARGET_DEBUG
#define CUDA_ERR_STRING(err)                                                   \
  do {                                                                         \
    if (getDebugLevel() > 0) {                                                 \
      const char *errStr = nullptr;                                            \
      CUresult errStr_status = cuGetErrorString(err, &errStr);                 \
      if (errStr_status == CUDA_ERROR_INVALID_VALUE)                           \
        REPORT("Unrecognized CUDA error code: %d\n", err);                     \
      else if (errStr_status == CUDA_SUCCESS)                                  \
        REPORT("CUDA error is: %s\n", errStr);                                 \
      else {                                                                   \
        REPORT("Unresolved CUDA error code: %d\n", err);                       \
        REPORT("Unsuccessful cuGetErrorString return status: %d\n",            \
               errStr_status);                                                 \
      }                                                                        \
    } else {                                                                   \
      const char *errStr = nullptr;                                            \
      CUresult errStr_status = cuGetErrorString(err, &errStr);                 \
      if (errStr_status == CUDA_SUCCESS)                                       \
        REPORT("%s \n", errStr);                                               \
    }                                                                          \
  } while (false)
#else // OMPTARGET_DEBUG
#define CUDA_ERR_STRING(err)                                                   \
  do {                                                                         \
    const char *errStr = nullptr;                                              \
    CUresult errStr_status = cuGetErrorString(err, &errStr);                   \
    if (errStr_status == CUDA_SUCCESS)                                         \
      REPORT("%s \n", errStr);                                                 \
  } while (false)
#endif // OMPTARGET_DEBUG

#define BOOL2TEXT(b) ((b) ? "Yes" : "No")

#include "elf_common.h"

static llvm::codegen::RegisterCodeGenFlags CGF;

/// Keep entries table per device.
struct FuncOrGblEntryTy {
  __tgt_target_table Table;
  std::vector<__tgt_offload_entry> Entries;
};

/// Use a single entity to encode a kernel and a set of flags.
struct KernelTy {
  CUfunction Func;

  // execution mode of kernel
  llvm::omp::OMPTgtExecModeFlags ExecutionMode;

  /// Maximal number of threads per block for this kernel.
  int MaxThreadsPerBlock = 0;

  KernelTy(CUfunction _Func, llvm::omp::OMPTgtExecModeFlags _ExecutionMode)
      : Func(_Func), ExecutionMode(_ExecutionMode) {}
};

namespace jit {
///
llvm::TargetOptions Options;

/// Init flag to call JIT::init().
std::once_flag InitFlag;

void init() {
  // Initialize the configured targets.
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmPrinters();
  llvm::InitializeAllAsmParsers();

  Options =
      llvm::codegen::InitTargetOptionsFromCodeGenFlags(llvm::Triple("nvptx64"));
}

class LLVMContextCache {
  std::mutex Mutex;
  std::unordered_map<std::thread::id, std::unique_ptr<LLVMContext>> Map;

public:
  LLVMContext &get() {
    auto TId = std::this_thread::get_id();
    std::lock_guard<std::mutex> LG(Mutex);
    auto Itr = Map.find(TId);
    if (Itr != Map.end())
      return *Itr->second;

    Map[TId] = std::make_unique<LLVMContext>();
    return *Map[TId];
  }
} ContextMap;

class LTOModuleCache {
  std::mutex Mutex;
  std::unordered_map<void *,
                     std::unordered_map<void *, std::unique_ptr<LTOModule>>>
      Map;

  LTOModule *getImpl(LLVMContext &Context, __tgt_device_image *Image) {
    auto I1 = Map.find(&Context);
    if (I1 == Map.end())
      return nullptr;

    auto I2 = I1->second.find(Image->ImageStart);
    if (I2 == I1->second.end())
      return nullptr;

    return I2->second.get();
  }

public:
  LTOModule *get(LLVMContext &Context, __tgt_device_image *Image) {
    std::lock_guard<std::mutex> LG(Mutex);
    return getImpl(Context, Image);
  }

  LTOModule *insert(LLVMContext &Context, __tgt_device_image *Image,
                    std::unique_ptr<LTOModule> M) {
    std::lock_guard<std::mutex> LG(Mutex);

    if (auto LM = getImpl(Context, Image))
      return LM;

    auto &L = Map[&Context];
    L[Image->ImageStart] = std::move(M);

    return L[Image->ImageStart].get();
  }
} ModuleCache;

std::unique_ptr<LTOModule> createFromImage(LLVMContext &Context,
                                           __tgt_device_image *Image) {
  auto *LM = ModuleCache.get(Context, Image);

  if (!LM) {
    ptrdiff_t ImageSize = (char *)Image->ImageEnd - (char *)Image->ImageStart;
    auto ErrorOrModule = LTOModule::createFromBuffer(Context, Image->ImageStart,
                                                     ImageSize, jit::Options);
    if (std::error_code Error = ErrorOrModule.getError())
      return nullptr;

    LM = ModuleCache.insert(Context, Image, std::move(*ErrorOrModule));
  }

  auto ErrorOrModule = LTOModule::clone(*LM, jit::Options);
  if (std::error_code Error = ErrorOrModule.getError())
    return nullptr;

  return std::move(*ErrorOrModule);
}

bool isSPMDMode(llvm::Module &M, llvm::StringRef KernelName) {
  bool IsSPMDMode = false;
  GlobalVariable *ExecMode =
      M.getGlobalVariable(std::string(KernelName) + "_exec_mode");
  if (!ExecMode)
    return false;

  assert(llvm::isa<ConstantInt>(ExecMode->getInitializer()) &&
         "ExecMode is not an integer!");

  const int8_t ExecModeVal =
      llvm::cast<ConstantInt>(ExecMode->getInitializer())->getSExtValue();
  if (ExecModeVal & llvm::omp::OMP_TGT_EXEC_MODE_SPMD)
    IsSPMDMode = true;

  return IsSPMDMode;
}

class Kernel {
  /// Kernel entry name.
  std::string Name;
  /// Number of teams.
  uint32_t NumTeams = 0;
  /// Number of threads.
  uint32_t NumThreads = 0;
  /// Number of arguments.
  uint32_t NumArgs = 0;
  /// Kernel arguments.
  std::vector<uintptr_t> ArgumentsBuffer;
  /// Kernel argument mask used to hash the kernel.
  std::vector<uintptr_t> Mask;
  /// Pointer to the kernel arguments. This is for lazy copy.
  uintptr_t *Arguments = nullptr;

  ///
  bool match(const Kernel &K) const {
    assert(Arguments == ArgumentsBuffer.data() && "broken kernel");

    assert(Mask.size() == ArgumentsBuffer.size() && "broken kernel");

    if (NumArgs != K.NumArgs)
      return false;

    if (NumTeams != K.NumTeams || NumThreads != K.NumThreads)
      return false;

    for (int I = 0; I < NumArgs; ++I)
      if ((Arguments[I] & Mask[I]) != (K.Arguments[I] & Mask[I]))
        return false;

    return true;
  }

public:
  Kernel() = default;

  explicit Kernel(const std::string &Name) : Name(Name) {}

  Kernel(const Kernel &K)
      : Name(K.Name), NumTeams(K.NumTeams), NumThreads(K.NumThreads),
        NumArgs(K.NumArgs), ArgumentsBuffer(K.ArgumentsBuffer), Mask(K.Mask),
        Arguments(K.Arguments) {
    if (K.Arguments == K.ArgumentsBuffer.data())
      Arguments = ArgumentsBuffer.data();
  }

  const std::string &getName() const { return Name; }

  ///
  void setArgs(void **Args, size_t Num) {
    NumArgs = Num;
    Arguments = (uintptr_t *)Args;
  }

  ///
  void setMask(const std::vector<uintptr_t> &M) {
    Mask.resize(M.size());
    std::copy(M.cbegin(), M.cend(), Mask.begin());
    if (Arguments != ArgumentsBuffer.data()) {
      ArgumentsBuffer.resize(NumArgs);
      std::copy(Arguments, Arguments + NumArgs, ArgumentsBuffer.begin());
      Arguments = ArgumentsBuffer.data();
    }
  }

  ///
  void setMask(const Kernel &K) {
    assert(NumArgs == K.NumArgs &&
           "try to set mask from an incompatible kernel descriptor");
    assert(K.NumArgs == K.Mask.size() && "broken kernel descriptor");
    setMask(K.Mask);
  }

  ///
  void setNumTeams(uint32_t Val) { NumTeams = Val; }

  ///
  int32_t getNumTeams() const { return NumTeams; }

  ///
  void setNumThreads(uint32_t Val) { NumThreads = Val; }

  ///
  int32_t getNumThreads() const { return NumThreads; }

  ///
  bool operator==(const Kernel &RHS) const { return match(RHS); }

  size_t size() const {
    size_t Size = 0;
    // Name
    Size += Name.size() + 1;
    // NumTeams
    Size += sizeof(uint32_t);
    // NumThreads
    Size += sizeof(uint32_t);
    // NumArgs
    Size += sizeof(uint32_t);
    // ArgumentsBuffer
    Size += NumArgs * sizeof(uintptr_t);
    // Mask
    Size += NumArgs * sizeof(uintptr_t);
    return Size;
  }

  ///
  ptrdiff_t materialize(const char *Buffer) {
    auto P = Buffer;

    Name = std::string(P);
    P += Name.size() + 1;

    NumTeams = *reinterpret_cast<const uint32_t *>(P);
    P += sizeof(uint32_t);

    NumThreads = *reinterpret_cast<const uint32_t *>(P);
    P += sizeof(uint32_t);

    NumArgs = *reinterpret_cast<const uint32_t *>(P);
    P += sizeof(uint32_t);

    ArgumentsBuffer.resize(NumArgs);
    Arguments = ArgumentsBuffer.data();
    Mask.resize(NumArgs);

    for (int I = 0; I < NumArgs; ++I) {
      ArgumentsBuffer[I] = *reinterpret_cast<const uintptr_t *>(P);
      P += sizeof(uintptr_t);
    }

    for (int I = 0; I < NumArgs; ++I) {
      Mask[I] = *reinterpret_cast<const uintptr_t *>(P);
      P += sizeof(uintptr_t);
    }

    return P - Buffer;
  }

  ///
  void serialize(char *Buffer) const {
    auto P = Buffer;

    // Name
    std::copy(Name.begin(), Name.end(), P);
    P += Name.size();
    *P = 0;
    P += 1;

    // NumTeams
    *reinterpret_cast<int32_t *>(P) = NumTeams;
    P += sizeof(int32_t);

    // NumThreads
    *reinterpret_cast<int32_t *>(P) = NumThreads;
    P += sizeof(int32_t);

    // NumArgs
    *reinterpret_cast<int64_t *>(P) = NumArgs;
    P += sizeof(int32_t);

    // ArgumentsBuffer
    for (int I = 0; I < NumArgs; ++I) {
      *reinterpret_cast<uintptr_t *>(P) = ArgumentsBuffer[I];
      P += sizeof(uintptr_t);
    }

    // Mask
    for (int I = 0; I < NumArgs; ++I) {
      *reinterpret_cast<uintptr_t *>(P) = Mask[I];
      P += sizeof(uintptr_t);
    }

    assert(P == Buffer + size());
  }
};

class Image {
  Kernel K;
  ///
  const char *Start = nullptr;
  ///
  const char *End = nullptr;

public:
  Image() = default;

  Image(const Kernel &K, const char *ImageStart, const char *ImageEnd)
      : K(K), Start(ImageStart), End(ImageEnd) {}

  Image(const char *Buffer) {
    auto Offset = K.materialize(Buffer);
    auto P = Buffer + Offset;
    auto ImageSize = *reinterpret_cast<const int64_t *>(P);
    P += sizeof(int64_t);
    Start = P;
    End = P + ImageSize;
  }

  ///
  const Kernel &getKernel() const { return K; }

  ///
  std::pair<void *, void *> get() const {
    return std::make_pair((void *)Start, (void *)End);
  }

  ///
  bool match(const Kernel &K) const { return this->K == K; }

  ///
  size_t size() const {
    size_t Size = 0;
    // Kernel
    Size += K.size();
    // ImageSize
    Size += sizeof(int64_t);
    // Image
    Size += End - Start + 1;
    return Size;
  }

  ///
  void serialize(char *Buffer) const {
    auto P = Buffer;
    K.serialize(P);
    P += K.size();
    // ImageSize
    *reinterpret_cast<int64_t *>(P) = End - Start;
    P += sizeof(int64_t);
    // Image
    std::copy(Start, End, P);
    P += End - Start;
    *P = '\0';
    P += 1;
    assert(P == Buffer + size());
  }
};

/// An offline image cache that can store images accross execution.
class ImageCacheTy {
public:
  ImageCacheTy() {
    {
      std::string FileName{CacheFileName};
      if (const char *Env = getenv("LIBOMPTARGET_JIT_CUDA_CACHE"))
        FileName = std::string{Env};
      std::ifstream In(CacheFileName, std::ios::binary);
      if (!In)
        return;
      In.unsetf(std::ios::skipws);
      std::istream_iterator<char> Start(In), End;
      std::vector<char> Buffer(Start, End);
      OfflineBuffer = std::move(Buffer);
    }

    if (OfflineBuffer.empty())
      return;

    const char *P = OfflineBuffer.data();

    std::string TargetName(P);
    P += TargetName.size() + 1;

    assert(TargetName == "nvptx64" && "target name does not match!");

    auto NumKernels = *reinterpret_cast<const int32_t *>(P);
    P += sizeof(int32_t);

    for (int I = 0; I < NumKernels; ++I) {
      std::string Key(P);
      P += Key.size() + 1;

      auto &L = Map[Key];

      auto NumImages = *reinterpret_cast<const int32_t *>(P);
      P += sizeof(NumImages);

      for (int J = 0; J < NumImages; ++J) {
        L.emplace_back(P);
        P += L.back().size();
      }
    }
  }

  ~ImageCacheTy() {
    std::ofstream Out(CacheFileName, std::ios::binary);
    if (!Out)
      return;

    // Target name
    Out << "nvptx64" << '\0';
    // Number of SMs
    int32_t NumKernels = (int32_t)Map.size();
    Out.write((const char *)&NumKernels, sizeof(NumKernels));

    for (auto &P : Map) {
      // Key
      Out << P.first << '\0';

      int32_t NumImages = (int32_t)P.second.size();
      Out.write((const char *)&NumImages, sizeof(NumImages));

      for (auto &I : P.second) {
        std::vector<char> Buffer(I.size());
        I.serialize(Buffer.data());
        Out.write(Buffer.data(), I.size());
      }
    }
  }

  ///
  const Image *insert(const std::string &Key, Kernel &K,
                      std::unique_ptr<MemoryBuffer> MB) {
    if (auto *I = get(Key, K))
      return I;

    const char *ImageStart = MB->getBufferStart();
    const char *ImageEnd = MB->getBufferEnd();

    NewBuffer.push_back(std::move(MB));

    auto &Images = Map[Key];
    Images.emplace_back(K, ImageStart, ImageEnd);

    return &Images.back();
  }

  ///
  const Image *get(const std::string &Key, const Kernel &K) const {
    auto Itr = Map.find(Key);
    if (Itr == Map.end())
      return nullptr;

    auto &L = Itr->second;
    for (auto &I : L)
      if (I.match(K))
        return &I;

    return nullptr;
  }

private:
  ///
  std::vector<char> OfflineBuffer;
  ///
  std::list<std::unique_ptr<MemoryBuffer>> NewBuffer;
  ///
  std::unordered_map<std::string, std::list<Image>> Map;

  ///
  static const char *CacheFileName;
};

const char *ImageCacheTy::CacheFileName = "libomptarget.jit.cuda.cache";

ImageCacheTy ImageCache;

class TargetTable {
  Kernel K;
  __tgt_target_table *Table;

public:
  TargetTable(const Kernel &K, __tgt_target_table *Table)
      : K(K), Table(Table) {}

  bool match(const Kernel &K) const { return this->K == K; }

  const __tgt_target_table *get() const { return Table; }
};

class TargetTableCache {
  ///
  std::unordered_map<std::string, std::list<TargetTable>> Map;

public:
  const __tgt_target_table *insert(const Kernel &K, __tgt_target_table *Table) {
    if (auto *T = get(K))
      return T;

    auto &Tables = Map[K.getName()];
    Tables.emplace_back(K, Table);

    return Tables.back().get();
  }

  const __tgt_target_table *get(const Kernel &K) const {
    auto Itr = Map.find(K.getName());
    if (Itr == Map.end())
      return nullptr;

    auto &L = Itr->second;
    for (auto &T : L)
      if (T.match(K))
        return T.get();

    return nullptr;
  }
};

struct ActionTy {
  enum ActionKind : uint8_t {
    None = 0,
    Alignment,
    Specialization,
    NumTeams,
    NumThreads,
  };

  std::string to_string() const {
    std::string S;
    // Kind
    S += std::to_string(Kind);
    S += ":";
    // Index if any
    if (Index != -1)
      S += std::to_string(Index);
    S += ":";
    if (HasValue)
      S += std::to_string(Value);
    return S;
  }

  friend void applyActionsToModule(LLVMContext &Context, llvm::Module &M,
                                   Function &Kernel,
                                   std::vector<ActionTy> &Actions,
                                   std::vector<uint64_t> &Mask);

  // ActionTy(ActionKind K) : Kind(K) {}

  // ActionTy(ActionKind K, int Idx) : Kind(K), Index(Idx) {}

  ActionTy(ActionKind K, uint64_t V) : Kind(K), HasValue(true), Value(V) {}

  ActionTy(ActionKind K, int Idx, uint64_t V)
      : Kind(K), Index(Idx), HasValue(true), Value(V) {}

  static std::string serialize(std::vector<ActionTy> &Actions);

private:
  /// Kind of the action.
  ActionKind Kind = None;
  /// Index of the kernel argument. If -1, the action should be to the kernel
  /// entry.
  int Index = -1;
  /// If the action requires value.
  bool HasValue = false;
  /// Corresponding value.
  uint64_t Value = 0;
  std::vector<ActionTy> SubActions;
};

std::string ActionTy::serialize(std::vector<ActionTy> &Actions) {
  std::string S;
  for (unsigned I = 0; I < Actions.size(); ++I) {
    S += Actions[I].to_string();
    if (I != S.size() - 1)
      S.push_back('-');
  }
  return S;
}

void applyActionsToModule(LLVMContext &Context, llvm::Module &M,
                          Function &Kernel, std::vector<ActionTy> &Actions,
                          std::vector<uintptr_t> &Mask) {
  for (auto &A : Actions) {
    switch (A.Kind) {
    case ActionTy::Alignment: {
      assert(A.Index != -1 && "invalid alignment action with index = -1!");
      assert(A.HasValue && "invalid alignment action with no value!");
      DP("[jit] set alignment of arg[%d] to %lu.\n", A.Index, A.Value);
      auto *Arg = Kernel.getArg(A.Index);
      Arg->addAttr(
          llvm::Attribute::get(Context, llvm::Attribute::Alignment, A.Value));
      Arg->addAttr(llvm::Attribute::get(Context, llvm::Attribute::NoUndef));
      Mask[A.Index] = A.Value - 1;
      break;
    }
    case ActionTy::Specialization: {
      assert(A.Index != -1 && "invalid specialization action with index=-1!");
      assert(A.HasValue && "invalid alignment action with no value!");
      DP("[jit] specialize arg[%d] with %lu.\n", A.Index, A.Value);
      auto *C = ConstantInt::get(Type::getInt64Ty(Context), A.Value);
      auto *Arg = Kernel.getArg(A.Index);
      Arg->replaceAllUsesWith(C);
      Mask[A.Index] = A.Value;
      break;
    }
    case ActionTy::NumTeams: {
      assert(A.Index == -1 && "invalid # teams action with index != -1!");
      assert(A.HasValue && "invalid alignment action with no value!");
      DP("[jit] add omp_target_num_teams = %lu to kernel attrs\n", A.Value);
      Kernel.addFnAttr("omp_target_num_teams", std::to_string(A.Value));
      // NOTE: WA
      auto *F = M.getFunction("llvm.nvvm.read.ptx.sreg.nctaid.x");
      if (F) {
        auto *NumTeamsVal =
            ConstantInt::get(Type::getInt32Ty(Context), A.Value);
        for (auto *U : F->users())
          if (auto *CI = dyn_cast<llvm::CallInst>(U))
            CI->replaceAllUsesWith(NumTeamsVal);
      }
      // We don't store the number here because we want to use user's value for
      // mapping. The logic is, if other settings, except user's # teams and #
      // threads, are same, the actual numbers must be same as long as user's
      // numbers are same.
      break;
    }
    case ActionTy::NumThreads: {
      assert(A.Index == -1 && "invalid # threads action with index != -1!");
      assert(A.HasValue && "invalid alignment action with no value!");
      DP("[jit] add omp_target_thread_limit = %lu to kernel attrs\n", A.Value);
      Kernel.addFnAttr("omp_target_thread_limit", std::to_string(A.Value));
      // NOTE: WA
      auto *F = M.getFunction("llvm.nvvm.read.ptx.sreg.ntid.x");
      if (F) {
        auto *NumTeamsVal =
            ConstantInt::get(Type::getInt32Ty(Context), A.Value);
        for (auto *U : F->users())
          if (auto *CI = dyn_cast<llvm::CallInst>(U))
            CI->replaceAllUsesWith(NumTeamsVal);
      }
      break;
    }
    default:
      llvm::llvm_unreachable_internal("invalid action kind");
    }
  }
}

std::unordered_set<ActionTy::ActionKind> DisabledOptimizations;

void parseDisabledOptimizations(const char *Env) {
  if (Env == nullptr)
    return;

  std::string T;
  const char *P = Env;

  auto Parse = [](std::string S) {
    if (S == "alignment")
      DisabledOptimizations.insert(ActionTy::Alignment);
    else if (S == "specialization")
      DisabledOptimizations.insert(ActionTy::Specialization);
    else if (S == "num_teams")
      DisabledOptimizations.insert(ActionTy::NumTeams);
    else if (S == "num_threads")
      DisabledOptimizations.insert(ActionTy::NumThreads);
    else if (S == "all") {
      DisabledOptimizations.insert(ActionTy::Specialization);
      DisabledOptimizations.insert(ActionTy::NumTeams);
      DisabledOptimizations.insert(ActionTy::NumThreads);
      DisabledOptimizations.insert(ActionTy::Alignment);
    }
  };

  while (P && *P != '\0') {
    if (*P != ';') {
      T.push_back(*P++);
      continue;
    }

    ++P;

    std::transform(T.begin(), T.end(), T.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    Parse(T);

    T.clear();
  }

  if (!T.empty())
    Parse(T);
}

bool isOptimizationEnabled(ActionTy::ActionKind Kind) {
  return DisabledOptimizations.find(Kind) == DisabledOptimizations.end();
}

bool isReadOnlyValue(llvm::Value *V) {
  for (auto U : V->users()) {
    if (auto *GEP = llvm::dyn_cast<llvm::GetElementPtrInst>(U)) {
      for (auto GEPU : GEP->users())
        if (!isReadOnlyValue(GEPU))
          return false;
      continue;
    }

    if (auto *LI = llvm::dyn_cast<llvm::LoadInst>(U))
      continue;

    return false;
  }

  return true;
}

void specializeGloblaVariable(LLVMContext &Context, GlobalVariable &GV,
                              size_t Size, void *Addr) {
  ConstantInt *C = nullptr;
  if (Size == 8)
    C = ConstantInt::get(Type::getInt8Ty(Context), *(uint8_t *)Addr);
  else if (Size == 16)
    C = ConstantInt::get(Type::getInt16Ty(Context), *(uint16_t *)Addr);
  else if (Size == 32)
    C = ConstantInt::get(Type::getInt32Ty(Context), *(uint32_t *)Addr);
  else if (Size == 64)
    C = ConstantInt::get(Type::getInt64Ty(Context), *(uint64_t *)Addr);
  else
    DP("[jit] Unsupported size %zu for global variable specialization\n", Size);

  if (C) {
    GV.setInitializer(C);
    DP("[jit] Specialize global variable %s with value %" PRId64 ".\n",
       GV.getName().bytes_begin(), C->getSExtValue());
  }
}
} // namespace jit

namespace {
bool checkResult(CUresult Err, const char *ErrMsg) {
  if (Err == CUDA_SUCCESS)
    return true;

  REPORT("%s", ErrMsg);
  CUDA_ERR_STRING(Err);
  return false;
}

int memcpyDtoD(const void *SrcPtr, void *DstPtr, int64_t Size,
               CUstream Stream) {
  CUresult Err =
      cuMemcpyDtoDAsync((CUdeviceptr)DstPtr, (CUdeviceptr)SrcPtr, Size, Stream);

  if (Err != CUDA_SUCCESS) {
    DP("Error when copying data from device to device. Pointers: src "
       "= " DPxMOD ", dst = " DPxMOD ", size = %" PRId64 "\n",
       DPxPTR(SrcPtr), DPxPTR(DstPtr), Size);
    CUDA_ERR_STRING(Err);
    return OFFLOAD_FAIL;
  }

  return OFFLOAD_SUCCESS;
}

int recordEvent(void *EventPtr, __tgt_async_info *AsyncInfo) {
  CUstream Stream = reinterpret_cast<CUstream>(AsyncInfo->Queue);
  CUevent Event = reinterpret_cast<CUevent>(EventPtr);

  CUresult Err = cuEventRecord(Event, Stream);
  if (Err != CUDA_SUCCESS) {
    DP("Error when recording event. stream = " DPxMOD ", event = " DPxMOD "\n",
       DPxPTR(Stream), DPxPTR(Event));
    CUDA_ERR_STRING(Err);
    return OFFLOAD_FAIL;
  }

  return OFFLOAD_SUCCESS;
}

int syncEvent(void *EventPtr) {
  CUevent Event = reinterpret_cast<CUevent>(EventPtr);

  CUresult Err = cuEventSynchronize(Event);
  if (Err != CUDA_SUCCESS) {
    DP("Error when syncing event = " DPxMOD "\n", DPxPTR(Event));
    CUDA_ERR_STRING(Err);
    return OFFLOAD_FAIL;
  }

  return OFFLOAD_SUCCESS;
}

// Structure contains per-device data
struct DeviceDataTy {
  /// List that contains all the kernels.
  std::list<KernelTy> KernelsList;

  ///
  std::list<__tgt_kernel_launch_entry> EntriesList;

  std::list<FuncOrGblEntryTy> FuncGblEntries;

  CUcontext Context = nullptr;
  // Device properties
  int ThreadsPerBlock = 0;
  int BlocksPerGrid = 0;
  int WarpSize = 0;
  // Maximum number of registers available per block
  int MaxRegisters = 0;
  // OpenMP properties
  int NumTeams = 0;
  int NumThreads = 0;

  struct ComputeCapabilityTy {
    int Major = 3;
    int Minor = 5;

    std::string toString() const { return "sm_" + std::to_string(toInt()); }

    int toInt() const { return Major * 10 + Minor; }
  } ComputeCapability;
};

/// Resource allocator where \p T is the resource type.
/// Functions \p create and \p destroy return OFFLOAD_SUCCESS and OFFLOAD_FAIL
/// accordingly. The implementation should not raise any exception.
template <typename T> class AllocatorTy {
public:
  /// Create a resource and assign to R.
  int create(T &R) noexcept;
  /// Destroy the resource.
  int destroy(T) noexcept;
};

/// Allocator for CUstream.
template <> class AllocatorTy<CUstream> {
  CUcontext Context;

public:
  AllocatorTy(CUcontext C) noexcept : Context(C) {}

  /// See AllocatorTy<T>::create.
  int create(CUstream &Stream) noexcept {
    if (!checkResult(cuCtxSetCurrent(Context),
                     "Error returned from cuCtxSetCurrent\n"))
      return OFFLOAD_FAIL;

    if (!checkResult(cuStreamCreate(&Stream, CU_STREAM_NON_BLOCKING),
                     "Error returned from cuStreamCreate\n"))
      return OFFLOAD_FAIL;

    return OFFLOAD_SUCCESS;
  }

  /// See AllocatorTy<T>::destroy.
  int destroy(CUstream Stream) noexcept {
    if (!checkResult(cuCtxSetCurrent(Context),
                     "Error returned from cuCtxSetCurrent\n"))
      return OFFLOAD_FAIL;
    if (!checkResult(cuStreamDestroy(Stream),
                     "Error returned from cuStreamDestroy\n"))
      return OFFLOAD_FAIL;

    return OFFLOAD_SUCCESS;
  }
};

/// Allocator for CUevent.
template <> class AllocatorTy<CUevent> {
public:
  /// See AllocatorTy<T>::create.
  int create(CUevent &Event) noexcept {
    if (!checkResult(cuEventCreate(&Event, CU_EVENT_DEFAULT),
                     "Error returned from cuEventCreate\n"))
      return OFFLOAD_FAIL;

    return OFFLOAD_SUCCESS;
  }

  /// See AllocatorTy<T>::destroy.
  int destroy(CUevent Event) noexcept {
    if (!checkResult(cuEventDestroy(Event),
                     "Error returned from cuEventDestroy\n"))
      return OFFLOAD_FAIL;

    return OFFLOAD_SUCCESS;
  }
};

/// A generic pool of resources where \p T is the resource type.
/// \p T should be copyable as the object is stored in \p std::vector .
template <typename T> class ResourcePoolTy {
  /// Index of the next available resource.
  size_t Next = 0;
  /// Mutex to guard the pool.
  std::mutex Mutex;
  /// Pool of resources.
  std::vector<T> Resources;
  /// A reference to the corresponding allocator.
  AllocatorTy<T> Allocator;

  /// If `Resources` is used up, we will fill in more resources. It assumes that
  /// the new size `Size` should be always larger than the current size.
  bool resize(size_t Size) {
    auto CurSize = Resources.size();
    assert(Size > CurSize && "Unexpected smaller size");
    Resources.reserve(Size);
    for (auto I = CurSize; I < Size; ++I) {
      T NewItem;
      int Ret = Allocator.create(NewItem);
      if (Ret != OFFLOAD_SUCCESS)
        return false;
      Resources.push_back(NewItem);
    }
    return true;
  }

public:
  ResourcePoolTy(AllocatorTy<T> &&A, size_t Size = 0) noexcept
      : Allocator(std::move(A)) {
    if (Size)
      (void)resize(Size);
  }

  ~ResourcePoolTy() noexcept { clear(); }

  /// Get a resource from pool. `Next` always points to the next available
  /// resource. That means, `[0, next-1]` have been assigned, and `[id,]` are
  /// still available. If there is no resource left, we will ask for more. Each
  /// time a resource is assigned, the id will increase one.
  /// xxxxxs+++++++++
  ///      ^
  ///      Next
  /// After assignment, the pool becomes the following and s is assigned.
  /// xxxxxs+++++++++
  ///       ^
  ///       Next
  int acquire(T &R) noexcept {
    std::lock_guard<std::mutex> LG(Mutex);
    if (Next == Resources.size()) {
      auto NewSize = Resources.size() ? Resources.size() * 2 : 1;
      if (!resize(NewSize))
        return OFFLOAD_FAIL;
    }

    assert(Next < Resources.size());

    R = Resources[Next++];

    return OFFLOAD_SUCCESS;
  }

  /// Return the resource back to the pool. When we return a resource, we need
  /// to first decrease `Next`, and then copy the resource back. It is worth
  /// noting that, the order of resources return might be different from that
  /// they're assigned, that saying, at some point, there might be two identical
  /// resources.
  /// xxax+a+++++
  ///     ^
  ///     Next
  /// However, it doesn't matter, because they're always on the two sides of
  /// `Next`. The left one will in the end be overwritten by another resource.
  /// Therefore, after several execution, the order of pool might be different
  /// from its initial state.
  void release(T R) noexcept {
    std::lock_guard<std::mutex> LG(Mutex);
    Resources[--Next] = R;
  }

  /// Released all stored resources and clear the pool.
  /// Note: This function is not thread safe. Be sure to guard it if necessary.
  void clear() noexcept {
    for (auto &R : Resources)
      (void)Allocator.destroy(R);
    Resources.clear();
  }
};

class DeviceRTLTy {
  int NumberOfDevices;
  // OpenMP environment properties
  int EnvNumTeams;
  int EnvTeamLimit;
  int EnvTeamThreadLimit;
  // OpenMP requires flags
  int64_t RequiresFlags;
  // Amount of dynamic shared memory to use at launch.
  uint64_t DynamicMemorySize;
  // Number of initial streams for each device.
  int NumInitialStreams = 32;

  static constexpr const int32_t HardThreadLimit = 1024;
  static constexpr const int32_t DefaultNumTeams = 128;
  static constexpr const int32_t DefaultNumThreads = 128;

  using StreamPoolTy = ResourcePoolTy<CUstream>;
  std::vector<std::unique_ptr<StreamPoolTy>> StreamPool;

  ResourcePoolTy<CUevent> EventPool;

  std::vector<DeviceDataTy> DeviceData;
  std::vector<CUmodule> Modules;

  std::vector<jit::TargetTableCache> TargetTableCaches;

  /// A class responsible for interacting with device native runtime library to
  /// allocate and free memory.
  class CUDADeviceAllocatorTy : public DeviceAllocatorTy {
    const int DeviceId;
    const std::vector<DeviceDataTy> &DeviceData;
    std::unordered_map<void *, TargetAllocTy> HostPinnedAllocs;

  public:
    CUDADeviceAllocatorTy(int DeviceId, std::vector<DeviceDataTy> &DeviceData)
        : DeviceId(DeviceId), DeviceData(DeviceData) {}

    void *allocate(size_t Size, void *, TargetAllocTy Kind) override {
      if (Size == 0)
        return nullptr;

      CUresult Err = cuCtxSetCurrent(DeviceData[DeviceId].Context);
      if (!checkResult(Err, "Error returned from cuCtxSetCurrent\n"))
        return nullptr;

      void *MemAlloc = nullptr;
      switch (Kind) {
      case TARGET_ALLOC_DEFAULT:
      case TARGET_ALLOC_DEVICE:
        CUdeviceptr DevicePtr;
        Err = cuMemAlloc(&DevicePtr, Size);
        MemAlloc = (void *)DevicePtr;
        if (!checkResult(Err, "Error returned from cuMemAlloc\n"))
          return nullptr;
        break;
      case TARGET_ALLOC_HOST:
        void *HostPtr;
        Err = cuMemAllocHost(&HostPtr, Size);
        MemAlloc = HostPtr;
        if (!checkResult(Err, "Error returned from cuMemAllocHost\n"))
          return nullptr;
        HostPinnedAllocs[MemAlloc] = Kind;
        break;
      case TARGET_ALLOC_SHARED:
        CUdeviceptr SharedPtr;
        Err = cuMemAllocManaged(&SharedPtr, Size, CU_MEM_ATTACH_GLOBAL);
        MemAlloc = (void *)SharedPtr;
        if (!checkResult(Err, "Error returned from cuMemAllocManaged\n"))
          return nullptr;
        break;
      }

      return MemAlloc;
    }

    int free(void *TgtPtr) override {
      CUresult Err = cuCtxSetCurrent(DeviceData[DeviceId].Context);
      if (!checkResult(Err, "Error returned from cuCtxSetCurrent\n"))
        return OFFLOAD_FAIL;

      // Host pinned memory must be freed differently.
      TargetAllocTy Kind =
          (HostPinnedAllocs.find(TgtPtr) == HostPinnedAllocs.end())
              ? TARGET_ALLOC_DEFAULT
              : TARGET_ALLOC_HOST;
      switch (Kind) {
      case TARGET_ALLOC_DEFAULT:
      case TARGET_ALLOC_DEVICE:
      case TARGET_ALLOC_SHARED:
        Err = cuMemFree((CUdeviceptr)TgtPtr);
        if (!checkResult(Err, "Error returned from cuMemFree\n"))
          return OFFLOAD_FAIL;
        break;
      case TARGET_ALLOC_HOST:
        Err = cuMemFreeHost(TgtPtr);
        if (!checkResult(Err, "Error returned from cuMemFreeHost\n"))
          return OFFLOAD_FAIL;
        break;
      }

      return OFFLOAD_SUCCESS;
    }
  };

  /// A vector of device allocators
  std::vector<CUDADeviceAllocatorTy> DeviceAllocators;

  /// A vector of memory managers. Since the memory manager is non-copyable and
  // non-removable, we wrap them into std::unique_ptr.
  std::vector<std::unique_ptr<MemoryManagerTy>> MemoryManagers;

  /// Whether use memory manager
  bool UseMemoryManager = true;

  // Record entry point associated with device
  void addOffloadEntry(const int DeviceId, const __tgt_offload_entry entry) {
    FuncOrGblEntryTy &E = DeviceData[DeviceId].FuncGblEntries.back();
    E.Entries.push_back(entry);
  }

  // Return a pointer to the entry associated with the pointer
  const __tgt_offload_entry *getOffloadEntry(const int DeviceId,
                                             const void *Addr) const {
    for (const __tgt_offload_entry &Itr :
         DeviceData[DeviceId].FuncGblEntries.back().Entries)
      if (Itr.addr == Addr)
        return &Itr;

    return nullptr;
  }

  // Return the pointer to the target entries table
  __tgt_target_table *getOffloadEntriesTable(const int DeviceId) {
    FuncOrGblEntryTy &E = DeviceData[DeviceId].FuncGblEntries.back();

    if (E.Entries.empty())
      return nullptr;

    // Update table info according to the entries and return the pointer
    E.Table.EntriesBegin = E.Entries.data();
    E.Table.EntriesEnd = E.Entries.data() + E.Entries.size();

    return &E.Table;
  }

  // Clear entries table for a device
  void clearOffloadEntriesTable(const int DeviceId) {
    DeviceData[DeviceId].FuncGblEntries.emplace_back();
    FuncOrGblEntryTy &E = DeviceData[DeviceId].FuncGblEntries.back();
    E.Entries.clear();
    E.Table.EntriesBegin = E.Table.EntriesEnd = nullptr;
  }

public:

  CUstream getStream(const int DeviceId, __tgt_async_info *AsyncInfo) const {
    assert(AsyncInfo && "AsyncInfo is nullptr");

    if (!AsyncInfo->Queue) {
      CUstream S;
      if (StreamPool[DeviceId]->acquire(S) != OFFLOAD_SUCCESS)
        return nullptr;

      AsyncInfo->Queue = S;
    }

    return reinterpret_cast<CUstream>(AsyncInfo->Queue);
  }

  __tgt_target_table *processCUmodule(int DeviceId, CUmodule Module,
                                      const __tgt_device_image *Image) {
    CUresult Err;

    // Clear the offload table as we are going to create a new one.
    clearOffloadEntriesTable(DeviceId);

    // Find the symbols in the module by name.
    const __tgt_offload_entry *HostBegin = Image->EntriesBegin;
    const __tgt_offload_entry *HostEnd = Image->EntriesEnd;

    std::list<KernelTy> &KernelsList = DeviceData[DeviceId].KernelsList;
    std::list<__tgt_kernel_launch_entry> &EntriesList =
        DeviceData[DeviceId].EntriesList;
    for (const __tgt_offload_entry *E = HostBegin; E != HostEnd; ++E) {
      if (!E->addr) {
        // We return nullptr when something like this happens, the host should
        // have always something in the address to uniquely identify the target
        // region.
        DP("Invalid binary: host entry '<null>' (size = %zd)...\n", E->size);
        return nullptr;
      }

      if (E->size) {
        __tgt_offload_entry Entry = *E;
        CUdeviceptr CUPtr;
        size_t CUSize;
        Err = cuModuleGetGlobal(&CUPtr, &CUSize, Module, E->name);
        // We keep this style here because we need the name
        if (Err != CUDA_SUCCESS) {
          REPORT("Loading global '%s' Failed\n", E->name);
          CUDA_ERR_STRING(Err);
          return nullptr;
        }

        if (CUSize != E->size) {
          DP("Loading global '%s' - size mismatch (%zd != %zd)\n", E->name,
             CUSize, E->size);
          return nullptr;
        }

        DP("Entry point " DPxMOD " maps to global %s (" DPxMOD ")\n",
           DPxPTR(E - HostBegin), E->name, DPxPTR(CUPtr));

        Entry.addr = (void *)(CUPtr);

        // Note: In the current implementation declare target variables
        // can either be link or to. This means that once unified
        // memory is activated via the requires directive, the variable
        // can be used directly from the host in both cases.
        // TODO: when variables types other than to or link are added,
        // the below condition should be changed to explicitly
        // check for to and link variables types:
        // (RequiresFlags & OMP_REQ_UNIFIED_SHARED_MEMORY && (e->flags &
        // OMP_DECLARE_TARGET_LINK || e->flags == OMP_DECLARE_TARGET_TO))
        if (RequiresFlags & OMP_REQ_UNIFIED_SHARED_MEMORY) {
          // If unified memory is present any target link or to variables
          // can access host addresses directly. There is no longer a
          // need for device copies.
          cuMemcpyHtoD(CUPtr, E->addr, sizeof(void *));
          DP("Copy linked variable host address (" DPxMOD
             ") to device address (" DPxMOD ")\n",
             DPxPTR(*((void **)E->addr)), DPxPTR(CUPtr));
        }

        addOffloadEntry(DeviceId, Entry);

        continue;
      }

      CUfunction Func;
      Err = cuModuleGetFunction(&Func, Module, E->name);
      // We keep this style here because we need the name
      if (Err != CUDA_SUCCESS) {
        REPORT("Loading '%s' Failed\n", E->name);
        CUDA_ERR_STRING(Err);
        return nullptr;
      }

      DP("Entry point " DPxMOD " maps to %s (" DPxMOD ")\n",
         DPxPTR(E - HostBegin), E->name, DPxPTR(Func));

      // default value GENERIC (in case symbol is missing from cubin file)
      llvm::omp::OMPTgtExecModeFlags ExecModeVal;
      std::string ExecModeNameStr(E->name);
      ExecModeNameStr += "_exec_mode";
      const char *ExecModeName = ExecModeNameStr.c_str();

      CUdeviceptr ExecModePtr;
      size_t CUSize;
      Err = cuModuleGetGlobal(&ExecModePtr, &CUSize, Module, ExecModeName);
      if (Err == CUDA_SUCCESS) {
        if (CUSize != sizeof(llvm::omp::OMPTgtExecModeFlags)) {
          DP("Loading global exec_mode '%s' - size mismatch (%zd != %zd)\n",
             ExecModeName, CUSize, sizeof(llvm::omp::OMPTgtExecModeFlags));
          return nullptr;
        }

        Err = cuMemcpyDtoH(&ExecModeVal, ExecModePtr, CUSize);
        if (Err != CUDA_SUCCESS) {
          REPORT("Error when copying data from device to host. Pointers: "
                 "host = " DPxMOD ", device = " DPxMOD ", size = %zd\n",
                 DPxPTR(&ExecModeVal), DPxPTR(ExecModePtr), CUSize);
          CUDA_ERR_STRING(Err);
          return nullptr;
        }
      } else {
        DP("Loading global exec_mode '%s' - symbol missing, using default "
           "value GENERIC (1)\n",
           ExecModeName);
      }

      KernelsList.emplace_back(Func, ExecModeVal);
      EntriesList.emplace_back();

      auto &LE = EntriesList.back();
      LE.TargetEntry = &KernelsList.back();

      __tgt_offload_entry Entry = *E;
      Entry.addr = &LE;
      addOffloadEntry(DeviceId, Entry);
    }

    // send device environment data to the device
    {
      // TODO: The device ID used here is not the real device ID used by OpenMP.
      DeviceEnvironmentTy DeviceEnv{0, static_cast<uint32_t>(NumberOfDevices),
                                    static_cast<uint32_t>(DeviceId),
                                    static_cast<uint32_t>(DynamicMemorySize)};

      if (const char *EnvStr = getenv("LIBOMPTARGET_DEVICE_RTL_DEBUG"))
        DeviceEnv.DebugKind = std::stoi(EnvStr);

      const char *DeviceEnvName = "omptarget_device_environment";
      CUdeviceptr DeviceEnvPtr;
      size_t CUSize;

      Err = cuModuleGetGlobal(&DeviceEnvPtr, &CUSize, Module, DeviceEnvName);
      if (Err == CUDA_SUCCESS) {
        if (CUSize != sizeof(DeviceEnv)) {
          REPORT(
              "Global device_environment '%s' - size mismatch (%zu != %zu)\n",
              DeviceEnvName, CUSize, sizeof(int32_t));
          CUDA_ERR_STRING(Err);
          return nullptr;
        }

        Err = cuMemcpyHtoD(DeviceEnvPtr, &DeviceEnv, CUSize);
        if (Err != CUDA_SUCCESS) {
          REPORT("Error when copying data from host to device. Pointers: "
                 "host = " DPxMOD ", device = " DPxMOD ", size = %zu\n",
                 DPxPTR(&DeviceEnv), DPxPTR(DeviceEnvPtr), CUSize);
          CUDA_ERR_STRING(Err);
          return nullptr;
        }

        DP("Sending global device environment data %zu bytes\n", CUSize);
      } else {
        DP("Finding global device environment '%s' - symbol missing.\n",
           DeviceEnvName);
        DP("Continue, considering this is a device RTL which does not accept "
           "environment setting.\n");
      }
    }

    return getOffloadEntriesTable(DeviceId);
  }

  CUmodule loadCUmodule(int DeviceId, const __tgt_device_image *Image,
                        CUjit_option *Options, void **OptionValues,
                        int NumOptions) {
    // Set the context we are using
    CUresult Err = cuCtxSetCurrent(DeviceData[DeviceId].Context);
    if (!checkResult(Err, "Error returned from cuCtxSetCurrent\n"))
      return nullptr;

    // Create the module and extract the function pointers.
    CUmodule Module;
    DP("Load data from image " DPxMOD "\n", DPxPTR(Image->ImageStart));
    Err = cuModuleLoadDataEx(&Module, Image->ImageStart, NumOptions, Options,
                             OptionValues);
    if (!checkResult(Err, "Error returned from cuModuleLoadData\n"))
      return nullptr;

    DP("CUDA module successfully loaded!\n");

    Modules.push_back(Module);

    return Module;
  }

  __tgt_target_table *loadJITImage(int DeviceId, __tgt_device_image *Image,
                                   __tgt_offload_entry *Entry, void **Args,
                                   int NumArgs, int NumTeams, int ThreadLimit,
                                   int LoopTripCount,
                                   __tgt_async_info *AsyncInfo) {
    DP("[jit] Load JIT image for kernel %s.\n", Entry->name);

    jit::Kernel Kernel(Entry->name);
    Kernel.setArgs(Args, NumArgs);
    if (jit::isOptimizationEnabled(jit::ActionTy::NumTeams))
      Kernel.setNumTeams(NumTeams);
    if (jit::isOptimizationEnabled(jit::ActionTy::NumThreads))
      Kernel.setNumThreads(ThreadLimit);

    auto &TableCache = TargetTableCaches[DeviceId];
    if (auto *T = TableCache.get(Kernel)) {
      DP("[jit] Found cached target table " DPxMOD ".\n", DPxPTR(T));
      return const_cast<__tgt_target_table *>(T);
    }

    DP("[jit] Cannot find cached target table. Try to find from image "
       "cache.\n");

    size_t NumRegs = DeviceData[DeviceId].MaxRegisters;
    __tgt_device_image NewImage = *Image;
    bool FoundImage = false;

    const std::string &SM = DeviceData[DeviceId].ComputeCapability.toString();
    const std::string Key = SM + "-" + std::string(Entry->name);

    if (auto *I = jit::ImageCache.get(Key, Kernel)) {
      auto IP = I->get();
      NewImage.ImageStart = IP.first;
      NewImage.ImageEnd = IP.second;
      if (I->getKernel().getNumThreads())
        NumRegs =
            DeviceData[DeviceId].MaxRegisters / I->getKernel().getNumThreads();
      Kernel.setMask(I->getKernel());
      FoundImage = true;
      DP("[jit] Found cached image " DPxMOD ", #regs=%zu.\n",
         DPxPTR(NewImage.ImageStart), NumRegs);
    }

    if (!FoundImage) {
      DP("[jit] Cannot find cached image.\n");

      std::vector<uintptr_t> Mask(NumArgs, 0);

      auto &Context = jit::ContextMap.get();
      auto LM = jit::createFromImage(Context, Image);
      if (LM == nullptr)
        return nullptr;

      std::vector<jit::ActionTy> Actions;

      auto &M = LM->getModule();

      // {
      //   std::cerr << ">>>>>> before optimization\n";
      //   M.dump();
      // }

      auto &DL = M.getDataLayout();
      auto F = M.getFunction(Entry->name);
      if (!F)
        return nullptr;

      if (F->arg_size() != NumArgs)
        return nullptr;

      bool IsSPMDMode = jit::isSPMDMode(M, F->getName());
      int NumThreads = 0;
      if (IsSPMDMode) {
        NumThreads =
            ThreadLimit ? ThreadLimit : DeviceData[DeviceId].NumThreads;
        if (NumThreads > DeviceData[DeviceId].ThreadsPerBlock)
          NumThreads = DeviceData[DeviceId].ThreadsPerBlock;
      }

      int Idx = -1;
      for (auto &Arg : F->args()) {
        ++Idx;

        // Alignment
        if (Arg.getType()->isPointerTy()) {
          if (!jit::isOptimizationEnabled(jit::ActionTy::Alignment))
            continue;

          auto ElemTy = Arg.getType()->getPointerElementType();
          if (ElemTy->isAggregateType()) {
            if (ElemTy->isStructTy()) {
              auto STy = llvm::cast<llvm::StructType>(ElemTy);
              auto SL = DL.getStructLayout(STy);
              for (int I = 0; I < STy->getNumElements(); ++I) {
                auto ET = STy->getElementType(I);
              }
            }
          } else {
            const intptr_t Ptr = (intptr_t)Args[Idx];
            const static unsigned Alignments[] = {128, 64, 32, 16, 8};
            unsigned Alignment = 0;
            for (unsigned A : Alignments)
              if (!(Ptr & (A - 1))) {
                Alignment = A;
                break;
              }

            if (Alignment)
              Actions.emplace_back(jit::ActionTy::Alignment, Idx, Alignment);
          }

          continue;
        }

        // Kernel argument specialization
        if (jit::isOptimizationEnabled(jit::ActionTy::Specialization))
          Actions.emplace_back(jit::ActionTy::Specialization, Idx,
                               (uint64_t)Args[Idx]);
      }

      // Parameter specialization
      // num_threads
      if (NumThreads && jit::isOptimizationEnabled(jit::ActionTy::NumThreads) &&
          !F->getFnAttribute("omp_target_thread_limit").isValid())
        Actions.emplace_back(jit::ActionTy::NumThreads, (uint64_t)NumThreads);

      // num_teams
      if (jit::isOptimizationEnabled(jit::ActionTy::NumTeams) &&
          !F->getFnAttribute("omp_target_num_teams").isValid()) {
        int NumBlocks = 0;
        if (NumTeams <= 0) {
          if (LoopTripCount > 0 && EnvNumTeams < 0) {
            if (IsSPMDMode)
              NumBlocks = ((LoopTripCount - 1) / NumThreads) + 1;
          } else
            NumBlocks = DeviceData[DeviceId].NumTeams;
        } else {
          NumBlocks = NumTeams;
        }

        if (NumBlocks > DeviceData[DeviceId].BlocksPerGrid)
          NumBlocks = DeviceData[DeviceId].BlocksPerGrid;

        if (NumBlocks)
          Actions.emplace_back(jit::ActionTy::NumTeams, (uint64_t)NumBlocks);
      }

      // Readonly global variable specialization
      for (auto E = Image->EntriesBegin; E != Image->EntriesEnd; ++E) {
        if (!E->size)
          continue;

        auto GV = M.getGlobalVariable(E->name);
        if (!GV)
          continue;

        // Currently we don't support aggregate type
        if (GV->getValueType()->isAggregateType())
          continue;

        if (!jit::isReadOnlyValue(GV))
          continue;

        jit::specializeGloblaVariable(Context, *GV, E->size, E->addr);
      }

      jit::applyActionsToModule(Context, M, *F, Actions, Mask);

      Kernel.setMask(Mask);

      llvm::LTOCodeGenerator CodeGen(Context);
      CodeGen.setDisableVerify(false);
      CodeGen.setCodePICModel(llvm::codegen::getExplicitRelocModel());
      CodeGen.setFreestanding(true);
      CodeGen.setDebugInfo(LTO_DEBUG_MODEL_NONE);
      CodeGen.setTargetOptions(jit::Options);
      CodeGen.setShouldRestoreGlobalsLinkage(false);
      CodeGen.setCpu(DeviceData[DeviceId].ComputeCapability.toString());
      CodeGen.setAttrs(llvm::codegen::getMAttrs());
      CodeGen.setUseNewPM(true);
      CodeGen.setFileType(llvm::CodeGenFileType::CGFT_AssemblyFile);
      CodeGen.setOptLevel(3);

      CodeGen.addModule(LM.get());
      CodeGen.addMustPreserveSymbol(Entry->name);
      CodeGen.addMustPreserveSymbol(std::string(Entry->name) + "_exec_mode");

      if (!CodeGen.optimize())
        return nullptr;

      // {
      //   std::cerr << ">>>>>> after optimization\n";
      //   auto &M = CodeGen.getMergedModule();
      //   M.dump();
      // }

      if (!IsSPMDMode) {
        auto &M = CodeGen.getMergedModule();

        GlobalVariable *ExecMode = M.getGlobalVariable(
            std::string(Entry->name) + "_exec_mode", true /* AllowInternal */);
        if (!ExecMode)
          return nullptr;
        assert(llvm::isa<ConstantInt>(ExecMode->getInitializer()) &&
               "ExecMode is not an integer!");
        const int8_t ExecModeVal =
            llvm::cast<ConstantInt>(ExecMode->getInitializer())->getSExtValue();
        if (ExecModeVal & llvm::omp::OMP_TGT_EXEC_MODE_SPMD)
          IsSPMDMode = true;

        assert(NumThreads == 0);
        NumThreads =
            ThreadLimit ? ThreadLimit : DeviceData[DeviceId].NumThreads;

        if (!IsSPMDMode)
          NumThreads += DeviceData[DeviceId].WarpSize;

        if (NumThreads > DeviceData[DeviceId].ThreadsPerBlock)
          NumThreads = DeviceData[DeviceId].ThreadsPerBlock;
      }

      assert(NumThreads && "NumThreads is still 0");

      NumRegs = DeviceData[DeviceId].MaxRegisters / NumThreads;

      auto OutputBuffer = CodeGen.compileOptimized();
      if (!OutputBuffer)
        return nullptr;

      auto *I = jit::ImageCache.insert(Key, Kernel, std::move(OutputBuffer));
      assert(I && "failed to insert image");
      auto IP = I->get();
      NewImage.ImageStart = IP.first;
      NewImage.ImageEnd = IP.second;
    }

    // Build options for CUDA JIT.
    std::vector<CUjit_option> Options;
    std::vector<void *> OptionValues;

    // Set max number of registers a thread can use.
    Options.push_back(CU_JIT_MAX_REGISTERS);
    OptionValues.push_back((void *)(size_t)(NumRegs));
    DP("[jit] set CU_JIT_MAX_REGISTERS to %zu\n", NumRegs);
    // Set log buffer
    constexpr const size_t LogBufSize = 16384;
    static std::vector<char> LogBuf(LogBufSize, 0);
    Options.push_back(CU_JIT_INFO_LOG_BUFFER);
    OptionValues.push_back(LogBuf.data());
    Options.push_back(CU_JIT_INFO_LOG_BUFFER_SIZE_BYTES);
    OptionValues.push_back((void *)LogBufSize);
    static std::vector<char> ErrorBuf(LogBufSize, 0);
    Options.push_back(CU_JIT_ERROR_LOG_BUFFER);
    OptionValues.push_back(ErrorBuf.data());
    Options.push_back(CU_JIT_ERROR_LOG_BUFFER_SIZE_BYTES);
    OptionValues.push_back((void *)LogBufSize);
    // Set verbose level
    Options.push_back(CU_JIT_LOG_VERBOSE);
    OptionValues.push_back((void *)(size_t)1);

    assert(Options.size() == OptionValues.size());

    auto CUModule = loadCUmodule(DeviceId, &NewImage, Options.data(),
                                 OptionValues.data(), Options.size());
    if (CUModule == nullptr) {
      DP("[jit] failed to jit module.\nPTX:\n%s\n",
         (const char *)NewImage.ImageStart);
      return nullptr;
    }

    if (std::strlen(LogBuf.data()))
      DP("[jit] CUDA JIT log:\n%s\n", LogBuf.data());
    if (std::strlen(ErrorBuf.data()))
      DP("[jit] CUDA JIT error log:\n%s\n", LogBuf.data());

    for (auto E = Image->EntriesBegin; E != Image->EntriesEnd; ++E) {
      if (!E->size)
        continue;

      CUdeviceptr CUPtr;
      size_t CUSize;

      CUresult Err = cuModuleGetGlobal(&CUPtr, &CUSize, CUModule, E->name);
      // If the variable is not found, it means it has been specialized. We
      // simly skip it.
      if (Err == CUDA_ERROR_NOT_FOUND)
        continue;

      // Other errors should not appear.
      if (!checkResult(Err, "Error returned from cuModuleGetGlobal\n"))
        return nullptr;

      if (CUSize != E->size) {
        DP("[jit] global size mismatches: exp: %zu, rel: %zu", E->size, CUSize);
        return nullptr;
      }

      if (dataSubmit(DeviceId, (const void *)CUPtr, E->addr, E->size,
                     AsyncInfo) != OFFLOAD_SUCCESS) {
        DP("[jit] failed to initialize global %s (" DPxMOD
           ") of size %zu with host address " DPxMOD "\n",
           E->name, DPxPTR(CUPtr), E->size, DPxPTR(E->addr));
        return nullptr;
      }
    }

    auto TT = processCUmodule(DeviceId, CUModule, &NewImage);
    if (TT == nullptr)
      return nullptr;

    TableCache.insert(Kernel, TT);

    return TT;
  }

public:
  // This class should not be copied
  DeviceRTLTy(const DeviceRTLTy &) = delete;
  DeviceRTLTy(DeviceRTLTy &&) = delete;

  DeviceRTLTy()
      : NumberOfDevices(0), EnvNumTeams(-1), EnvTeamLimit(-1),
        EnvTeamThreadLimit(-1), RequiresFlags(OMP_REQ_UNDEFINED),
        DynamicMemorySize(0), EventPool(AllocatorTy<CUevent>()) {

    DP("Start initializing CUDA\n");

    CUresult Err = cuInit(0);
    if (Err == CUDA_ERROR_INVALID_HANDLE) {
      // Can't call cuGetErrorString if dlsym failed
      DP("Failed to load CUDA shared library\n");
      return;
    }
    if (!checkResult(Err, "Error returned from cuInit\n")) {
      return;
    }

    Err = cuDeviceGetCount(&NumberOfDevices);
    if (!checkResult(Err, "Error returned from cuDeviceGetCount\n"))
      return;

    if (NumberOfDevices == 0) {
      DP("There are no devices supporting CUDA.\n");
      return;
    }

    DeviceData.resize(NumberOfDevices);
    StreamPool.resize(NumberOfDevices);

    // Get environment variables regarding teams
    if (const char *EnvStr = getenv("OMP_TEAM_LIMIT")) {
      // OMP_TEAM_LIMIT has been set
      EnvTeamLimit = std::stoi(EnvStr);
      DP("Parsed OMP_TEAM_LIMIT=%d\n", EnvTeamLimit);
    }
    if (const char *EnvStr = getenv("OMP_TEAMS_THREAD_LIMIT")) {
      // OMP_TEAMS_THREAD_LIMIT has been set
      EnvTeamThreadLimit = std::stoi(EnvStr);
      DP("Parsed OMP_TEAMS_THREAD_LIMIT=%d\n", EnvTeamThreadLimit);
    }
    if (const char *EnvStr = getenv("OMP_NUM_TEAMS")) {
      // OMP_NUM_TEAMS has been set
      EnvNumTeams = std::stoi(EnvStr);
      DP("Parsed OMP_NUM_TEAMS=%d\n", EnvNumTeams);
    }
    if (const char *EnvStr = getenv("LIBOMPTARGET_SHARED_MEMORY_SIZE")) {
      // LIBOMPTARGET_SHARED_MEMORY_SIZE has been set
      DynamicMemorySize = std::stoi(EnvStr);
      DP("Parsed LIBOMPTARGET_SHARED_MEMORY_SIZE = %" PRIu64 "\n",
         DynamicMemorySize);
    }
    if (const char *EnvStr = getenv("LIBOMPTARGET_NUM_INITIAL_STREAMS")) {
      // LIBOMPTARGET_NUM_INITIAL_STREAMS has been set
      NumInitialStreams = std::stoi(EnvStr);
      DP("Parsed LIBOMPTARGET_NUM_INITIAL_STREAMS=%d\n", NumInitialStreams);
    }

    for (int I = 0; I < NumberOfDevices; ++I)
      DeviceAllocators.emplace_back(I, DeviceData);

    // Get the size threshold from environment variable
    std::pair<size_t, bool> Res = MemoryManagerTy::getSizeThresholdFromEnv();
    UseMemoryManager = Res.second;
    size_t MemoryManagerThreshold = Res.first;

    if (UseMemoryManager)
      for (int I = 0; I < NumberOfDevices; ++I)
        MemoryManagers.emplace_back(std::make_unique<MemoryManagerTy>(
            DeviceAllocators[I], MemoryManagerThreshold));

    TargetTableCaches.resize(NumberOfDevices);

    if (const char *EnvStr = getenv("LIBOMPTARGET_JIT_DISABLED_OPTIMIZATIONS"))
      jit::parseDisabledOptimizations(EnvStr);
  }

  ~DeviceRTLTy() {
    // We first destruct memory managers in case that its dependent data are
    // destroyed before it.
    for (auto &M : MemoryManagers)
      M.release();

    for (CUmodule &M : Modules)
      // Close module
      if (M)
        checkResult(cuModuleUnload(M), "Error returned from cuModuleUnload\n");

    for (auto &S : StreamPool)
      S.reset();

    EventPool.clear();

    for (DeviceDataTy &D : DeviceData) {
      // Destroy context
      if (D.Context) {
        checkResult(cuCtxSetCurrent(D.Context),
                    "Error returned from cuCtxSetCurrent\n");
        CUdevice Device;
        checkResult(cuCtxGetDevice(&Device),
                    "Error returned from cuCtxGetDevice\n");
        checkResult(cuDevicePrimaryCtxRelease(Device),
                    "Error returned from cuDevicePrimaryCtxRelease\n");
      }
    }
  }

  // Check whether a given DeviceId is valid
  bool isValidDeviceId(const int DeviceId) const {
    return DeviceId >= 0 && DeviceId < NumberOfDevices;
  }

  int getNumOfDevices() const { return NumberOfDevices; }

  void setRequiresFlag(const int64_t Flags) { this->RequiresFlags = Flags; }

  int initDevice(const int DeviceId) {
    CUdevice Device;

    DP("Getting device %d\n", DeviceId);
    CUresult Err = cuDeviceGet(&Device, DeviceId);
    if (!checkResult(Err, "Error returned from cuDeviceGet\n"))
      return OFFLOAD_FAIL;

    // Query the current flags of the primary context and set its flags if
    // it is inactive
    unsigned int FormerPrimaryCtxFlags = 0;
    int FormerPrimaryCtxIsActive = 0;
    Err = cuDevicePrimaryCtxGetState(Device, &FormerPrimaryCtxFlags,
                                     &FormerPrimaryCtxIsActive);
    if (!checkResult(Err, "Error returned from cuDevicePrimaryCtxGetState\n"))
      return OFFLOAD_FAIL;

    if (FormerPrimaryCtxIsActive) {
      DP("The primary context is active, no change to its flags\n");
      if ((FormerPrimaryCtxFlags & CU_CTX_SCHED_MASK) !=
          CU_CTX_SCHED_BLOCKING_SYNC)
        DP("Warning the current flags are not CU_CTX_SCHED_BLOCKING_SYNC\n");
    } else {
      DP("The primary context is inactive, set its flags to "
         "CU_CTX_SCHED_BLOCKING_SYNC\n");
      Err = cuDevicePrimaryCtxSetFlags(Device, CU_CTX_SCHED_BLOCKING_SYNC);
      if (!checkResult(Err, "Error returned from cuDevicePrimaryCtxSetFlags\n"))
        return OFFLOAD_FAIL;
    }

    // Retain the per device primary context and save it to use whenever this
    // device is selected.
    Err = cuDevicePrimaryCtxRetain(&DeviceData[DeviceId].Context, Device);
    if (!checkResult(Err, "Error returned from cuDevicePrimaryCtxRetain\n"))
      return OFFLOAD_FAIL;

    Err = cuCtxSetCurrent(DeviceData[DeviceId].Context);
    if (!checkResult(Err, "Error returned from cuCtxSetCurrent\n"))
      return OFFLOAD_FAIL;

    // Initialize stream pool
    if (!StreamPool[DeviceId])
      StreamPool[DeviceId] = std::make_unique<StreamPoolTy>(
          AllocatorTy<CUstream>(DeviceData[DeviceId].Context),
          NumInitialStreams);

    // Query attributes to determine number of threads/block and blocks/grid.
    int MaxGridDimX;
    Err = cuDeviceGetAttribute(&MaxGridDimX, CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_X,
                               Device);
    if (Err != CUDA_SUCCESS) {
      DP("Error getting max grid dimension, use default value %d\n",
         DeviceRTLTy::DefaultNumTeams);
      DeviceData[DeviceId].BlocksPerGrid = DeviceRTLTy::DefaultNumTeams;
    } else {
      DP("Using %d CUDA blocks per grid\n", MaxGridDimX);
      DeviceData[DeviceId].BlocksPerGrid = MaxGridDimX;
    }

    // We are only exploiting threads along the x axis.
    int MaxBlockDimX;
    Err = cuDeviceGetAttribute(&MaxBlockDimX,
                               CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_X, Device);
    if (Err != CUDA_SUCCESS) {
      DP("Error getting max block dimension, use default value %d\n",
         DeviceRTLTy::DefaultNumThreads);
      DeviceData[DeviceId].ThreadsPerBlock = DeviceRTLTy::DefaultNumThreads;
    } else {
      DP("Using %d CUDA threads per block\n", MaxBlockDimX);
      DeviceData[DeviceId].ThreadsPerBlock = MaxBlockDimX;

      if (EnvTeamThreadLimit > 0 &&
          DeviceData[DeviceId].ThreadsPerBlock > EnvTeamThreadLimit) {
        DP("Max CUDA threads per block %d exceeds the thread limit %d set by "
           "OMP_TEAMS_THREAD_LIMIT, capping at the limit\n",
           DeviceData[DeviceId].ThreadsPerBlock, EnvTeamThreadLimit);
        DeviceData[DeviceId].ThreadsPerBlock = EnvTeamThreadLimit;
      }
      if (DeviceData[DeviceId].ThreadsPerBlock > DeviceRTLTy::HardThreadLimit) {
        DP("Max CUDA threads per block %d exceeds the hard thread limit %d, "
           "capping at the hard limit\n",
           DeviceData[DeviceId].ThreadsPerBlock, DeviceRTLTy::HardThreadLimit);
        DeviceData[DeviceId].ThreadsPerBlock = DeviceRTLTy::HardThreadLimit;
      }
    }

    // Get and set warp size
    int WarpSize;
    Err =
        cuDeviceGetAttribute(&WarpSize, CU_DEVICE_ATTRIBUTE_WARP_SIZE, Device);
    if (Err != CUDA_SUCCESS) {
      DP("Error getting warp size, assume default value 32\n");
      DeviceData[DeviceId].WarpSize = 32;
    } else {
      DP("Using warp size %d\n", WarpSize);
      DeviceData[DeviceId].WarpSize = WarpSize;
    }

    // Adjust teams to the env variables
    if (EnvTeamLimit > 0 && DeviceData[DeviceId].BlocksPerGrid > EnvTeamLimit) {
      DP("Capping max CUDA blocks per grid to OMP_TEAM_LIMIT=%d\n",
         EnvTeamLimit);
      DeviceData[DeviceId].BlocksPerGrid = EnvTeamLimit;
    }

    size_t StackLimit;
    size_t HeapLimit;
    if (const char *EnvStr = getenv("LIBOMPTARGET_STACK_SIZE")) {
      StackLimit = std::stol(EnvStr);
      if (cuCtxSetLimit(CU_LIMIT_STACK_SIZE, StackLimit) != CUDA_SUCCESS)
        return OFFLOAD_FAIL;
    } else {
      if (cuCtxGetLimit(&StackLimit, CU_LIMIT_STACK_SIZE) != CUDA_SUCCESS)
        return OFFLOAD_FAIL;
    }
    if (const char *EnvStr = getenv("LIBOMPTARGET_HEAP_SIZE")) {
      HeapLimit = std::stol(EnvStr);
      if (cuCtxSetLimit(CU_LIMIT_MALLOC_HEAP_SIZE, HeapLimit) != CUDA_SUCCESS)
        return OFFLOAD_FAIL;
    } else {
      if (cuCtxGetLimit(&HeapLimit, CU_LIMIT_MALLOC_HEAP_SIZE) != CUDA_SUCCESS)
        return OFFLOAD_FAIL;
    }

    INFO(OMP_INFOTYPE_PLUGIN_KERNEL, DeviceId,
         "Device supports up to %d CUDA blocks and %d threads with a "
         "warp size of %d\n",
         DeviceData[DeviceId].BlocksPerGrid,
         DeviceData[DeviceId].ThreadsPerBlock, DeviceData[DeviceId].WarpSize);
    INFO(OMP_INFOTYPE_PLUGIN_KERNEL, DeviceId,
         "Device heap size is %d Bytes, device stack size is %d Bytes per "
         "thread\n",
         (int)HeapLimit, (int)StackLimit);

    // Set default number of teams
    if (EnvNumTeams > 0) {
      DP("Default number of teams set according to environment %d\n",
         EnvNumTeams);
      DeviceData[DeviceId].NumTeams = EnvNumTeams;
    } else {
      DeviceData[DeviceId].NumTeams = DeviceRTLTy::DefaultNumTeams;
      DP("Default number of teams set according to library's default %d\n",
         DeviceRTLTy::DefaultNumTeams);
    }

    if (DeviceData[DeviceId].NumTeams > DeviceData[DeviceId].BlocksPerGrid) {
      DP("Default number of teams exceeds device limit, capping at %d\n",
         DeviceData[DeviceId].BlocksPerGrid);
      DeviceData[DeviceId].NumTeams = DeviceData[DeviceId].BlocksPerGrid;
    }

    // Set default number of threads
    DeviceData[DeviceId].NumThreads = DeviceRTLTy::DefaultNumThreads;
    DP("Default number of threads set according to library's default %d\n",
       DeviceRTLTy::DefaultNumThreads);
    if (DeviceData[DeviceId].NumThreads >
        DeviceData[DeviceId].ThreadsPerBlock) {
      DP("Default number of threads exceeds device limit, capping at %d\n",
         DeviceData[DeviceId].ThreadsPerBlock);
      DeviceData[DeviceId].NumThreads = DeviceData[DeviceId].ThreadsPerBlock;
    }

    // Get compute capability
    int SM;
    Err = cuDeviceGetAttribute(
        &SM, CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR, Device);
    if (Err != CUDA_SUCCESS) {
      DP("Error getting compute capablity major, use default value %d\n",
         DeviceData[DeviceId].ComputeCapability.Major);
    } else {
      DeviceData[DeviceId].ComputeCapability.Major = SM;
    }
    Err = cuDeviceGetAttribute(
        &SM, CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR, Device);
    if (Err != CUDA_SUCCESS) {
      DP("Error getting compute capablity minor, use default value %d\n",
         DeviceData[DeviceId].ComputeCapability.Minor);
    } else {
      DeviceData[DeviceId].ComputeCapability.Minor = SM;
    }
    int MaxRegs;
    Err = cuDeviceGetAttribute(
        &MaxRegs, CU_DEVICE_ATTRIBUTE_MAX_REGISTERS_PER_BLOCK, Device);
    if (Err != CUDA_SUCCESS) {
      DP("Error getting max registers per block, use default value %d\n",
         DeviceData[DeviceId].MaxRegisters);
    } else {
      DeviceData[DeviceId].MaxRegisters = MaxRegs;
    }

    return OFFLOAD_SUCCESS;
  }

  __tgt_target_table *loadBinary(const int DeviceId,
                                 const __tgt_device_image *Image) {
    // Create the module and extract the function pointers.
    CUmodule Module = loadCUmodule(DeviceId, Image, nullptr, nullptr, 0);
    if (Module == nullptr)
      return nullptr;

    return processCUmodule(DeviceId, Module, Image);
  }

  void *dataAlloc(const int DeviceId, const int64_t Size,
                  const TargetAllocTy Kind) {
    switch (Kind) {
    case TARGET_ALLOC_DEFAULT:
    case TARGET_ALLOC_DEVICE:
      if (UseMemoryManager)
        return MemoryManagers[DeviceId]->allocate(Size, nullptr);
      else
        return DeviceAllocators[DeviceId].allocate(Size, nullptr, Kind);
    case TARGET_ALLOC_HOST:
    case TARGET_ALLOC_SHARED:
      return DeviceAllocators[DeviceId].allocate(Size, nullptr, Kind);
    }

    REPORT("Invalid target data allocation kind or requested allocator not "
           "implemented yet\n");

    return nullptr;
  }

  int dataSubmit(const int DeviceId, const void *TgtPtr, const void *HstPtr,
                 const int64_t Size, __tgt_async_info *AsyncInfo) const {
    assert(AsyncInfo && "AsyncInfo is nullptr");

    CUresult Err = cuCtxSetCurrent(DeviceData[DeviceId].Context);
    if (!checkResult(Err, "Error returned from cuCtxSetCurrent\n"))
      return OFFLOAD_FAIL;

    CUstream Stream = getStream(DeviceId, AsyncInfo);

    Err = cuMemcpyHtoDAsync((CUdeviceptr)TgtPtr, HstPtr, Size, Stream);
    if (Err != CUDA_SUCCESS) {
      DP("Error when copying data from host to device. Pointers: host "
         "= " DPxMOD ", device = " DPxMOD ", size = %" PRId64 "\n",
         DPxPTR(HstPtr), DPxPTR(TgtPtr), Size);
      CUDA_ERR_STRING(Err);
      return OFFLOAD_FAIL;
    }

    return OFFLOAD_SUCCESS;
  }

  int dataRetrieve(const int DeviceId, void *HstPtr, const void *TgtPtr,
                   const int64_t Size, __tgt_async_info *AsyncInfo) const {
    assert(AsyncInfo && "AsyncInfo is nullptr");

    CUresult Err = cuCtxSetCurrent(DeviceData[DeviceId].Context);
    if (!checkResult(Err, "Error returned from cuCtxSetCurrent\n"))
      return OFFLOAD_FAIL;

    CUstream Stream = getStream(DeviceId, AsyncInfo);

    Err = cuMemcpyDtoHAsync(HstPtr, (CUdeviceptr)TgtPtr, Size, Stream);
    if (Err != CUDA_SUCCESS) {
      DP("Error when copying data from device to host. Pointers: host "
         "= " DPxMOD ", device = " DPxMOD ", size = %" PRId64 "\n",
         DPxPTR(HstPtr), DPxPTR(TgtPtr), Size);
      CUDA_ERR_STRING(Err);
      return OFFLOAD_FAIL;
    }

    return OFFLOAD_SUCCESS;
  }

  int dataExchange(int SrcDevId, const void *SrcPtr, int DstDevId, void *DstPtr,
                   int64_t Size, __tgt_async_info *AsyncInfo) const {
    assert(AsyncInfo && "AsyncInfo is nullptr");

    CUresult Err = cuCtxSetCurrent(DeviceData[SrcDevId].Context);
    if (!checkResult(Err, "Error returned from cuCtxSetCurrent\n"))
      return OFFLOAD_FAIL;

    CUstream Stream = getStream(SrcDevId, AsyncInfo);

    // If they are two devices, we try peer to peer copy first
    if (SrcDevId != DstDevId) {
      int CanAccessPeer = 0;
      Err = cuDeviceCanAccessPeer(&CanAccessPeer, SrcDevId, DstDevId);
      if (Err != CUDA_SUCCESS) {
        REPORT("Error returned from cuDeviceCanAccessPeer. src = %" PRId32
               ", dst = %" PRId32 "\n",
               SrcDevId, DstDevId);
        CUDA_ERR_STRING(Err);
        return memcpyDtoD(SrcPtr, DstPtr, Size, Stream);
      }

      if (!CanAccessPeer) {
        DP("P2P memcpy not supported so fall back to D2D memcpy");
        return memcpyDtoD(SrcPtr, DstPtr, Size, Stream);
      }

      Err = cuCtxEnablePeerAccess(DeviceData[DstDevId].Context, 0);
      if (Err != CUDA_SUCCESS) {
        REPORT("Error returned from cuCtxEnablePeerAccess. src = %" PRId32
               ", dst = %" PRId32 "\n",
               SrcDevId, DstDevId);
        CUDA_ERR_STRING(Err);
        return memcpyDtoD(SrcPtr, DstPtr, Size, Stream);
      }

      Err = cuMemcpyPeerAsync((CUdeviceptr)DstPtr, DeviceData[DstDevId].Context,
                              (CUdeviceptr)SrcPtr, DeviceData[SrcDevId].Context,
                              Size, Stream);
      if (Err == CUDA_SUCCESS)
        return OFFLOAD_SUCCESS;

      DP("Error returned from cuMemcpyPeerAsync. src_ptr = " DPxMOD
         ", src_id =%" PRId32 ", dst_ptr = " DPxMOD ", dst_id =%" PRId32 "\n",
         DPxPTR(SrcPtr), SrcDevId, DPxPTR(DstPtr), DstDevId);
      CUDA_ERR_STRING(Err);
    }

    return memcpyDtoD(SrcPtr, DstPtr, Size, Stream);
  }

  int dataDelete(const int DeviceId, void *TgtPtr) {
    if (UseMemoryManager)
      return MemoryManagers[DeviceId]->free(TgtPtr);

    return DeviceAllocators[DeviceId].free(TgtPtr);
  }

  int runTargetTeamRegion(const int DeviceId, void *TgtEntryPtr, void **TgtArgs,
                          ptrdiff_t *TgtOffsets, const int ArgNum,
                          const int TeamNum, const int ThreadLimit,
                          const unsigned int LoopTripCount,
                          __tgt_async_info *AsyncInfo) {
    CUresult Err = cuCtxSetCurrent(DeviceData[DeviceId].Context);
    if (!checkResult(Err, "Error returned from cuCtxSetCurrent\n"))
      return OFFLOAD_FAIL;

    // All args are references.
    std::vector<void *> Args(ArgNum);
    std::vector<void *> Ptrs(ArgNum);

    for (int I = 0; I < ArgNum; ++I) {
      Ptrs[I] = (void *)((intptr_t)TgtArgs[I] + TgtOffsets[I]);
      Args[I] = &Ptrs[I];
    }

    auto LaunchEntry =
        reinterpret_cast<__tgt_kernel_launch_entry *>(TgtEntryPtr);
    KernelTy *KernelInfo =
        reinterpret_cast<KernelTy *>(LaunchEntry->TargetEntry);
    // If kernel info is nullptr, it means we are dealing with JIT image.
    if (KernelInfo == nullptr) {
      assert(LaunchEntry->Image && LaunchEntry->HostEntry);
      __tgt_device_image NewImage = *(LaunchEntry->Image);
      NewImage.EntriesBegin = LaunchEntry->HostEntry;
      NewImage.EntriesEnd = NewImage.EntriesBegin + 1;
      auto TargetTable =
          loadJITImage(DeviceId, &NewImage, LaunchEntry->HostEntry, Ptrs.data(),
                       ArgNum, TeamNum, ThreadLimit, LoopTripCount, AsyncInfo);
      if (!TargetTable)
        return OFFLOAD_FAIL;

      auto JITLE = reinterpret_cast<__tgt_kernel_launch_entry *>(
          TargetTable->EntriesBegin->addr);
      KernelInfo = reinterpret_cast<KernelTy *>(JITLE->TargetEntry);
    }

    assert(KernelInfo);

    const bool IsSPMDGenericMode =
        KernelInfo->ExecutionMode == llvm::omp::OMP_TGT_EXEC_MODE_GENERIC_SPMD;
    const bool IsSPMDMode =
        KernelInfo->ExecutionMode == llvm::omp::OMP_TGT_EXEC_MODE_SPMD;
    const bool IsGenericMode =
        KernelInfo->ExecutionMode == llvm::omp::OMP_TGT_EXEC_MODE_GENERIC;

    int CudaThreadsPerBlock;
    if (ThreadLimit > 0) {
      DP("Setting CUDA threads per block to requested %d\n", ThreadLimit);
      CudaThreadsPerBlock = ThreadLimit;
      // Add master warp if necessary
      if (IsGenericMode) {
        DP("Adding master warp: +%d threads\n", DeviceData[DeviceId].WarpSize);
        CudaThreadsPerBlock += DeviceData[DeviceId].WarpSize;
      }
    } else {
      DP("Setting CUDA threads per block to default %d\n",
         DeviceData[DeviceId].NumThreads);
      CudaThreadsPerBlock = DeviceData[DeviceId].NumThreads;
    }

    if (CudaThreadsPerBlock > DeviceData[DeviceId].ThreadsPerBlock) {
      DP("Threads per block capped at device limit %d\n",
         DeviceData[DeviceId].ThreadsPerBlock);
      CudaThreadsPerBlock = DeviceData[DeviceId].ThreadsPerBlock;
    }

    if (!KernelInfo->MaxThreadsPerBlock) {
      Err = cuFuncGetAttribute(&KernelInfo->MaxThreadsPerBlock,
                               CU_FUNC_ATTRIBUTE_MAX_THREADS_PER_BLOCK,
                               KernelInfo->Func);
      if (!checkResult(Err, "Error returned from cuFuncGetAttribute\n"))
        return OFFLOAD_FAIL;
    }

    if (KernelInfo->MaxThreadsPerBlock < CudaThreadsPerBlock) {
      DP("Threads per block capped at kernel limit %d\n",
         KernelInfo->MaxThreadsPerBlock);
      CudaThreadsPerBlock = KernelInfo->MaxThreadsPerBlock;
    }

    unsigned int CudaBlocksPerGrid;
    if (TeamNum <= 0) {
      if (LoopTripCount > 0 && EnvNumTeams < 0) {
        if (IsSPMDGenericMode) {
          // If we reach this point, then we are executing a kernel that was
          // transformed from Generic-mode to SPMD-mode. This kernel has
          // SPMD-mode execution, but needs its blocks to be scheduled
          // differently because the current loop trip count only applies to the
          // `teams distribute` region and will create var too few blocks using
          // the regular SPMD-mode method.
          CudaBlocksPerGrid = LoopTripCount;
        } else if (IsSPMDMode) {
          // We have a combined construct, i.e. `target teams distribute
          // parallel for [simd]`. We launch so many teams so that each thread
          // will execute one iteration of the loop. round up to the nearest
          // integer
          CudaBlocksPerGrid = ((LoopTripCount - 1) / CudaThreadsPerBlock) + 1;
        } else if (IsGenericMode) {
          // If we reach this point, then we have a non-combined construct, i.e.
          // `teams distribute` with a nested `parallel for` and each team is
          // assigned one iteration of the `distribute` loop. E.g.:
          //
          // #pragma omp target teams distribute
          // for(...loop_tripcount...) {
          //   #pragma omp parallel for
          //   for(...) {}
          // }
          //
          // Threads within a team will execute the iterations of the `parallel`
          // loop.
          CudaBlocksPerGrid = LoopTripCount;
        } else {
          REPORT("Unknown execution mode: %d\n",
                 static_cast<int8_t>(KernelInfo->ExecutionMode));
          return OFFLOAD_FAIL;
        }
        DP("Using %d teams due to loop trip count %" PRIu32
           " and number of threads per block %d\n",
           CudaBlocksPerGrid, LoopTripCount, CudaThreadsPerBlock);
      } else {
        DP("Using default number of teams %d\n", DeviceData[DeviceId].NumTeams);
        CudaBlocksPerGrid = DeviceData[DeviceId].NumTeams;
      }
    } else {
      DP("Using requested number of teams %d\n", TeamNum);
      CudaBlocksPerGrid = TeamNum;
    }

    if (CudaBlocksPerGrid > DeviceData[DeviceId].BlocksPerGrid) {
      DP("Capping number of teams to team limit %d\n",
         DeviceData[DeviceId].BlocksPerGrid);
      CudaBlocksPerGrid = DeviceData[DeviceId].BlocksPerGrid;
    }

    INFO(OMP_INFOTYPE_PLUGIN_KERNEL, DeviceId,
         "Launching kernel %s with %d blocks and %d threads in %s mode\n",
         (getOffloadEntry(DeviceId, TgtEntryPtr))
             ? getOffloadEntry(DeviceId, TgtEntryPtr)->name
             : "(null)",
         CudaBlocksPerGrid, CudaThreadsPerBlock,
         (!IsSPMDMode ? (IsGenericMode ? "Generic" : "SPMD-Generic") : "SPMD"));

    CUstream Stream = getStream(DeviceId, AsyncInfo);
    Err = cuLaunchKernel(KernelInfo->Func, CudaBlocksPerGrid, /* gridDimY */ 1,
                         /* gridDimZ */ 1, CudaThreadsPerBlock,
                         /* blockDimY */ 1, /* blockDimZ */ 1,
                         DynamicMemorySize, Stream, &Args[0], nullptr);
    if (!checkResult(Err, "Error returned from cuLaunchKernel\n"))
      return OFFLOAD_FAIL;

    DP("Launch of entry point at " DPxMOD " successful!\n",
       DPxPTR(TgtEntryPtr));

    return OFFLOAD_SUCCESS;
  }

  int synchronize(const int DeviceId, __tgt_async_info *AsyncInfo) const {
    CUstream Stream = reinterpret_cast<CUstream>(AsyncInfo->Queue);
    CUresult Err = cuStreamSynchronize(Stream);

    // Once the stream is synchronized, return it to stream pool and reset
    // AsyncInfo. This is to make sure the synchronization only works for its
    // own tasks.
    StreamPool[DeviceId]->release(reinterpret_cast<CUstream>(AsyncInfo->Queue));
    AsyncInfo->Queue = nullptr;

    if (Err != CUDA_SUCCESS) {
      DP("Error when synchronizing stream. stream = " DPxMOD
         ", async info ptr = " DPxMOD "\n",
         DPxPTR(Stream), DPxPTR(AsyncInfo));
      CUDA_ERR_STRING(Err);
    }
    return (Err == CUDA_SUCCESS) ? OFFLOAD_SUCCESS : OFFLOAD_FAIL;
  }

  void printDeviceInfo(int32_t device_id) {
    char TmpChar[1000];
    std::string TmpStr;
    size_t TmpSt;
    int TmpInt, TmpInt2, TmpInt3;

    CUdevice Device;
    checkResult(cuDeviceGet(&Device, device_id),
                "Error returned from cuCtxGetDevice\n");

    cuDriverGetVersion(&TmpInt);
    printf("    CUDA Driver Version: \t\t%d \n", TmpInt);
    printf("    CUDA Device Number: \t\t%d \n", device_id);
    checkResult(cuDeviceGetName(TmpChar, 1000, Device),
                "Error returned from cuDeviceGetName\n");
    printf("    Device Name: \t\t\t%s \n", TmpChar);
    checkResult(cuDeviceTotalMem(&TmpSt, Device),
                "Error returned from cuDeviceTotalMem\n");
    printf("    Global Memory Size: \t\t%zu bytes \n", TmpSt);
    checkResult(cuDeviceGetAttribute(
                    &TmpInt, CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT, Device),
                "Error returned from cuDeviceGetAttribute\n");
    printf("    Number of Multiprocessors: \t\t%d \n", TmpInt);
    checkResult(
        cuDeviceGetAttribute(&TmpInt, CU_DEVICE_ATTRIBUTE_GPU_OVERLAP, Device),
        "Error returned from cuDeviceGetAttribute\n");
    printf("    Concurrent Copy and Execution: \t%s \n", BOOL2TEXT(TmpInt));
    checkResult(cuDeviceGetAttribute(
                    &TmpInt, CU_DEVICE_ATTRIBUTE_TOTAL_CONSTANT_MEMORY, Device),
                "Error returned from cuDeviceGetAttribute\n");
    printf("    Total Constant Memory: \t\t%d bytes\n", TmpInt);
    checkResult(
        cuDeviceGetAttribute(
            &TmpInt, CU_DEVICE_ATTRIBUTE_MAX_SHARED_MEMORY_PER_BLOCK, Device),
        "Error returned from cuDeviceGetAttribute\n");
    printf("    Max Shared Memory per Block: \t%d bytes \n", TmpInt);
    checkResult(
        cuDeviceGetAttribute(
            &TmpInt, CU_DEVICE_ATTRIBUTE_MAX_REGISTERS_PER_BLOCK, Device),
        "Error returned from cuDeviceGetAttribute\n");
    printf("    Registers per Block: \t\t%d \n", TmpInt);
    checkResult(
        cuDeviceGetAttribute(&TmpInt, CU_DEVICE_ATTRIBUTE_WARP_SIZE, Device),
        "Error returned from cuDeviceGetAttribute\n");
    printf("    Warp Size: \t\t\t\t%d Threads \n", TmpInt);
    checkResult(cuDeviceGetAttribute(
                    &TmpInt, CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK, Device),
                "Error returned from cuDeviceGetAttribute\n");
    printf("    Maximum Threads per Block: \t\t%d \n", TmpInt);
    checkResult(cuDeviceGetAttribute(
                    &TmpInt, CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_X, Device),
                "Error returned from cuDeviceGetAttribute\n");
    checkResult(cuDeviceGetAttribute(
                    &TmpInt2, CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Y, Device),
                "Error returned from cuDeviceGetAttribute\n");
    checkResult(cuDeviceGetAttribute(
                    &TmpInt3, CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Z, Device),
                "Error returned from cuDeviceGetAttribute\n");
    printf("    Maximum Block Dimensions: \t\t%d, %d, %d \n", TmpInt, TmpInt2,
           TmpInt3);
    checkResult(cuDeviceGetAttribute(
                    &TmpInt, CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_X, Device),
                "Error returned from cuDeviceGetAttribute\n");
    checkResult(cuDeviceGetAttribute(
                    &TmpInt2, CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Y, Device),
                "Error returned from cuDeviceGetAttribute\n");
    checkResult(cuDeviceGetAttribute(
                    &TmpInt3, CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Z, Device),
                "Error returned from cuDeviceGetAttribute\n");
    printf("    Maximum Grid Dimensions: \t\t%d x %d x %d \n", TmpInt, TmpInt2,
           TmpInt3);
    checkResult(
        cuDeviceGetAttribute(&TmpInt, CU_DEVICE_ATTRIBUTE_MAX_PITCH, Device),
        "Error returned from cuDeviceGetAttribute\n");
    printf("    Maximum Memory Pitch: \t\t%d bytes \n", TmpInt);
    checkResult(cuDeviceGetAttribute(
                    &TmpInt, CU_DEVICE_ATTRIBUTE_TEXTURE_ALIGNMENT, Device),
                "Error returned from cuDeviceGetAttribute\n");
    printf("    Texture Alignment: \t\t\t%d bytes \n", TmpInt);
    checkResult(
        cuDeviceGetAttribute(&TmpInt, CU_DEVICE_ATTRIBUTE_CLOCK_RATE, Device),
        "Error returned from cuDeviceGetAttribute\n");
    printf("    Clock Rate: \t\t\t%d kHz\n", TmpInt);
    checkResult(cuDeviceGetAttribute(
                    &TmpInt, CU_DEVICE_ATTRIBUTE_KERNEL_EXEC_TIMEOUT, Device),
                "Error returned from cuDeviceGetAttribute\n");
    printf("    Execution Timeout: \t\t\t%s \n", BOOL2TEXT(TmpInt));
    checkResult(
        cuDeviceGetAttribute(&TmpInt, CU_DEVICE_ATTRIBUTE_INTEGRATED, Device),
        "Error returned from cuDeviceGetAttribute\n");
    printf("    Integrated Device: \t\t\t%s \n", BOOL2TEXT(TmpInt));
    checkResult(cuDeviceGetAttribute(
                    &TmpInt, CU_DEVICE_ATTRIBUTE_CAN_MAP_HOST_MEMORY, Device),
                "Error returned from cuDeviceGetAttribute\n");
    printf("    Can Map Host Memory: \t\t%s \n", BOOL2TEXT(TmpInt));
    checkResult(
        cuDeviceGetAttribute(&TmpInt, CU_DEVICE_ATTRIBUTE_COMPUTE_MODE, Device),
        "Error returned from cuDeviceGetAttribute\n");
    if (TmpInt == CU_COMPUTEMODE_DEFAULT)
      TmpStr = "DEFAULT";
    else if (TmpInt == CU_COMPUTEMODE_PROHIBITED)
      TmpStr = "PROHIBITED";
    else if (TmpInt == CU_COMPUTEMODE_EXCLUSIVE_PROCESS)
      TmpStr = "EXCLUSIVE PROCESS";
    else
      TmpStr = "unknown";
    printf("    Compute Mode: \t\t\t%s \n", TmpStr.c_str());
    checkResult(cuDeviceGetAttribute(
                    &TmpInt, CU_DEVICE_ATTRIBUTE_CONCURRENT_KERNELS, Device),
                "Error returned from cuDeviceGetAttribute\n");
    printf("    Concurrent Kernels: \t\t%s \n", BOOL2TEXT(TmpInt));
    checkResult(
        cuDeviceGetAttribute(&TmpInt, CU_DEVICE_ATTRIBUTE_ECC_ENABLED, Device),
        "Error returned from cuDeviceGetAttribute\n");
    printf("    ECC Enabled: \t\t\t%s \n", BOOL2TEXT(TmpInt));
    checkResult(cuDeviceGetAttribute(
                    &TmpInt, CU_DEVICE_ATTRIBUTE_MEMORY_CLOCK_RATE, Device),
                "Error returned from cuDeviceGetAttribute\n");
    printf("    Memory Clock Rate: \t\t\t%d kHz\n", TmpInt);
    checkResult(
        cuDeviceGetAttribute(
            &TmpInt, CU_DEVICE_ATTRIBUTE_GLOBAL_MEMORY_BUS_WIDTH, Device),
        "Error returned from cuDeviceGetAttribute\n");
    printf("    Memory Bus Width: \t\t\t%d bits\n", TmpInt);
    checkResult(cuDeviceGetAttribute(&TmpInt, CU_DEVICE_ATTRIBUTE_L2_CACHE_SIZE,
                                     Device),
                "Error returned from cuDeviceGetAttribute\n");
    printf("    L2 Cache Size: \t\t\t%d bytes \n", TmpInt);
    checkResult(cuDeviceGetAttribute(
                    &TmpInt, CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_MULTIPROCESSOR,
                    Device),
                "Error returned from cuDeviceGetAttribute\n");
    printf("    Max Threads Per SMP: \t\t%d \n", TmpInt);
    checkResult(cuDeviceGetAttribute(
                    &TmpInt, CU_DEVICE_ATTRIBUTE_ASYNC_ENGINE_COUNT, Device),
                "Error returned from cuDeviceGetAttribute\n");
    printf("    Async Engines: \t\t\t%s (%d) \n", BOOL2TEXT(TmpInt), TmpInt);
    checkResult(cuDeviceGetAttribute(
                    &TmpInt, CU_DEVICE_ATTRIBUTE_UNIFIED_ADDRESSING, Device),
                "Error returned from cuDeviceGetAttribute\n");
    printf("    Unified Addressing: \t\t%s \n", BOOL2TEXT(TmpInt));
    checkResult(cuDeviceGetAttribute(
                    &TmpInt, CU_DEVICE_ATTRIBUTE_MANAGED_MEMORY, Device),
                "Error returned from cuDeviceGetAttribute\n");
    printf("    Managed Memory: \t\t\t%s \n", BOOL2TEXT(TmpInt));
    checkResult(
        cuDeviceGetAttribute(
            &TmpInt, CU_DEVICE_ATTRIBUTE_CONCURRENT_MANAGED_ACCESS, Device),
        "Error returned from cuDeviceGetAttribute\n");
    printf("    Concurrent Managed Memory: \t\t%s \n", BOOL2TEXT(TmpInt));
    checkResult(
        cuDeviceGetAttribute(
            &TmpInt, CU_DEVICE_ATTRIBUTE_COMPUTE_PREEMPTION_SUPPORTED, Device),
        "Error returned from cuDeviceGetAttribute\n");
    printf("    Preemption Supported: \t\t%s \n", BOOL2TEXT(TmpInt));
    checkResult(cuDeviceGetAttribute(
                    &TmpInt, CU_DEVICE_ATTRIBUTE_COOPERATIVE_LAUNCH, Device),
                "Error returned from cuDeviceGetAttribute\n");
    printf("    Cooperative Launch: \t\t%s \n", BOOL2TEXT(TmpInt));
    checkResult(cuDeviceGetAttribute(
                    &TmpInt, CU_DEVICE_ATTRIBUTE_MULTI_GPU_BOARD, Device),
                "Error returned from cuDeviceGetAttribute\n");
    printf("    Multi-Device Boars: \t\t%s \n", BOOL2TEXT(TmpInt));
    checkResult(
        cuDeviceGetAttribute(
            &TmpInt, CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR, Device),
        "Error returned from cuDeviceGetAttribute\n");
    checkResult(
        cuDeviceGetAttribute(
            &TmpInt2, CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR, Device),
        "Error returned from cuDeviceGetAttribute\n");
    printf("    Compute Capabilities: \t\t%d%d \n", TmpInt, TmpInt2);
  }

  int createEvent(void **P) {
    CUevent Event = nullptr;
    if (EventPool.acquire(Event) != OFFLOAD_SUCCESS)
      return OFFLOAD_FAIL;
    *P = Event;
    return OFFLOAD_SUCCESS;
  }

  int destroyEvent(void *EventPtr) {
    EventPool.release(reinterpret_cast<CUevent>(EventPtr));
    return OFFLOAD_SUCCESS;
  }

  int waitEvent(const int DeviceId, __tgt_async_info *AsyncInfo,
                void *EventPtr) const {
    CUstream Stream = getStream(DeviceId, AsyncInfo);
    CUevent Event = reinterpret_cast<CUevent>(EventPtr);

    // We don't use CU_EVENT_WAIT_DEFAULT here as it is only available from
    // specific CUDA version, and defined as 0x0. In previous version, per CUDA
    // API document, that argument has to be 0x0.
    CUresult Err = cuStreamWaitEvent(Stream, Event, 0);
    if (Err != CUDA_SUCCESS) {
      DP("Error when waiting event. stream = " DPxMOD ", event = " DPxMOD "\n",
         DPxPTR(Stream), DPxPTR(Event));
      CUDA_ERR_STRING(Err);
      return OFFLOAD_FAIL;
    }

    return OFFLOAD_SUCCESS;
  }

  int releaseAsyncInfo(int DeviceId, __tgt_async_info *AsyncInfo) const {
    if (AsyncInfo->Queue) {
      StreamPool[DeviceId]->release(
          reinterpret_cast<CUstream>(AsyncInfo->Queue));
      AsyncInfo->Queue = nullptr;
    }

    return OFFLOAD_SUCCESS;
  }

  int initAsyncInfo(int DeviceId, __tgt_async_info **AsyncInfo) const {
    CUresult Err = cuCtxSetCurrent(DeviceData[DeviceId].Context);
    if (!checkResult(Err, "error returned from cuCtxSetCurrent"))
      return OFFLOAD_FAIL;

    *AsyncInfo = new __tgt_async_info;
    getStream(DeviceId, *AsyncInfo);
    return OFFLOAD_SUCCESS;
  }

  int initDeviceInfo(int DeviceId, __tgt_device_info *DeviceInfo,
                     const char **ErrStr) const {
    assert(DeviceInfo && "DeviceInfo is nullptr");

    if (!DeviceInfo->Context)
      DeviceInfo->Context = DeviceData[DeviceId].Context;
    if (!DeviceInfo->Device) {
      CUdevice Dev;
      CUresult Err = cuDeviceGet(&Dev, DeviceId);
      if (Err == CUDA_SUCCESS) {
        DeviceInfo->Device = reinterpret_cast<void *>(Dev);
      } else {
        cuGetErrorString(Err, ErrStr);
        return OFFLOAD_FAIL;
      }
    }
    return OFFLOAD_SUCCESS;
  }
};

DeviceRTLTy DeviceRTL;
} // namespace

// Exposed library API function
#ifdef __cplusplus
extern "C" {
#endif

int32_t __tgt_rtl_is_valid_binary(__tgt_device_image *image) {
  if (elf_check_machine(image, /* EM_CUDA */ 190))
    return 1;

  // Init JIT engine just onece.
  std::call_once(jit::InitFlag, jit::init);

  auto LM = jit::createFromImage(jit::ContextMap.get(), image);
  if (LM == nullptr)
    return 0;

  if (LM->getModule().getTargetTriple().find("nvptx64") == std::string::npos)
    return 0;

  return 2;
}

int32_t __tgt_rtl_number_of_devices() { return DeviceRTL.getNumOfDevices(); }

int64_t __tgt_rtl_init_requires(int64_t RequiresFlags) {
  DP("Init requires flags to %" PRId64 "\n", RequiresFlags);
  DeviceRTL.setRequiresFlag(RequiresFlags);
  return RequiresFlags;
}

int32_t __tgt_rtl_is_data_exchangable(int32_t src_dev_id, int dst_dev_id) {
  if (DeviceRTL.isValidDeviceId(src_dev_id) &&
      DeviceRTL.isValidDeviceId(dst_dev_id))
    return 1;

  return 0;
}

int32_t __tgt_rtl_init_device(int32_t device_id) {
  assert(DeviceRTL.isValidDeviceId(device_id) && "device_id is invalid");

  return DeviceRTL.initDevice(device_id);
}

__tgt_target_table *__tgt_rtl_load_binary(int32_t device_id,
                                          __tgt_device_image *image) {
  assert(DeviceRTL.isValidDeviceId(device_id) && "device_id is invalid");

  return DeviceRTL.loadBinary(device_id, image);
}

void *__tgt_rtl_data_alloc(int32_t device_id, int64_t size, void *,
                           int32_t kind) {
  assert(DeviceRTL.isValidDeviceId(device_id) && "device_id is invalid");

  return DeviceRTL.dataAlloc(device_id, size, (TargetAllocTy)kind);
}

int32_t __tgt_rtl_data_submit(int32_t device_id, void *tgt_ptr, void *hst_ptr,
                              int64_t size) {
  assert(DeviceRTL.isValidDeviceId(device_id) && "device_id is invalid");

  __tgt_async_info AsyncInfo;
  const int32_t rc = __tgt_rtl_data_submit_async(device_id, tgt_ptr, hst_ptr,
                                                 size, &AsyncInfo);
  if (rc != OFFLOAD_SUCCESS)
    return OFFLOAD_FAIL;

  return __tgt_rtl_synchronize(device_id, &AsyncInfo);
}

int32_t __tgt_rtl_data_submit_async(int32_t device_id, void *tgt_ptr,
                                    void *hst_ptr, int64_t size,
                                    __tgt_async_info *async_info_ptr) {
  assert(DeviceRTL.isValidDeviceId(device_id) && "device_id is invalid");
  assert(async_info_ptr && "async_info_ptr is nullptr");

  return DeviceRTL.dataSubmit(device_id, tgt_ptr, hst_ptr, size,
                              async_info_ptr);
}

int32_t __tgt_rtl_data_retrieve(int32_t device_id, void *hst_ptr, void *tgt_ptr,
                                int64_t size) {
  assert(DeviceRTL.isValidDeviceId(device_id) && "device_id is invalid");

  __tgt_async_info AsyncInfo;
  const int32_t rc = __tgt_rtl_data_retrieve_async(device_id, hst_ptr, tgt_ptr,
                                                   size, &AsyncInfo);
  if (rc != OFFLOAD_SUCCESS)
    return OFFLOAD_FAIL;

  return __tgt_rtl_synchronize(device_id, &AsyncInfo);
}

int32_t __tgt_rtl_data_retrieve_async(int32_t device_id, void *hst_ptr,
                                      void *tgt_ptr, int64_t size,
                                      __tgt_async_info *async_info_ptr) {
  assert(DeviceRTL.isValidDeviceId(device_id) && "device_id is invalid");
  assert(async_info_ptr && "async_info_ptr is nullptr");

  return DeviceRTL.dataRetrieve(device_id, hst_ptr, tgt_ptr, size,
                                async_info_ptr);
}

int32_t __tgt_rtl_data_exchange_async(int32_t src_dev_id, void *src_ptr,
                                      int dst_dev_id, void *dst_ptr,
                                      int64_t size,
                                      __tgt_async_info *AsyncInfo) {
  assert(DeviceRTL.isValidDeviceId(src_dev_id) && "src_dev_id is invalid");
  assert(DeviceRTL.isValidDeviceId(dst_dev_id) && "dst_dev_id is invalid");
  assert(AsyncInfo && "AsyncInfo is nullptr");

  return DeviceRTL.dataExchange(src_dev_id, src_ptr, dst_dev_id, dst_ptr, size,
                                AsyncInfo);
}

int32_t __tgt_rtl_data_exchange(int32_t src_dev_id, void *src_ptr,
                                int32_t dst_dev_id, void *dst_ptr,
                                int64_t size) {
  assert(DeviceRTL.isValidDeviceId(src_dev_id) && "src_dev_id is invalid");
  assert(DeviceRTL.isValidDeviceId(dst_dev_id) && "dst_dev_id is invalid");

  __tgt_async_info AsyncInfo;
  const int32_t rc = __tgt_rtl_data_exchange_async(
      src_dev_id, src_ptr, dst_dev_id, dst_ptr, size, &AsyncInfo);
  if (rc != OFFLOAD_SUCCESS)
    return OFFLOAD_FAIL;

  return __tgt_rtl_synchronize(src_dev_id, &AsyncInfo);
}

int32_t __tgt_rtl_data_delete(int32_t device_id, void *tgt_ptr) {
  assert(DeviceRTL.isValidDeviceId(device_id) && "device_id is invalid");

  return DeviceRTL.dataDelete(device_id, tgt_ptr);
}

int32_t __tgt_rtl_run_target_team_region(int32_t device_id, void *tgt_entry_ptr,
                                         void **tgt_args,
                                         ptrdiff_t *tgt_offsets,
                                         int32_t arg_num, int32_t team_num,
                                         int32_t thread_limit,
                                         uint64_t loop_tripcount) {
  assert(DeviceRTL.isValidDeviceId(device_id) && "device_id is invalid");

  __tgt_async_info AsyncInfo;
  const int32_t rc = __tgt_rtl_run_target_team_region_async(
      device_id, tgt_entry_ptr, tgt_args, tgt_offsets, arg_num, team_num,
      thread_limit, loop_tripcount, &AsyncInfo);
  if (rc != OFFLOAD_SUCCESS)
    return OFFLOAD_FAIL;

  return __tgt_rtl_synchronize(device_id, &AsyncInfo);
}

int32_t __tgt_rtl_run_target_team_region_async(
    int32_t device_id, void *tgt_entry_ptr, void **tgt_args,
    ptrdiff_t *tgt_offsets, int32_t arg_num, int32_t team_num,
    int32_t thread_limit, uint64_t loop_tripcount,
    __tgt_async_info *async_info_ptr) {
  assert(DeviceRTL.isValidDeviceId(device_id) && "device_id is invalid");

  return DeviceRTL.runTargetTeamRegion(
      device_id, tgt_entry_ptr, tgt_args, tgt_offsets, arg_num, team_num,
      thread_limit, loop_tripcount, async_info_ptr);
}

int32_t __tgt_rtl_run_target_region(int32_t device_id, void *tgt_entry_ptr,
                                    void **tgt_args, ptrdiff_t *tgt_offsets,
                                    int32_t arg_num) {
  assert(DeviceRTL.isValidDeviceId(device_id) && "device_id is invalid");

  __tgt_async_info AsyncInfo;
  const int32_t rc = __tgt_rtl_run_target_region_async(
      device_id, tgt_entry_ptr, tgt_args, tgt_offsets, arg_num, &AsyncInfo);
  if (rc != OFFLOAD_SUCCESS)
    return OFFLOAD_FAIL;

  return __tgt_rtl_synchronize(device_id, &AsyncInfo);
}

int32_t __tgt_rtl_run_target_region_async(int32_t device_id,
                                          void *tgt_entry_ptr, void **tgt_args,
                                          ptrdiff_t *tgt_offsets,
                                          int32_t arg_num,
                                          __tgt_async_info *async_info_ptr) {
  assert(DeviceRTL.isValidDeviceId(device_id) && "device_id is invalid");

  return __tgt_rtl_run_target_team_region_async(
      device_id, tgt_entry_ptr, tgt_args, tgt_offsets, arg_num,
      /* team num*/ 1, /* thread_limit */ 1, /* loop_tripcount */ 0,
      async_info_ptr);
}

int32_t __tgt_rtl_synchronize(int32_t device_id,
                              __tgt_async_info *async_info_ptr) {
  assert(DeviceRTL.isValidDeviceId(device_id) && "device_id is invalid");
  assert(async_info_ptr && "async_info_ptr is nullptr");
  assert(async_info_ptr->Queue && "async_info_ptr->Queue is nullptr");

  return DeviceRTL.synchronize(device_id, async_info_ptr);
}

void __tgt_rtl_set_info_flag(uint32_t NewInfoLevel) {
  std::atomic<uint32_t> &InfoLevel = getInfoLevelInternal();
  InfoLevel.store(NewInfoLevel);
}

void __tgt_rtl_print_device_info(int32_t device_id) {
  assert(DeviceRTL.isValidDeviceId(device_id) && "device_id is invalid");
  DeviceRTL.printDeviceInfo(device_id);
}

int32_t __tgt_rtl_create_event(int32_t device_id, void **event) {
  assert(event && "event is nullptr");
  return DeviceRTL.createEvent(event);
}

int32_t __tgt_rtl_record_event(int32_t device_id, void *event_ptr,
                               __tgt_async_info *async_info_ptr) {
  assert(async_info_ptr && "async_info_ptr is nullptr");
  assert(async_info_ptr->Queue && "async_info_ptr->Queue is nullptr");
  assert(event_ptr && "event_ptr is nullptr");

  return recordEvent(event_ptr, async_info_ptr);
}

int32_t __tgt_rtl_wait_event(int32_t device_id, void *event_ptr,
                             __tgt_async_info *async_info_ptr) {
  assert(DeviceRTL.isValidDeviceId(device_id) && "device_id is invalid");
  assert(async_info_ptr && "async_info_ptr is nullptr");
  assert(event_ptr && "event is nullptr");

  return DeviceRTL.waitEvent(device_id, async_info_ptr, event_ptr);
}

int32_t __tgt_rtl_sync_event(int32_t device_id, void *event_ptr) {
  assert(event_ptr && "event is nullptr");

  return syncEvent(event_ptr);
}

int32_t __tgt_rtl_destroy_event(int32_t device_id, void *event_ptr) {
  assert(event_ptr && "event is nullptr");

  return DeviceRTL.destroyEvent(event_ptr);
}

int32_t __tgt_rtl_release_async_info(int32_t device_id,
                                     __tgt_async_info *async_info) {
  assert(DeviceRTL.isValidDeviceId(device_id) && "device_id is invalid");
  assert(async_info && "async_info is nullptr");

  return DeviceRTL.releaseAsyncInfo(device_id, async_info);
}

int32_t __tgt_rtl_init_async_info(int32_t device_id,
                                  __tgt_async_info **async_info) {
  assert(DeviceRTL.isValidDeviceId(device_id) && "device_id is invalid");
  assert(async_info && "async_info is nullptr");

  return DeviceRTL.initAsyncInfo(device_id, async_info);
}

int32_t __tgt_rtl_init_device_info(int32_t device_id,
                                   __tgt_device_info *device_info_ptr,
                                   const char **err_str) {
  assert(DeviceRTL.isValidDeviceId(device_id) && "device_id is invalid");
  assert(device_info_ptr && "device_info_ptr is nullptr");

  return DeviceRTL.initDeviceInfo(device_id, device_info_ptr, err_str);
}

#ifdef __cplusplus
}
#endif
