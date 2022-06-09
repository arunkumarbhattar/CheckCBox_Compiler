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
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <vector>
#include <unordered_map>

using namespace llvm;

#define DEBUG_TYPE  "tainted-sbx-invoke"

//-----------------------------------------------------------------------------
// TaintedInvokeInSbx implementation
//-----------------------------------------------------------------------------


Function &create_sandbox_invoke_call(Module &M, Type* RetValType, std::vector<Type*> ParamTys, std::string &generated_function_name)
{
  LLVMContext &Ctx = M.getContext();

  auto Callee = M.getOrInsertFunction(generated_function_name,
                                      FunctionType::get(RetValType,ParamTys, false));

  auto F = dyn_cast<Constant>(Callee.getCallee());
  auto *new_func = cast<Function>(F);
  return *new_func;
}

Value* addSbxInvkCall(Module &M, Instruction &I, std::vector<Value*> args,
                      std::vector<Type*> ParamTys, Type* RetValType, std::string &generated_function_name)
{
  IRBuilder<>Bld(&I);
  return Bld.CreateCall(&(create_sandbox_invoke_call(M, RetValType, ParamTys, generated_function_name)), args);
}

Function& create_sandbox_fetch_offset(Module &M){
  LLVMContext &Ctx = M.getContext();

  Type* CharPtr = const_cast<PointerType *>(Type::getInt8PtrTy(M.getContext()));
  Type* offsetRetTyp = const_cast<IntegerType *>(IntegerType::getInt64Ty(M.getContext()));
  auto Callee = M.getOrInsertFunction("c_fetch_pointer_offset",
                                      offsetRetTyp, CharPtr);
  auto F = dyn_cast<Constant>(Callee.getCallee());
  auto *new_func = cast<Function>(F);
  return *new_func;
}

Function& create_fetch_pointer_from_offset_call(Module &M){
  LLVMContext &Ctx = M.getContext();

  Type* VOIDPtr = const_cast<PointerType *>(Type::getInt8PtrTy(M.getContext()));
  Type* OffsetTyp = const_cast<IntegerType *>(IntegerType::getInt64Ty(M.getContext()));
  auto Callee = M.getOrInsertFunction("c_fetch_pointer_from_offset",
                                      VOIDPtr, OffsetTyp);
  auto F = dyn_cast<Constant>(Callee.getCallee());
  auto *new_func = cast<Function>(F);
  return *new_func;
}

Function& create_sandbox_address_fetch_call(Module& M)
{
  LLVMContext &Ctx = M.getContext();

  Type* VOIDPTR = const_cast<PointerType *>(Type::getInt8PtrTy(M.getContext()));
  auto Callee = M.getOrInsertFunction("c_fetch_sandbox_address",
                                      VOIDPTR);
  auto F = dyn_cast<Constant>(Callee.getCallee());
  auto *new_func = cast<Function>(F);
  return *new_func;
}

Value* addPointerOffsetFetchCall(Module &M, Instruction &I, Value* pointer_name)
{
  IRBuilder<>Bld(&I);
  return Bld.CreateCall(&(create_sandbox_fetch_offset(M)), pointer_name);
}

Function& create_sandbox_heap_fetch_call(Module &M)
{
  LLVMContext &Ctx = M.getContext();
  Type* heap_address_return_type = const_cast<IntegerType*>(IntegerType::getInt64Ty(M.getContext()));
  auto Callee = M.getOrInsertFunction("c_fetch_sandbox_heap_address", heap_address_return_type);
  auto F = dyn_cast<Constant>(Callee.getCallee());
  auto* new_func = cast<Function>(F);
  return *new_func;
}

Value* addSandboxAddressFetchCall(Module &M, Instruction &I)
{
  IRBuilder<> Bld(&I);
  return Bld.CreateCall(&(create_sandbox_address_fetch_call(M)));

}

Value* AddPointerFetchFromOffsetCall(Module &M, Instruction &I, Value* Offset)
{
  IRBuilder<> Bld(&I);
  return Bld.CreateCall(&(create_fetch_pointer_from_offset_call(M)), Offset);
}

