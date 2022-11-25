//===--- CGDynamicCheck.cpp - Emit LLVM Code for Checked C Dynamic Checks -===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This contains code to emit Checked C Dynamic Checks as LLVM code.
//
//===----------------------------------------------------------------------===//

#include "CodeGenFunction.h"
#include "llvm/ADT/Statistic.h"

using namespace clang;
using namespace CodeGen;
using namespace llvm;

#define DEBUG_TYPE "DynamicCheckCodeGen"

namespace {
  STATISTIC(NumDynamicChecksElided,
              "The # of dynamic checks elided (due to constant folding)");
  STATISTIC(NumDynamicChecksInserted,
              "The # of dynamic checks inserted");
  STATISTIC(NumDynamicChecksExplicit,
              "The # of dynamic _Dynamic_check(cond) checks found");
  STATISTIC(NumDynamicChecksNonNull,
              "The # of dynamic non-null checks found");

  STATISTIC(NumDynamicChecksTainted, "The # of dynamic tainted ptr memory checks found");

  STATISTIC(NumDynamicChecksOverflow,
              "The # of dynamic overflow checks found");
  STATISTIC(NumDynamicChecksRange,
              "The # of dynamic bounds checks found");
  STATISTIC(NumDynamicChecksCast,
              "The # of dynamic cast checks found");
}

//
// Expression-specific dynamic check insertion
//

void CodeGenFunction::EmitExplicitDynamicCheck(const Expr *Condition) {
  if (!getLangOpts().CheckedC)
    return;

  ++NumDynamicChecksExplicit;

  // Emit Check
  Value *ConditionVal = EvaluateExprAsBool(Condition);
  EmitDynamicCheckBlocks(ConditionVal);
}

//
// General Functions for inserting dynamic checks
//

static bool shouldEmitNonNullCheck(const CodeGenModule &CGM,
                                   const QualType BaseTy) {
  if (!CGM.getLangOpts().CheckedC)
    return false;

  if (!(BaseTy->isCheckedPointerType() || BaseTy->isCheckedArrayType()))
    return false;

  return true;
}

static bool shouldEmitTaintedPtrDerefAdaptor(const CodeGenModule &CGM,
                                            const QualType BaseTy) {
    if(!CGM.getLangOpts().CheckedC)
        return false;
    //if(!CGM.getLangOpts().TaintedC)
    //return false;

    if((!(BaseTy->isTaintedPointerType() || BaseTy->isTaintedStructureType())
          || BaseTy->isFunctionType() || BaseTy->isFunctionPointerType()))
        return false;

    return true;

}

static bool shouldEmitTaintedPtrDerefAdaptor(const CodeGenModule &CGM,
                                             const llvm::Type* BaseTy) {
  if(!CGM.getLangOpts().CheckedC)
    return false;

  if (BaseTy->isPointerTy() && BaseTy->getCoreElementType()->isVoidTy())
    return false;

  if((!(BaseTy->isTaintedPtrTy() || BaseTy->isTStructTy())
       || BaseTy->isFunctionTy()))
    return false;

  return true;

}

void CodeGenFunction::EmitDynamicNonNullCheck(const Address BaseAddr,
                                              const QualType BaseTy) {
  if (!shouldEmitNonNullCheck(CGM, BaseTy))
    return;

  ++NumDynamicChecksNonNull;

  Value *ConditionVal = Builder.CreateIsNotNull(BaseAddr.getPointer(),
                                                "_Dynamic_check.non_null");
  EmitDynamicCheckBlocks(ConditionVal);
}

Value* CodeGenFunction::EmitTaintedPtrDerefAdaptor(const Address BaseAddr,
                                                const llvm::Type* BaseTy){
    if(!shouldEmitTaintedPtrDerefAdaptor
            (CGM, BaseTy))
        return NULL;

    ++NumDynamicChecksTainted;

    return EmitDynamicTaintedPtrAdaptorBlock(BaseAddr);
}

