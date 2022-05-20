//
// Created by arun on 5/16/22.
//
/// FILE:
/// TaintedMalloc.cpp

/// DESCRIPTION:
/// For every call to t_malloc function in the input IR module, TaintedMalloc
/// inserts a call to malloc_in_sandbox with the appropriate size_t parameter,
/// following which the original t_malloc function call gets removed.

/// USAGE:
/// 1.) Legacy Pass manager:
///     $opt -load <BUILD_DIR>/lib/libTaintedMalloc.so '\'
///         --legacy-tmalloc <bitcode-file>
/// 2.) New Pass Manager:
///      $opt -laod-pass-plugin <BUILD_DIR>/lib/libTaintedMalloc.so '\'
///       -passes=-"tmalloc" <bitcode-file>

#include "llvm/Transforms/Instrumentation/TaintedMalloc.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetFolder.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include <vector>

using namespace llvm;

#define DEBUG_TYPE  "tainted-malloc"

//-----------------------------------------------------------------------------
// TaintedMalloc implementation
//-----------------------------------------------------------------------------


Function &create_sandbox_malloc(Module &M)
{
  LLVMContext &Ctx = M.getContext();


  auto Callee = M.getOrInsertFunction("malloc_in_sandbox",
                                      Type::getVoidTy(Ctx), Type::getInt64Ty(Ctx));

  auto F = dyn_cast<Constant>(Callee.getCallee());
  auto *new_func = cast<Function>(F);
  return *new_func;
}

void addCall(Module &M, Instruction &I, Value* size_param)
{
  IRBuilder<>Bld(&I);
  Bld.CreateCall(&(create_sandbox_malloc(M)), size_param);
  return;
}

static bool Instrument_tainted_malloc(Module& M)
{
  /*
  bool modified = false;
  for (auto &F : M) {
    for(auto &BB : F) {
      for (auto &I : BB) {
        // if this is a call instruction then CB will not be NULL
        auto *CB = dyn_cast<CallBase>(&I);
        if (nullptr == CB) {
          continue;
        }

        auto DirectInvoc = CB->getCalledFunction();
        if (nullptr == DirectInvoc) {
          continue;
        }

        if (CB->getCalledFunction()->getName() == "t_malloc") {
          I.eraseFromParent();
          addCall(M, reinterpret_cast<Instruction &>(I), CB->getArgOperand(1));
          modified = true;
        }
      }
    }
  }
  return modified;*/
}

PreservedAnalyses TaintedMallocPass::run(Function& F,
                                         FunctionAnalysisManager &AM) {
  auto module = F.getParent();
  if (!Instrument_tainted_malloc(*module))
    return PreservedAnalyses::all();
  else
    return PreservedAnalyses::none();
}

namespace {
struct TaintedMallocLegacyPass : public ModulePass {
  static char ID;
  TaintedMallocLegacyPass() : ModulePass(ID) {
    initializeTaintedMallocLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
  //  errs() << "inside runOnFunction;\n";
    return Instrument_tainted_malloc(M);
  }
};
}//namespace

char TaintedMallocLegacyPass::ID = 0;

INITIALIZE_PASS_BEGIN(TaintedMallocLegacyPass, "tainted-malloc",
                      "tainted memory allocation", false, false)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(TaintedMallocLegacyPass, "tainted-malloc",
                    "tainted memory allocation", false, false)

ModulePass *llvm::createTaintedMallocLegacyPass() {
  return new TaintedMallocLegacyPass();
}
