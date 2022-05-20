//===--- sandbox_interface.h - Interface to LLVM for Sandbox Specific APIs -===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//
//
//===----------------------------------------------------------------------===//
#include <stdlib.h>

#ifndef LLVM_CLANG_LIB_SANDBOX_API_H
#define LLVM_CLANG_LIB_SANDBOX_API_H

class sandbox_interface {

public:

  sandbox_interface(){
    //initialize the sandbox
    create_sandbox();
  }
  void *malloc_in_sandbox(size_t size){return malloc(size*sizeof(int));};

  void free_in_sandbox(void *pointer);

  template <typename T, typename... T_Args>
  auto execute_sandbox_function(const char *func_name, T_Args &&...params);

  void create_sandbox();

  void destroy_sandbox();

};

#endif