Value* CodeGenFunction::EmitTaintedPtrDerefAdaptor(const Address BaseAddr,
                                                   const QualType BaseTy){
  if(!shouldEmitTaintedPtrDerefAdaptor
      (CGM, BaseTy))
    return NULL;

  ++NumDynamicChecksTainted;

  return EmitDynamicTaintedPtrAdaptorBlock(BaseAddr);
}


/*
 * This Function is introduced to insert Tainted Pointer Dereference
 * Instrumentation against Call Arguments to a function that does not have
 * Explicit Tainted Argument Types as function parameters.
 *
 * The user might still be passing Tainted Pointers because it is quite
 * possible that function arguments might of itype accepting Tainted Pointers.
 *
 * Hence, We do the following.
 * 1.) We check if the Pointer is a Tainted Pointer offset or Not.
 * 2.) If it is Offset, we return pointer, else we return whatever we receive
 * (This way we dont break, if the passed pointer argument is a checked region
 * pointer)
 */

Value* CodeGenFunction::EmitConditionalTaintedPtrDerefAdaptor(Value* Base){
  ++NumDynamicChecksTainted;
  llvm::Type* OriginalType = Base->getType();
  if (!Base->getType()->isPointerTy())
    return NULL;

  Value *OffsetVal = Builder.CreatePtrToInt(
      Base,
      llvm::Type::getInt64Ty(Base->getContext()));
  llvm::Value* ConvPtr = Builder.CreateDerefCondlTaintedPtr(OffsetVal,
                                                   "_Dynamic_check.tainted_pointer");
  /*
   * Returned Ptr is of type i8* (void*) , hence cast it back to original type.
   */
  return Builder.CreatePointerCast(ConvPtr, OriginalType);
}

Value* CodeGenFunction::EmitConditionalTaintedP2OAdaptor(Value* Base){
  ++NumDynamicChecksTainted;
  llvm::Type* OriginalType = Base->getType();
  if (!Base->getType()->isPointerTy())
    return NULL;

  Value *OffsetVal = Builder.CreatePointerCast(
      Base,
      llvm::Type::getInt8PtrTy(Base->getContext()));
  llvm::Value* ConvPtr = Builder.CreatePtrToInt(OffsetVal, llvm::Type::getInt32Ty(Base->getContext()));
  /*
   * Returned Ptr is of type unsigned int , hence cast it back to original type.
   */
  llvm::Value* RetVal = Builder.CreateIntToPtr(ConvPtr, OriginalType);
  Address TempAlloca = CreateTempAllocaWithoutCast(OriginalType, CharUnits::Four()); //Returned Pointer must align to N/2 bytes for tainted pointers.
  Builder.CreateStore(RetVal, TempAlloca);
  //Load from Alloca and return
  auto LoadVal =  Builder.CreateLoad(TempAlloca);
  return Builder.CreatePointerCast(LoadVal, RetVal->getType());
}

void CodeGenFunction::EmitDynamicNonNullCheck(Value *Val,
                                              const QualType BaseTy) {
  if (!shouldEmitNonNullCheck(CGM, BaseTy))
    return;

  ++NumDynamicChecksNonNull;

  Value *ConditionVal = Builder.CreateIsNotNull(Val,
                                                "_Dynamic_check.non_null");
  EmitDynamicCheckBlocks(ConditionVal);
}

// TODO: This is currently unused. It may never be used.
void CodeGenFunction::EmitDynamicOverflowCheck(const Address BaseAddr,
                                               const QualType BaseTy,
                                               const Address PtrAddr) {
  if (!getLangOpts().CheckedC)
    return;

  ++NumDynamicChecksOverflow;

  // EmitDynamicCheckBlocks(Condition);
}

