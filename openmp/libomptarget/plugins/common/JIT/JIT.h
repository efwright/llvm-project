//===-- elf_common.cpp - Common ELF functionality -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// JIT module for target plugins.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/LLVMContext.h"
#include "llvm/LTO/legacy/LTOCodeGenerator.h"
#include "llvm/Target/TargetOptions.h"

#include <string>

// Forward declaration
namespace llvm {
class MemoryBuffer;
class StringRef;
struct LTOModule;
} // namespace llvm

class JIT {
  llvm::LTOCodeGenerator CodeGen;
  std::unique_ptr<llvm::MemoryBuffer> OutputBuffer;

public:
  JIT(llvm::LLVMContext &Context, const llvm::Triple &T,
      const llvm::TargetOptions &Options);

  /// Set the optimization level.
  void setOptLevel(unsigned Level);

  /// Set CPU.
  void setCPU(const llvm::StringRef &CPU);

  /// Add module to JIT. Return true if it succeeds.
  bool addModule(struct llvm::LTOModule *LM);

  /// Optimize and compile all linked modules. Return true if we can get a valid
  /// output.
  bool run();

  /// Return the memory buffer of the output. The ownership of the buffer will
  /// be transferred to the caller.
  std::unique_ptr<llvm::MemoryBuffer> getOutput() {
    return std::move(OutputBuffer);
  }

  /// Init the JIT engine.
  static llvm::TargetOptions init(const llvm::Triple &T);
};
