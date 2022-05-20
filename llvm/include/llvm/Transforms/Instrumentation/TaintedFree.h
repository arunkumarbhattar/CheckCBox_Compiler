//
// Created by arun on 5/20/22.
//

#ifndef LLVM_TAINTEDFREE_H
#define LLVM_TAINTEDFREE_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
//---------------------------------------------------------------
// New PM Interface
//---------------------------------------------------------------
namespace llvm {

struct TaintedFreePass : public PassInfoMixin<TaintedFreePass>{
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

/// Legacy pass creation function for the above pass.
ModulePass *createTaintedFreeLegacyPass();

}
#endif // LLVM_TAINTEDFREE_H