void CodeGenFunction::EmitDynamicBoundsCheck(const Address PtrAddr,
                                             const BoundsExpr *Bounds,
                                             BoundsCheckKind CheckKind,
                                             llvm::Value *Val) {
  if (!getLangOpts().CheckedC)
    return;

  if (!Bounds)
    return;

  if (Bounds->isAny() || Bounds->isInvalid())
    return;

  // We'll insert the bounds check for an assignment through a null-terminated
  // pointer later, when we know the value.
  if (CheckKind == BoundsCheckKind::BCK_NullTermWriteAssign && !Val)
    return;

  // We can only generate the check if we have the bounds as a range.
  if (!isa<RangeBoundsExpr>(Bounds)) {
    llvm_unreachable(
      "Can Only Emit Dynamic Bounds Check For RangeBounds Exprs");
    return;
  }

  llvm::Value *TaintedPtrFromOffset = PtrAddr.getPointer();
  //TaintedPtrFromOffset = EmitConditionalTaintedPtrDerefAdaptor(TaintedPtrFromOffset);
  TaintedPtrFromOffset = EmitConditionalTaintedP2OAdaptor(TaintedPtrFromOffset);
  if(TaintedPtrFromOffset == NULL)
    TaintedPtrFromOffset = PtrAddr.getPointer();
  const RangeBoundsExpr *BoundsRange = dyn_cast<RangeBoundsExpr>(Bounds);

  ++NumDynamicChecksRange;

  // Emit the code to generate the pointer values
  Address Lower = EmitPointerWithAlignment(BoundsRange->getLowerExpr());

  // We don't infer an expression with the correct cast for
  // multidimensional array access, but icmp requires that
  // its operands are of the same type, so we bitcast Lower to
  // match the type of TaintedPtrFromOffset at the LLVM IR Level.
  if (Lower.getType() != TaintedPtrFromOffset->getType())
    Lower = Builder.CreateBitCast(Lower, TaintedPtrFromOffset->getType());

  Address Upper = EmitPointerWithAlignment(BoundsRange->getUpperExpr());
  /*
   * This is Dummied as its screwed
   * If the above is a GEP instruction
   * and if the GEP is on a double/triple pointer type, modify the GEP
   * to GEP on i32* (only if the variable of decoyed type)
   */

  if (isa<GEPOperator>(Upper.getPointer()))
  {
    llvm::GEPOperator* GEP = dyn_cast<GEPOperator>(Upper.getPointer());
    auto GEPOp = GEP->getPointerOperandType();
    if(GEPOp->isPointerTy() && GEPOp->getCoreElementType()->isDecoyed())
    {
      llvm::Type* PtrTy = GEP->getPointerOperandType();
      llvm::Type* IntPtrTy = llvm::Type::getInt32PtrTy(CGM.getLLVMContext());
      llvm::Type* IntTy = llvm::Type::getInt32Ty(CGM.getLLVMContext());
      llvm::Value* NewPtr = Builder.CreatePointerCast(GEP->getPointerOperand(), IntPtrTy);
      ArrayRef<llvm::Value *> indices(
          reinterpret_cast<Value *const *>(GEP->idx_begin()),
          reinterpret_cast<Value *const *>(GEP->idx_end()));
      llvm::Value* NewGEP = Builder.CreateInBoundsGEP(IntTy, NewPtr, indices[0]);
      llvm::Value* OriginalTypeVal = Builder.CreatePointerCast(NewGEP, PtrTy);
      Upper = Address(OriginalTypeVal, Upper.getAlignment());
    }
  }

  // As above, we may need to bitcast Upper to match the type
  // of TaintedPtrFromOffset at the LLVM IR Level.
  if (Upper.getType() != TaintedPtrFromOffset->getType())
    Upper = Builder.CreateBitCast(Upper, TaintedPtrFromOffset->getType());

  llvm::Value *TaintedUpperPtrFromOffset = Upper.getPointer();
  //TaintedUpperPtrFromOffset = EmitConditionalTaintedPtrDerefAdaptor(TaintedUpperPtrFromOffset);
  TaintedUpperPtrFromOffset = EmitConditionalTaintedP2OAdaptor(TaintedUpperPtrFromOffset);
  if(TaintedUpperPtrFromOffset == NULL)
    TaintedUpperPtrFromOffset = Upper.getPointer();

  llvm::Value *TaintedLowerPtrFromOffset = Lower.getPointer();
  //TaintedLowerPtrFromOffset = EmitConditionalTaintedPtrDerefAdaptor(TaintedLowerPtrFromOffset);
  TaintedLowerPtrFromOffset = EmitConditionalTaintedP2OAdaptor(TaintedLowerPtrFromOffset);
  if(TaintedLowerPtrFromOffset == NULL)
    TaintedLowerPtrFromOffset = Lower.getPointer();

  // Make the lower check
  Value *LowerChk = Builder.CreateICmpULE(
      TaintedLowerPtrFromOffset, TaintedPtrFromOffset, "_Dynamic_check.lower");

  // Make the upper check
  Value *UpperChk;
  assert(CheckKind != BCK_None);
  if (CheckKind != BCK_NullTermRead)
    UpperChk = Builder.CreateICmpULT(TaintedPtrFromOffset, TaintedUpperPtrFromOffset,
                                     "_Dynamic_check.upper");
  else
    // For reads of null-terminated pointers, we allow the element exactly
    // at the upper bound to be read.
    UpperChk = Builder.CreateICmpULE(TaintedPtrFromOffset, TaintedUpperPtrFromOffset,
                                     "_Dynamic_check.upper");
  llvm::Value *Condition =
    Builder.CreateAnd(LowerChk, UpperChk, "_Dynamic_check.range");
  if (const ConstantInt *ConditionConstant = dyn_cast<ConstantInt>(Condition)) {
    if (ConditionConstant->isOne())
      return;
  }
  BasicBlock *DyCkSuccess = createBasicBlock("_Dynamic_check.succeeded");
  BasicBlock *DyCkFailure;
  if (CheckKind == BCK_NullTermWriteAssign)
    DyCkFailure = EmitNulltermWriteAdditionalCheck(Address(TaintedPtrFromOffset, getPointerAlign()) ,
                                                   Address(TaintedUpperPtrFromOffset, getPointerAlign()), LowerChk,
                                                   Val, DyCkSuccess);
  else
    DyCkFailure = EmitDynamicCheckFailedBlock();
  Builder.CreateCondBr(Condition, DyCkSuccess, DyCkFailure);
  // This ensures the success block comes directly after the branch
  EmitBlock(DyCkSuccess);
  Builder.SetInsertPoint(DyCkSuccess);
}

