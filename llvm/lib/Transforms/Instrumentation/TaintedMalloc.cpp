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

#include "TaintedMalloc.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"

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

namespace {
struct TaintedMallocLegacyPass : public ModulePass {
  static char ID;
  TaintedMallocLegacyPass() : ModulePass(ID) {
    initializeTaintedMallocLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override{
    bool modified = false;
    for (auto &BB : M) {
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
    return modified;
  }
};

} //namespace

char TaintedMallocLegacyPass::ID = 0;

INITIALIZE_PASS_BEGIN(TaintedMallocLegacyPass, "tainted-malloc",
                      "tainted memory allocation", false, false)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(TaintedMallocLegacyPass, "tainted-malloc",
                    "tainted memory allocation", false, false)

ModulePass *llvm::createTaintedMallocLegacyPass() {
  return new TaintedMallocLegacyPass();
}