Value* AddBitCastOperation(Module &M, Instruction &I, Value* voidPointer, Type* TrueRetValType)
{
  IRBuilder<>Bld(&I);
  return Bld.CreateCast(llvm::Instruction::BitCast ,voidPointer, TrueRetValType);
}
static bool Instrument_tainted_func_call(Module& M) {
  static IRBuilder<> Builder(M.getContext());
  bool modified = false;
  Instruction *inst_to_delete = nullptr;
  Value* Sandbox_return_val;
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

          std::vector<Value *> args;
          std::vector<Type *> ParamTys;

          /*
           * The first argument would be the sandbox address which is technically a void*
           *
           */
          Value* Sandbox_address;
          Sandbox_address = addSandboxAddressFetchCall(M, reinterpret_cast<Instruction &>(I));
          args.push_back(Sandbox_address);
          ParamTys.push_back(Sandbox_address->getType());

          std::string func_name = CB->getCalledFunction()->getName().str();
          std::string generated_function_name = "w2c_"+func_name;
          /*
           * unordered hash map (1) -->
           * key --> pointer_name /////  value --> argument index (To insert it back)
           * unordered hash map (2) -->
           * key --> pointer_name /////  value --> pointer offset
           *
           * In the future we shall have advanced name mangling to handle the scenario of same
           * pointer name within two different lexical scopes (local variables vs global variables
           * with same pointer names)
          */

          std::unordered_map<int, std::string> hash_map_1;
          std::unordered_map<std::string, Value*> hash_map_2;

          int number_of_args = CB->getNumArgOperands();

          // Initialize the argument counter to the number of arguments
          int arg_counter = number_of_args;
          for (auto op = I.op_begin(); op != I.op_end();op++){
            if((arg_counter > 0) && (op->get()->getType()->isPointerTy())) {
              Value *v = op->get();
              StringRef name = getLoadStorePointerOperand(v)->getName().data();
              hash_map_1[op->getOperandNo()] = name.str();
            }
            arg_counter--;
          }

          /*
           * Now in a loop, retrieve all the pointer offsets for the arguments
           */
          for (auto itr = hash_map_1.begin(); itr != hash_map_1.end();  itr++)
          {
            /*
             * Insert call Instructions to fetch the pointer offsets for these pointer names
             *
             */
            Value* returned_pointer_offset;
            Type* CHAR = PointerType::getUnqual(Type::getInt8Ty(M.getContext()));
            // Create a variable that will hold the string

            //Fetch the value object pointer of the argument

            llvm::Constant *Argument = llvm::ConstantDataArray::getString(
                M.getContext(), itr->second.c_str()
            );

            Constant *PrintfFormatStrVar =
                M.getOrInsertGlobal("GlobalSbxOffsetVar", Argument->getType());
            dyn_cast<GlobalVariable>(PrintfFormatStrVar)->setInitializer(Argument);

            llvm::Value *FormatArgument =
                Builder.CreatePointerCast(PrintfFormatStrVar, CHAR, "OffsetVar");

            Argument->setName("argument");
            //Set the value to this from the constant we declared above
            //Issue call and start storing the return offset into the second hash_map
            returned_pointer_offset = addPointerOffsetFetchCall(M, reinterpret_cast<Instruction &>(I), FormatArgument);
            hash_map_2[itr->second.c_str()] = returned_pointer_offset;
          }

          /*
           * Now loop through the arguments and create a argument list needed to issue the call to the sandbox
           *
           */
          for (int i = 0; i < number_of_args; i++) {
            /*
             * If the argument type is a pointer offset,
             * then fetch and push back the pointer offset value
             */
            if(hash_map_1.find(i) != hash_map_1.end())
            {
              //Generate a Value* from the value
              Type* Int64 = const_cast<IntegerType *>(IntegerType::getInt64Ty(M.getContext()));
              args.push_back(hash_map_2[hash_map_1[i]]);
              ParamTys.push_back(Int64);
            }
            else {
              args.push_back(CB->getArgOperand(i));
              ParamTys.push_back(CB->getArgOperand(i)->getType());
            }
          }

          //Now you have the proper argument list ready --> pass it on and use it to invoke the sandbox call
          // if function returns a pointer value, then the return type for the sandbox invoke function would be a int64
          // pointer offset, hence accordingly change the return type
          Type* retValType;
          Type* TrueretValType = CB->getCalledFunction()->getReturnType();

          if (CB->getCalledFunction()->getReturnType()->isPointerTy())
          {
            retValType = const_cast<IntegerType*>(IntegerType::getInt64Ty(M.getContext()));
          }
          else
          {
            retValType = CB->getCalledFunction()->getReturnType();
          }

          Sandbox_return_val = addSbxInvkCall(M, reinterpret_cast<Instruction &>(I), args, ParamTys,
                         retValType, generated_function_name);

          /*
           * Check if retValType is a pointer or not, if it is a pointer, then you need to return appropriate type,
           * else we are done here
           */
          if (TrueretValType->isPointerTy()){
            /*
             * Now you need to the following additional calls
             * 1.) fetch a call to retrieve the pointer value(void*) from pointer offset (u32)
             * 2.) insert a call to bitcast the returned void* to funciton return pointer type
             */
            // Adding support for call 1
            Value* returned_void_pointer;
            returned_void_pointer = AddPointerFetchFromOffsetCall(M,
                                                                        reinterpret_cast<Instruction &>(I)
                                                                        , Sandbox_return_val);
            // Adding support for call 2
            Sandbox_return_val = AddBitCastOperation(M, reinterpret_cast<Instruction&>(I), returned_void_pointer, TrueretValType);
          }
          inst_to_delete = &I;
          modified = true;
        }
      }
    }
  }
    if(inst_to_delete !=nullptr) {
        inst_to_delete->replaceAllUsesWith(Sandbox_return_val);
        inst_to_delete->eraseFromParent();
    }
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