void
CodeGenFunction::EmitDynamicBoundsCastCheck(const Address BaseAddr,
                                            const BoundsExpr *CastBounds,
                                            const BoundsExpr *SubExprBounds) {
  if (!getLangOpts().CheckedC)
    return;

  if (!SubExprBounds || !CastBounds)
    return;

  if (SubExprBounds->isAny() || SubExprBounds->isInvalid())
    return;

  // SubExprBounds can be Any by inference but CastBounds can't be Any
  assert(!CastBounds->isAny());
  if (CastBounds->isInvalid())
    return;

  // We can only generate the check if we have the bounds as a range.
  if (!isa<RangeBoundsExpr>(SubExprBounds) ||
      !isa<RangeBoundsExpr>(CastBounds)) {
    llvm_unreachable(
        "Can Only Emit Dynamic Bounds Check For RangeBounds Exprs");
    return;
  }

  const RangeBoundsExpr *SubRange = dyn_cast<RangeBoundsExpr>(SubExprBounds);
  const RangeBoundsExpr *CastRange = dyn_cast<RangeBoundsExpr>(CastBounds);

  ++NumDynamicChecksCast;

  // Emits code as follows:
  //
  // %entry:
  //   ... (prior code)
  //   %is_null = %base == null
  //   br i1 %is_null, %success, %subsumption
  // %subsumption:
  //   %subsumes = (%lower <= %cast_lower && %cast_upper <= %upper)
  //   br i1 %subsumes, %success, %failure
  // %success:
  //   ... (following code)
  // %failure:
  //   trap()

  Value *IsNull =
      Builder.CreateIsNull(BaseAddr.getPointer(), "_Dynamic_check.is_null");

  // Constant Folding:
  // If IsNull is true (one), then we don't need to insert the rest
  // of the check, as computation should continue without inserting
  // the branch.
  if (const ConstantInt *IsNullConstant = dyn_cast<ConstantInt>(IsNull)) {
    if (IsNullConstant->isOne()) {
      ++NumDynamicChecksElided;

      // We have not emitted any blocks or any branches so far,
      // so we can just return here, which leaves the Builder
      // in the right position to add instructions to the end of
      // the entry block.
      //
      // The code will look like:
      // %entry:
      //   ... (prior code)
      //   %is_null = true
      //   ... (following code)
      // (No %failure Block)

      return;
    }
  }

  BasicBlock *DyCkSubsumption = createBasicBlock("_Dynamic_check.subsumption");
  BasicBlock *DyCkSuccess = createBasicBlock("_Dynamic_check.success");

  // Insert the IsNull Branch
  Builder.CreateCondBr(IsNull, DyCkSuccess, DyCkSubsumption);

  // This ensures the subsumption block comes directly after the IsNull branch
  EmitBlock(DyCkSubsumption);

  Builder.SetInsertPoint(DyCkSubsumption);

  // SubRange - bounds(lb, ub) vs CastRange - bounds(castlb, castub)
  // Dynamic_check(lb <= castlb && castub <= ub)
  // If required, we will be bitcasting castlb and castub at the
  // LLVM IR level to match the types of lb and ub respectively.

  // Emit the code to generate pointers for SubRange, lb and ub
  Address Lower = EmitPointerWithAlignment(SubRange->getLowerExpr());
  Address Upper = EmitPointerWithAlignment(SubRange->getUpperExpr());

  // Emit the code to generate pointers for CastRange, castlb and castub

  Address CastLower = EmitPointerWithAlignment(CastRange->getLowerExpr());
  // We will be comparing CastLower to Lower. Their types may not match,
  // so we're going to bitcast CastLower to match the type of Lower if needed.
  if (CastLower.getType() != Lower.getType())
    CastLower = Builder.CreateBitCast(CastLower, Lower.getType());

  Address CastUpper = EmitPointerWithAlignment(CastRange->getUpperExpr());
  // Again we're going to bitcast CastUpper to match the type of Upper
  // if needed.
  if (CastUpper.getType() != Upper.getType())
    CastUpper = Builder.CreateBitCast(CastUpper, Upper.getType());

  llvm::Value *TaintedPtrFromOffsetLower = Lower.getPointer();
  //TaintedPtrFromOffsetLower = EmitConditionalTaintedPtrDerefAdaptor(TaintedPtrFromOffsetLower);
  TaintedPtrFromOffsetLower = EmitConditionalTaintedP2OAdaptor(TaintedPtrFromOffsetLower);
  if(TaintedPtrFromOffsetLower == NULL)
    TaintedPtrFromOffsetLower = Lower.getPointer();

  llvm::Value *TaintedPtrFromOffsetCastLower = CastLower.getPointer();
  //TaintedPtrFromOffsetCastLower = EmitConditionalTaintedPtrDerefAdaptor(TaintedPtrFromOffsetCastLower);
  TaintedPtrFromOffsetCastLower = EmitConditionalTaintedP2OAdaptor(TaintedPtrFromOffsetCastLower);
  if(TaintedPtrFromOffsetCastLower == NULL)
    TaintedPtrFromOffsetCastLower = CastLower.getPointer();

  // Make the lower check (Lower <= CastLower)
  Value *LowerChk = Builder.CreateICmpULE(
      TaintedPtrFromOffsetLower, TaintedPtrFromOffsetCastLower, "_Dynamic_check.lower");

  llvm::Value *TaintedPtrFromOffsetCastUpper = CastUpper.getPointer();
  //TaintedPtrFromOffsetCastUpper = EmitConditionalTaintedPtrDerefAdaptor(TaintedPtrFromOffsetCastUpper);
  TaintedPtrFromOffsetCastUpper = EmitConditionalTaintedP2OAdaptor(TaintedPtrFromOffsetCastUpper);
  if(TaintedPtrFromOffsetCastUpper == NULL)
    TaintedPtrFromOffsetCastUpper = CastUpper.getPointer();

  llvm::Value *TaintedPtrFromOffsetUpper = Upper.getPointer();
  //TaintedPtrFromOffsetUpper = EmitConditionalTaintedPtrDerefAdaptor(TaintedPtrFromOffsetUpper);
  TaintedPtrFromOffsetUpper = EmitConditionalTaintedP2OAdaptor(TaintedPtrFromOffsetUpper);
  if(TaintedPtrFromOffsetUpper == NULL)
    TaintedPtrFromOffsetUpper = Upper.getPointer();


  // Make the upper check (CastUpper <= Upper)
  Value *UpperChk = Builder.CreateICmpULE(
      TaintedPtrFromOffsetCastUpper, TaintedPtrFromOffsetUpper, "_Dynamic_check.upper");

  // Make Both Checks
  Value *CastCond =
      Builder.CreateAnd(LowerChk, UpperChk, "_Dynamic_check.cast");

  // Constant Folding:
  // If CastCond is true (one), then we need to insert a direct branch
  // to the success block, emit it, and set the insert point there for
  // further code generation.
  if (const ConstantInt *CastCondConstant = dyn_cast<ConstantInt>(CastCond)) {
    if (CastCondConstant->isOne()) {
      ++NumDynamicChecksElided;

      // We have emitted a branch to the failure block, along with the
      // failure block, so we have to emit a direct branch to success,
      //
      // The code will look like this:
      // %entry:
      //   ... (prior code)
      //   %is_null = %base == null
      //   br i1 %is_null, %success, %subsumption
      // %subsumption:
      //   %subsumes = true
      //   br %success
      // %success:
      //   ... (following code)
      // (No %failure Block)

      // This check will always pass, directly jump to the success block.
      Builder.CreateBr(DyCkSuccess);

      // This ensures the success block comes directly after the subsumption
      // branch
      EmitBlock(DyCkSuccess);
      Builder.SetInsertPoint(DyCkSuccess);

      return;
    }
  }

  ++NumDynamicChecksInserted;

  BasicBlock *DyCkFail = EmitDynamicCheckFailedBlock();

  // Insert the CastCond Branch
  Builder.CreateCondBr(CastCond, DyCkSuccess, DyCkFail);

  // This ensures the success block comes directly after the subsumption branch
  EmitBlock(DyCkSuccess);
  Builder.SetInsertPoint(DyCkSuccess);
}

