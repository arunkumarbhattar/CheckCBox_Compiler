//
// Created by arun on 5/16/22.
//

#ifndef LLVM_TAINTEDMALLOC_H
#define LLVM_TAINTEDMALLOC_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

//---------------------------------------------------------------
// New PM Interface
//---------------------------------------------------------------
struct TaintedMalloc : public llvm::ModulePass{
  static char ID;
  TaintedMalloc() : ModulePass(ID) {};
  bool runOnModule(llvm::Module &M);
};

#endif // LLVM_TAINTEDMALLOC_H
