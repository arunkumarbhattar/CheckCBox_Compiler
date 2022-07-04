//=--IntermediateToolHook.cpp-------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Implementation of methods in IntermediateToolHook.h
//===----------------------------------------------------------------------===//

#include "clang/CheckMate/IntermediateToolHook.h"
#include "clang/CheckMate/ArrayBoundsInferenceConsumer.h"
#include "clang/CheckMate/CheckMateGlobalOptions.h"
#include "clang/AST/RecursiveASTVisitor.h"

using namespace llvm;
using namespace clang;

void IntermediateToolHook::HandleTranslationUnit(ASTContext &Context) {
  Info.enterCompilationUnit(Context);
  Info.getPerfStats().startArrayBoundsInferenceTime();
  handleArrayVariablesBoundsDetection(&Context, Info, !_CheckMateOpts.DisableArrH);
  Info.getPerfStats().endArrayBoundsInferenceTime();
  Info.exitCompilationUnit();
}