void CodeGenFunction::EmitDynamicCheckBlocks(Value *Condition) {
  assert(Condition->getType()->isIntegerTy(1) &&
         "May only dynamic check boolean conditions");

  // Constant Folding:
  // If we have generated a constant condition, and the condition is true,
  // then the check will always pass and we can elide it.
  if (const ConstantInt *ConditionConstant = dyn_cast<ConstantInt>(Condition)) {
    if (ConditionConstant->isOne()) {
      ++NumDynamicChecksElided;
      return;
    }
  }

  ++NumDynamicChecksInserted;

  BasicBlock *DyCkSuccess = createBasicBlock("_Dynamic_check.succeeded");
  BasicBlock *DyCkFail = EmitDynamicCheckFailedBlock();

  Builder.CreateCondBr(Condition, DyCkSuccess, DyCkFail);
  // This ensures the success block comes directly after the branch
  EmitBlock(DyCkSuccess);
  Builder.SetInsertPoint(DyCkSuccess);
}

Value*
CodeGenFunction::EmitDynamicTaintedPtrAdaptorBlock(const Address BaseAddr) {

    ++NumDynamicChecksTainted;

    // Emits code as follows:
    //
    // %entry:
    //   ... (prior code)
    // %0 = base_addr
    // %1 = ptrtoint base_ty* %0 to i32
    // %conv = sext i32 %1 to i64 THis instruction can be made to be avoided
    // %call = call i8* @c_fetch_pointer_from_offset(i64 %conv)
    // %LHS = bitcast i8* %call to base_ty*
    llvm::Type * BitCastType = BaseAddr.getType();
    if (BitCastType->isTStructTy() || (BitCastType->isPointerTy() &&
                                       BitCastType->getCoreElementType()->isTStructTy())) {
      BitCastType =
          ChangeStructName(reinterpret_cast<StructType *>(BitCastType->getCoreElementType()));
      if ((BitCastType != NULL) && (BitCastType->isDecoyed()))
      {
        llvm::Type* CurrentTypeReferences =  BaseAddr.getType();
        while (CurrentTypeReferences->isPointerTy()) {
          CurrentTypeReferences = CurrentTypeReferences->getPointerElementType();
          BitCastType = BitCastType->getPointerTo(0);
        }
        BaseAddr.setType(BitCastType);
      }
      else
        BitCastType = BaseAddr.getType();
    }
    Value *PointerVal = BaseAddr.getPointer();

    if (!BaseAddr.getType()->getCoreElementType()->isFunctionTy()) {
      Value *OffsetVal = Builder.CreatePtrToInt(
          BaseAddr.getPointer(),
          llvm::Type::getInt32Ty(BaseAddr.getPointer()->getContext()));
      Value *OffsetVal64 = Builder.CreateZExt(OffsetVal, llvm::Type::getInt64Ty(BaseAddr.getPointer()->getContext()));
      //PointerVal = Builder.CreateTaintedOffset2Ptr(OffsetVal);
      //as an optimization, this call will be replaced by -->
      //get global variable sbxHeap
      GlobalVariable* sbxHeap = CGM.getModule().getNamedGlobal("sbxHeap");
      //Create a load on sbxHeap
      //create a i64 type
      llvm::Type* i64Type = llvm::Type::getInt64Ty(BaseAddr.getPointer()->getContext());
      Address* SbxHeapLoaded = new Address(sbxHeap, CGM.getPointerAlign());
      //Add this with the offset
      Value* OffsetValWithHeap = Builder.CreateAlignedLoad(i64Type, sbxHeap, 8, false);
      Value* OffsetValWithHeapAndOffset = Builder.CreateAdd(OffsetValWithHeap, OffsetVal64);
      //Bitcast this to the type of the pointer
      PointerVal = Builder.CreateIntToPtr(OffsetValWithHeapAndOffset ,
                                          llvm::Type::getInt8PtrTy(BaseAddr.getPointer()->getContext()));
    }

    Value *ConditionVal = PointerVal;

    if (!BaseAddr.getType()->getCoreElementType()->isFunctionTy())
    {
      ConditionVal = Builder.CreateIsTaintedPtr(PointerVal,
                                                "_Dynamic_check.tainted_pointer");
      EmitDynamicCheckBlocks(ConditionVal);
    }

    /*
     * If the destination type is NOT a TStruct Type, then we finna gonna follow
     * Template Tstruct Type itself
     */
    llvm::Type* DestTy = BitCastType;
    llvm::Type* DecoyedDestTy = BitCastType;
    if (DestTy->isTStructTy() || (DestTy->isPointerTy() &&
                                  DestTy->getCoreElementType()->isTStructTy())) {
      DestTy =
      DecoyedDestTy =
          ChangeStructName(reinterpret_cast<StructType *>(DestTy));

      if (DecoyedDestTy != NULL)
      {
        auto *CurrentTypeReferences = DestTy;
        while (CurrentTypeReferences->isPointerTy())
        {
                CurrentTypeReferences = CurrentTypeReferences->getPointerElementType();
                DecoyedDestTy = DecoyedDestTy->getPointerTo(0);
        }
      }
      else
        DecoyedDestTy  = BitCastType;
    }

    Value* CastedPointer = ConditionVal;
    if (!BaseAddr.getType()->getCoreElementType()->isFunctionTy())
      CastedPointer = Builder.CreatePointerCast(PointerVal, DecoyedDestTy);

    return CastedPointer;
}
BasicBlock *CodeGenFunction::EmitDynamicCheckFailedBlock() {
  // Save current insert point
  BasicBlock *Begin = Builder.GetInsertBlock();

  // Add a "failed block", which will be inserted at the end of CurFn
  BasicBlock *FailBlock = createBasicBlock("_Dynamic_check.failed", CurFn);
  Builder.SetInsertPoint(FailBlock);
  if (getLangOpts().InjectVerifierCalls) {
    llvm::Module &module = CGM.getModule();
    static llvm::Function *verr = llvm::Function::Create(
        llvm::FunctionType::get(Builder.getVoidTy(), false),
        llvm::Function::ExternalLinkage,
        "__VERIFIER_error",
        &module);
    CallInst *VerifierError = Builder.CreateCall(verr);
    VerifierError->setDoesNotReturn();
    VerifierError->setDoesNotThrow();
  }
  CallInst *TrapCall = Builder.CreateCall(CGM.getIntrinsic(Intrinsic::trap));
  TrapCall->setDoesNotReturn();
  TrapCall->setDoesNotThrow();
  Builder.CreateUnreachable();

  // Return the insert point back to the saved insert point
  Builder.SetInsertPoint(Begin);

  return FailBlock;
}

