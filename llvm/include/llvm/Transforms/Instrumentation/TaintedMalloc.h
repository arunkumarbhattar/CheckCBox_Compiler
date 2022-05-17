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
namespace llvm {

struct TaintedMallocPass : public PassInfoMixin<TaintedMallocPass>{
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

/// Legacy pass creation function for the above pass.
ModulePass *createTaintedMallocLegacyPass();

}
#endif // LLVM_TAINTEDMALLOC_H
