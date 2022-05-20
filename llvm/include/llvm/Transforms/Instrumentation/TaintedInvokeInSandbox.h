//
// Created by arun on 5/20/22.
//

#ifndef LLVM_TAINTEDINVOKEINSANDBOX_H
#define LLVM_TAINTEDINVOKEINSANDBOX_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
//---------------------------------------------------------------
// New PM Interface
//---------------------------------------------------------------
namespace llvm {

struct TaintedInvokeInSandboxPass : public PassInfoMixin<TaintedInvokeInSandboxPass>{
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

/// Legacy pass creation function for the above pass.
ModulePass *createTaintedInvokeInSandboxLegacyPass();

}
#endif // LLVM_TAINTEDINVOKEINSANDBOX_H
