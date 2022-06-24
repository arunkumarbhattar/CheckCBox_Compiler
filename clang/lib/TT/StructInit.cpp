//=--StructInit.cpp-----------------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This class contains implementation of the functions and
// classes of StructInit.h
//===----------------------------------------------------------------------===//

#include "clang/TT/StructInit.h"
#include "clang/TT/RewriteUtils.h"
#include "clang/Tooling/Transformer/SourceCode.h"
#include "clang/TT/Utils.h"
#include <sstream>

using namespace clang;

bool TStructVariableInitializer::hasCheckedMembers(DeclaratorDecl *DD) {
  // If this this isn't a structure or union, it can't have checked members.
  if (!isStructOrUnionType(DD))
    return false;

  // If this is a structure or union, then we can get the declaration for that
  // record and examine each member.
  RecordDecl *RD = DD->getType()->getAsRecordDecl();
  if (RecordDecl *Definition = RD->getDefinition()) {
    // See if we already know that this structure has a checked pointer.
    if (RecordsWithCPointers.find(Definition) != RecordsWithCPointers.end())
      return true;

    for (auto *const D : Definition->fields()) {
      if (D->getType()->isPointerType() || D->getType()->isArrayType()) {
        CVarOption CV = I.getVariable(D, Context);
        if (CV.hasValue()) {
          PVConstraint *PV = dyn_cast<PVConstraint>(&CV.getValue());
          if (PV && PV->isSolutionChecked(I.getConstraints().getVariables())) {
            // Ok this contains a pointer that is checked. Store it.
            // So you basically store the struct definitions (which is basically a Record Decl)
            // which have C pointer members into a RecordsWithCPointers Structure for future conversion.
            RecordsWithCPointers.insert(Definition);
            return true;
          }
        }
      } else if (hasCheckedMembers(D)) {
        // A field of a structure can be another structure. If the inner
        // structure would need an initializer, we have to put it on the outer.
        RecordsWithCPointers.insert(Definition);
        return true;
      }
    }
  }
  return false;
}

//So you would basically do something like this for Resolving TStructs -->
//Just before a TStruct is passed to a tainted function as an argument --> this is what you would basically do -->
// You would create a copy of the original Tstruct Tstruct_name (called Tstruct_name_ptr).
// For Every  Tstruct_name's Tstruct member you would create a _TPtr<TStruct> member
// Memory shall be allocated for each of these nested TStructs and subsequent assignment operations are inserted

bool TStructVariableInitializer::TStructHasTStructMembers(DeclaratorDecl *DD) {
  // If this this isn't a structure or union, it can't have TStruct members.
  if (!isTaintedStruct(DD))
    return false;

  // If this is a TStruct, then we can get the declaration for that
  // record and examine each member.
  RecordDecl *RD = DD->getType()->getAsRecordDecl();
  if (RecordDecl *Definition = RD->getDefinition()) {
    // See if we already know that this structure has a checked pointer.
    if (TStructRecordsWithTStructMembers.find(Definition) != TStructRecordsWithTStructMembers.end())
      return true;

    for (auto *const D : Definition->fields()) {
      if ((D->getType()->isTaintedStructureType()) && (!(D->getType()->isPointerType() ||
                                                         D->getType()->isCheckedPointerType() ||
                                                         D->getType()->isTaintedPointerType()))) {
            // Ok this contains a TStruct That is not of a pointer type.
            // So you basically store the Tstruct definitions (which is basically a Record Decl)
            TStructRecordsWithTStructMembers.insert(Definition);
            return true;
      }
    }
  }
  return false;
}

// Insert the declaration and correct replacement text for the declaration into
// the set of required rewritings.
bool TStructVariableInitializer::VisitVarDecl(VarDecl *VD) {
  // Check if we need to add an initializer.
  // TODO: Centralize initialization logic for all types:
  // https://github.com/correctcomputation/checkedc-clang/issues/645#issuecomment-876474200

  // The first two conditions are the same as in Sema::ActOnUninitializedDecl.
  if (VD->hasLocalStorage() && !isa<ParmVarDecl>(VD) && !VD->hasInit() &&
      hasCheckedMembers(VD)) {
    // Create replacement declaration text with an initializer.
//    std::string ToReplace = mkStringForDeclWithUnchangedType(VD, I) + " = {}";
    std::string ToReplace = "int temp = 0;";
    RewriteThese.insert(
        std::make_pair(VD, new MultiDeclMemberReplacement(VD, ToReplace, {})));
  }
  return true;
}

// You would have a function here that would basically insert all the malloc/assignment instrumentation

bool TStructVariableInitializer::InitializeTStructthathasTStructMembers(VarDecl *VD){
    //Write this later -->
}
