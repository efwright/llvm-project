##===----------------------------------------------------------------------===##
#
# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
##===----------------------------------------------------------------------===##
#
# JIT module
#
##===----------------------------------------------------------------------===##

set(LLVM_LINK_COMPONENTS
  AllTargetsAsmParsers
  AllTargetsCodeGens
  AllTargetsDescs
  AllTargetsInfos
  LTO
)

add_llvm_library(LLVM-LIBOMPTARGET-JIT STATIC BUILDTREE_ONLY JIT.cpp)

# Build elf_common with PIC to be able to link it with plugin shared libraries.
set_property(TARGET LLVM-LIBOMPTARGET-JIT PROPERTY POSITION_INDEPENDENT_CODE ON)

llvm_update_compile_flags(LLVM-LIBOMPTARGET-JIT)

target_link_libraries(LLVM-LIBOMPTARGET-JIT INTERFACE ${OPENMP_PTHREAD_LIB} curses dl)

# Expose JIT.h directory to the users of this library.
target_include_directories(LLVM-LIBOMPTARGET-JIT INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
