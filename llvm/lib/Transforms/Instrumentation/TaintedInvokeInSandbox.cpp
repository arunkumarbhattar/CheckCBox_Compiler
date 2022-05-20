//
// Created by arun on 5/20/22.
//
/// FILE:
/// TaintedInvokeInSandbox.cpp

/// DESCRIPTION:
/// For every call to a function that has a tainted attribute in the input IR module,
/// TaintedInvokeInSbxPass
/// inserts a call to execute_sandbox_function with the appropriate operands,
/// following which the original call to the function call gets removed.


#include "llvm/Transforms/Instrumentation/TaintedInvokeInSandbox.h"
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

#define DEBUG_TYPE  "tainted-sbx-invoke"

//-----------------------------------------------------------------------------
// TaintedInvokeInSbx implementation
//-----------------------------------------------------------------------------


Function &create_sandbox_invoke_call(Module &M, Type* RetValType, std::vector<Type*> ParamTys)
{
  LLVMContext &Ctx = M.getContext();

  auto Callee = M.getOrInsertFunction("execute_sandbox_function",
                                      FunctionType::get(RetValType,ParamTys, false));

  auto F = dyn_cast<Constant>(Callee.getCallee());
  auto *new_func = cast<Function>(F);
  return *new_func;
}

Value* addSbxInvkCall(Module &M, Instruction &I, std::vector<Value*> args,
                      std::vector<Type*> ParamTys, Type* RetValType)
{
  IRBuilder<>Bld(&I);
  return Bld.CreateCall(&(create_sandbox_invoke_call(M, RetValType, ParamTys)), args);
}

static bool Instrument_tainted_func_call(Module& M) {
  static IRBuilder<> Builder(M.getContext());
  bool modified = false;
  Instruction *inst_to_delete = nullptr;
  for (auto &F : M) {
    for (auto &BB : F) {
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

        auto isCalledFnTainted =
            CB->getCalledFunction()->getFnAttribute(Attribute::Tainted);
        if (isCalledFnTainted.isValid()) {

          // Create a vector of operands -->
          std::vector<Value *> args;
          std::vector<Type *> ParamTys;

          int number_of_args = CB->getNumArgOperands();
          for (int i = 0; i < number_of_args; i++) {
            args.push_back(CB->getArgOperand(i));
            ParamTys.push_back(CB->getArgOperand(i)->getType());
          }
          Type *retValType = CB->getCalledFunction()->getReturnType();

          addSbxInvkCall(M, reinterpret_cast<Instruction &>(I), args, ParamTys,
                         retValType);
          inst_to_delete = &I;
          modified = true;
        }
      }
    }
  }
    if(inst_to_delete !=nullptr)
      inst_to_delete->eraseFromParent();
    return modified;
  }


  PreservedAnalyses TaintedInvokeInSandboxPass::run(
      Function & F, FunctionAnalysisManager & AM) {
    auto module = F.getParent();
    if (!Instrument_tainted_func_call(*module))
      return PreservedAnalyses::all();
    else
      return PreservedAnalyses::none();
  }

  namespace {
  struct TaintedInvokeInSandboxLegacyPass : public ModulePass {
    static char ID;
    TaintedInvokeInSandboxLegacyPass() : ModulePass(ID) {
      initializeTaintedInvokeInSandboxLegacyPassPass(
          *PassRegistry::getPassRegistry());
    }

    bool runOnModule(Module &M) override {
      //  errs() << "inside runOnFunction;\n";
      return Instrument_tainted_func_call(M);
    }
  };
  } // namespace

  char TaintedInvokeInSandboxLegacyPass::ID = 0;

  INITIALIZE_PASS_BEGIN(TaintedInvokeInSandboxLegacyPass, "tainted-sbx-invoke",
                        "Execute Function in Sandbox", false, false)
  INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
  INITIALIZE_PASS_END(TaintedInvokeInSandboxLegacyPass, "tainted-sbx-invoke",
                      "Execute Function in Sandbox", false, false)

  ModulePass *llvm::createTaintedInvokeInSandboxLegacyPass() {
    return new TaintedInvokeInSandboxLegacyPass();
  }
