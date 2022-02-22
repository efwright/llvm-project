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

// #include "JIT.h"

// #include "llvm/ADT/SmallVector.h"
// #include "llvm/ADT/StringRef.h"
// #include "llvm/CodeGen/CommandFlags.h"
// #include "llvm/LTO/legacy/LTOModule.h"
// #include "llvm/Support/MemoryBuffer.h"
// #include "llvm/Support/TargetSelect.h"

// using namespace llvm;

// // static codegen::RegisterCodeGenFlags CGF;

// JIT::JIT(LLVMContext &Context, const Triple &T, const TargetOptions &Options)
//     : CodeGen(Context) {
//   // Configure CodeGen
//   CodeGen.setDisableVerify(false);
//   CodeGen.setCodePICModel(codegen::getExplicitRelocModel());
//   CodeGen.setFreestanding(true);
//   CodeGen.setDebugInfo(LTO_DEBUG_MODEL_DWARF);
//   CodeGen.setTargetOptions(Options);
//   CodeGen.setShouldRestoreGlobalsLinkage(true);
//   CodeGen.setCpu(codegen::getMCPU());
//   CodeGen.setAttrs(codegen::getMAttrs());
//   CodeGen.setUseNewPM(true);
//   CodeGen.setFileType(CodeGenFileType::CGFT_AssemblyFile);
// }

// void JIT::setOptLevel(unsigned Level) { CodeGen.setOptLevel(Level); }

// void JIT::setCPU(const StringRef &CPU) { CodeGen.setCpu(CPU); }

// /// Add module to JIT. Return true if it succeeds.
// bool JIT::addModule(struct LTOModule *LM) {
//   Module &M = LM->getModule();
//   NamedMDNode *MD = M.getNamedMetadata("nvvm.annotations");

//   // TODO: Do we want to skip the module if there is no kernel?
//   if (!MD)
//     return false;

//   SmallVector<StringRef> WorkList;

//   for (auto *Op : MD->operands()) {
//     if (Op->getNumOperands() < 2)
//       continue;

//     MDString *KindID = dyn_cast<MDString>(Op->getOperand(1));
//     if (!KindID || KindID->getString() != "kernel")
//       continue;

//     Function *KernelFn =
//         mdconst::dyn_extract_or_null<Function>(Op->getOperand(0));
//     if (!KernelFn)
//       continue;

//     WorkList.push_back(KernelFn->getName());
//   }

//   if (!CodeGen.addModule(LM))
//     return false;

//   for (auto &F : WorkList)
//     CodeGen.addMustPreserveSymbol(F);

//   return true;
// }

// /// Optimize and compile all linked modules. Return true if we can get a valid
// /// output.
// bool JIT::run() {
//   OutputBuffer = CodeGen.compile();
//   return OutputBuffer != nullptr;
// }

// TargetOptions JIT::init(const Triple &T) {
//   // Initialize the configured targets.
//   InitializeAllTargets();
//   InitializeAllTargetMCs();
//   InitializeAllAsmPrinters();
//   InitializeAllAsmParsers();

//   return codegen::InitTargetOptionsFromCodeGenFlags(T);
// }