BasicBlock *CodeGenFunction::EmitNulltermWriteAdditionalCheck(
   const Address PtrAddr,
   const Address Upper,
   llvm::Value *LowerChk,
   llvm::Value *Val,
   BasicBlock *Succeeded) {
  // Save current insert point
  BasicBlock *Begin = Builder.GetInsertBlock();

  // Add a "failed block", which will be inserted at the end of CurFn
  BasicBlock *FailBlock =
    createBasicBlock("_Nullterm_range_check.failed", CurFn);
  Builder.SetInsertPoint(FailBlock);
  Value *AtUpper =
    Builder.CreateICmpEQ(PtrAddr.getPointer(), Upper.getPointer(),
                                        "_Dynamic_check.at_upper");
  BasicBlock *OnFailure = EmitDynamicCheckFailedBlock();
  llvm::Value *Condition1 =
    Builder.CreateAnd(LowerChk, AtUpper, "_Dynamic_check.nt_upper_bound");
  Value *IsZero = Builder.CreateIsNull(Val, "_Dynamic_check.write_nul");
  llvm::Value *Condition2 =
    Builder.CreateAnd(Condition1, IsZero, "_Dynamic_check.allowed_write");
  Builder.CreateCondBr(Condition2, Succeeded, OnFailure);
  // Return the insert point back to the saved insert point
  Builder.SetInsertPoint(Begin);

  return FailBlock;
}

BoundsExpr *CodeGenFunction::GetNullTermBoundsCheck(Expr *E) {
  E = E->IgnoreParenCasts();
  switch (E->getStmtClass()) {
    case Expr::UnaryOperatorClass: {
      UnaryOperator *UO = cast<UnaryOperator>(E);
      if (UO->getBoundsCheckKind() == BoundsCheckKind::BCK_NullTermWriteAssign)
        return UO->getBoundsExpr();
      break;
    }
    case Expr::ArraySubscriptExprClass: {
      ArraySubscriptExpr *AS = cast<ArraySubscriptExpr>(E);
      if (AS->getBoundsCheckKind() == BoundsCheckKind::BCK_NullTermWriteAssign)
        return AS->getBoundsExpr();
      break;
    }
    default:
      break;
  }
  return nullptr;
}
