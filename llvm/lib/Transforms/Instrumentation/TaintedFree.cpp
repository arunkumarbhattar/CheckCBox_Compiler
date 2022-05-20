//
// Created by arun on 5/16/22.
//
/// FILE:
/// TaintedFree.cpp

/// DESCRIPTION:
/// For every call to t_free function in the input IR module, TaintedFreePass
/// inserts a call to free_in_sandbox with the appropriate operand parameter,
/// following which the original t_free function call gets removed.

/// USAGE:
/// 1.) Legacy Pass manager:
///     $opt -load <BUILD_DIR>/lib/libTaintedFree.so '\'
///         --legacy-tmalloc <bitcode-file>

#include "llvm/Transforms/Instrumentation/TaintedFree.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetFolder.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Value.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Instrumentation/sandbox_interface.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <vector>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

using namespace llvm;

#define DEBUG_TYPE  "tainted-free"

//-----------------------------------------------------------------------------
// TaintedFree implementation
//-----------------------------------------------------------------------------


Function &create_sandbox_free(Module &M)
{
  LLVMContext &Ctx = M.getContext();

  Type* VOID = const_cast<Type*>(Type::getVoidTy(M.getContext()));

  auto Callee = M.getOrInsertFunction("free_in_sandbox",
                                      VOID, Type::getInt8PtrTy(Ctx));

  auto F = dyn_cast<Constant>(Callee.getCallee());
  auto *new_func = cast<Function>(F);
  return *new_func;
}

Value* addFreeCall(Module &M, Instruction &I, Value* pointer)
{
  IRBuilder<>Bld(&I);
  return Bld.CreateCall(&(create_sandbox_free(M)), pointer);
}

static bool Instrument_tainted_free(Module& M)
{
  static IRBuilder<> Builder(M.getContext());
  bool modified = false;
  Instruction* inst_to_delete;
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

        if (CB->getCalledFunction()->getName() == "t_free") {
          addFreeCall(M, reinterpret_cast<Instruction &>(I), CB->getArgOperand(0));
          inst_to_delete = &I;
          modified = true;
        }
      }
    }
  }
  inst_to_delete->eraseFromParent();
  return modified;
}

PreservedAnalyses TaintedFreePass::run(Function& F,
                                         FunctionAnalysisManager &AM) {
  auto module = F.getParent();
  if (!Instrument_tainted_free(*module))
    return PreservedAnalyses::all();
  else
    return PreservedAnalyses::none();
}

namespace {
struct TaintedFreeLegacyPass : public ModulePass {
  static char ID;
  TaintedFreeLegacyPass() : ModulePass(ID) {
    initializeTaintedFreeLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    //  errs() << "inside runOnFunction;\n";
    return Instrument_tainted_free(M);

  }
};
}//namespace

char TaintedFreeLegacyPass::ID = 0;

INITIALIZE_PASS_BEGIN(TaintedFreeLegacyPass, "tainted-free",
                      "tainted memory free", false, false)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(TaintedFreeLegacyPass, "tainted-free",
                    "tainted memory free", false, false)

ModulePass *llvm::createTaintedFreeLegacyPass() {
  return new TaintedFreeLegacyPass();
}
