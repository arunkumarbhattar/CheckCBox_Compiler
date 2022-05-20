//
// Created by arun on 5/16/22.
//
/// FILE:
/// TaintedMalloc.cpp

/// DESCRIPTION:
/// For every call to t_malloc function in the input IR module, TaintedMallocPass
/// inserts a call to malloc_in_sandbox with the appropriate operand parameter,
/// following which the original t_malloc function call gets removed.

/// USAGE:
/// 1.) Legacy Pass manager:
///     $opt -load <BUILD_DIR>/lib/libTaintedMalloc.so '\'
///         --legacy-tmalloc <bitcode-file>
/// 2.) New Pass Manager:
///      $opt -load-pass-plugin <BUILD_DIR>/lib/libTaintedMalloc.so '\'
///       -passes=-"tmalloc" <bitcode-file>

#include "llvm/Transforms/Instrumentation/TaintedMalloc.h"
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
#include "llvm/Transforms/Instrumentation/sandbox_interface.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <vector>

using namespace llvm;

#define DEBUG_TYPE  "tainted-malloc"

//-----------------------------------------------------------------------------
// TaintedMalloc implementation
//-----------------------------------------------------------------------------


Function &create_sandbox_malloc(Module &M)
{
  LLVMContext &Ctx = M.getContext();

  Type* VOID_Ptr = const_cast<PointerType*>(Type::getInt8PtrTy(M.getContext()));
  auto Callee = M.getOrInsertFunction("malloc_in_sandbox",
                                      VOID_Ptr, Type::getInt64Ty(Ctx));

  auto F = dyn_cast<Constant>(Callee.getCallee());
  auto *new_func = cast<Function>(F);
  return *new_func;
}

Value* addCall(Module &M, Instruction &I, Value* size_param)
{
  IRBuilder<>Bld(&I);
  return Bld.CreateCall(&(create_sandbox_malloc(M)), size_param);
}

static bool Instrument_tainted_malloc(Module& M)
{
  static IRBuilder<> Builder(M.getContext());
  bool modified = false;
  Value* RetVal;
  Instruction* inst_to_delete = nullptr;
  for (auto &F : M) {
    for(auto &BB : F) {
      for (auto &I : BB) {
        // if this is a call instruction then CB will not be NULL
        auto *CB = dyn_cast<CallBase>(&I);

        auto *BC = dyn_cast<BitCastOperator>(&I);

        if ((nullptr == CB) && (BC == nullptr)) {
          continue;
        }


        if(BC != nullptr)
        {
          auto *CB_tmp = dyn_cast<CallBase>(I.getPrevNonDebugInstruction(true));
          if((CB_tmp != nullptr) && (CB_tmp->getCalledFunction()->getName() == "t_malloc"))
          {
            I.setOperand(0, RetVal);
            continue;
          }
          else
            continue;
        }

        auto DirectInvoc = CB->getCalledFunction();
        if (nullptr == DirectInvoc) {
          continue;
        }

        if (CB->getCalledFunction()->getName() == "t_malloc") {
          RetVal =
              addCall(M, reinterpret_cast<Instruction &>(I), CB->getArgOperand(0));
          inst_to_delete = &I;
          modified = true;
        }
      }
    }
  }
  if(inst_to_delete != nullptr)
    inst_to_delete->eraseFromParent();
  return modified;
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
