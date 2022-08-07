//=--ConstraintBuilder.cpp----------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Implementation of visitor methods for the FunctionVisitor class. These
// visitors create constraints based on the AST of the program.
//===----------------------------------------------------------------------===//

#include "clang/CheckMate/ConstraintBuilder.h"
#include "clang/CheckMate/LowerBoundAssignment.h"
#include "clang/CheckMate/CheckMateGlobalOptions.h"
#include "clang/CheckMate/CheckMateStats.h"
#include "clang/CheckMate/ArrayBoundsInferenceConsumer.h"
#include "clang/CheckMate/ConstraintResolver.h"
#include "clang/CheckMate/TypeVariableAnalysis.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include <algorithm>
#include <iostream>

using namespace llvm;
using namespace clang;

class MyASTVisitor : public clang::RecursiveASTVisitor<MyASTVisitor> {
public:
  explicit MyASTVisitor(ASTContext *C, ProgramInfo &I)
      : Context(C), Info(I), CB(Info, Context) {}

  std::string fetchMacroExpanseForDecl(SourceRange SR,
                                       SourceManager *SM)
  {
       auto FullSourceLocation = FullSourceLoc(SR.getBegin(),
                                            *SM);
       /*
        * Now in a loop walk backwards the source until you find the pre-
        * processor token (#)
        */

       signed int Counter = 0;
       auto MacroLoc = FullSourceLocation.getSpellingLoc();
       char CurrToken = (char)MacroLoc.getCharacterData()[0];
       auto IntermediateMacroLoc = MacroLoc;
       while(CurrToken != '#')
       {
         Counter = Counter - 1;
         auto TempLoc = MacroLoc.getSpellingLoc()
                                 .getLocWithOffset(Counter);
         IntermediateMacroLoc = FullSourceLoc(TempLoc, *SM);
         CurrToken = (char)IntermediateMacroLoc.getCharacterData()[0];
       }

       const auto *Spelling = FullSourceLoc(IntermediateMacroLoc, *SM).getSpellingLoc()
                           .getCharacterData();
       /*
        * Now that you point to the right location fetch the sourceLoc string
        */
       /*
        * Support for multiline macros is a little different
        * if a escape character is preceeded by a backslash, it means the macro is continued
        * through the next line
        *
        * Hence You basically look for the location that has no preceeding non-space character as
        * backslash
        *
        */
       bool MultiLineMacroNotFound = true;
       int pos = 0;
       int MacroTerminatingPos = std::string(Spelling).find("\n");

       do{
         pos = std::string(Spelling).find("\n", pos);
         int Counter = 1;
         auto PreceedingNonSpaceChar =
             std::string(Spelling).find("\n", pos)-Counter;
         while(isSpace(reinterpret_cast<char>(
                    std::string(Spelling).at(PreceedingNonSpaceChar))))
         {
           Counter = Counter - 1;
           PreceedingNonSpaceChar = std::string(Spelling).find("\n", pos)-Counter;
         }
         if (reinterpret_cast<char>(
                 std::string(Spelling).at(PreceedingNonSpaceChar)) !=
             '\\')
        {
           MacroTerminatingPos = std::string(Spelling).find("\n", pos);
           MultiLineMacroNotFound = false;
        }
        else
         {
          pos =  std::string(Spelling).find("\n", pos+1);
         }
       }while(MultiLineMacroNotFound);

       return std::string(Spelling).substr(0,
                                           MacroTerminatingPos);
  }

  bool VisitStmt(Stmt *S)
  {
    if(S->getBeginLoc().isMacroID() && S->getTaintedScopeSpecifier() != clang::Tainted_None)
    {
      std::string IncludeDirective = fetchMacroExpanseForDecl(
          S->getSourceRange(),
          &(Context->getSourceManager()));
      CB.storeIncludeStatement(
          Context->getFullLoc(S->getBeginLoc()).getSpellingLoc(), Context->getSourceManager(),
          IncludeDirective);
    }
    return true;
  }

  bool VisitExpr(Expr *S)
  {
    if(S->getBeginLoc().isMacroID() && S->getTaintedScopeSpecifier() != clang::Tainted_None)
    {
      std::string IncludeDirective = fetchMacroExpanseForDecl(
          S->getSourceRange(),
          &(Context->getSourceManager()));
      CB.storeIncludeStatement(
          Context->getFullLoc(S->getBeginLoc()).getSpellingLoc(), Context->getSourceManager(),
          IncludeDirective);
    }
    return true;
  }

  bool VisitCallExpr(clang::CallExpr *callexpr)
  {
    auto RefDec = callexpr->getCallee()->getReferencedDeclOfCallee();
    if( RefDec != NULL && RefDec->getAsFunction() != NULL &&
        callexpr->getBeginLoc().isMacroID() && (callexpr->getTaintedScopeSpecifier() != clang::Tainted_None))
    {
      std::cout<<"Visiting a MACRO call expression: " <<
          RefDec->getAsFunction()->getName().str()<<std::endl;
      std::string IncludeDirective = fetchMacroExpanseForDecl(

          callexpr->getSourceRange(),
          &(Context->getSourceManager()));
      CB.storeIncludeStatement(
          Context->getFullLoc(callexpr->getBeginLoc()).getSpellingLoc(), Context->getSourceManager(),
          IncludeDirective);
    }

    return true;
  }

  bool VisitUnaryOperator(clang::UnaryOperator* unary)
  {
    auto decl = unary->getReferencedDeclOfCallee();
    if(decl != NULL)
    {
      auto Vardecl = dyn_cast<VarDecl>(decl);
      if(Vardecl != NULL && Vardecl->getLocation().isMacroID()
          && (unary->getTaintedScopeSpecifier() != clang::Tainted_None))
      {
        /*
         * fetch the macro expanse for this Macro
         */
        std::string IncludeDirective = fetchMacroExpanseForDecl(
            Vardecl->getSourceRange(),
            &(Context->getSourceManager()));
        std::cout<<"Include directive: "<< IncludeDirective<<std::endl;
        CB.storeIncludeStatement(
            Context->getFullLoc(unary->getExprLoc()).getSpellingLoc(), Context->getSourceManager(),
            IncludeDirective);
      }
    }
    return true;
  }

  bool VisitBinaryOperator(clang::BinaryOperator *bo)
  {
    auto LHSExpr = bo->getLHS();
    auto RHSExpr = bo->getRHS();

    if((RHSExpr == NULL) || (LHSExpr == NULL))
      return true;

    auto RhsD = RHSExpr->getReferencedDeclOfCallee();
    auto LhsD = LHSExpr->getReferencedDeclOfCallee();
    if (RhsD != NULL) {
      auto *Vardec = dyn_cast<VarDecl>(RhsD);
      if(Vardec != NULL && RhsD->getLocation().isMacroID() &&
          bo->getTaintedScopeSpecifier() != clang::Tainted_None)
      {
        std::cout<<"Visiting a Binary expression where your RHS is "
                  << Vardec->getNameAsString()<<std::endl;
        std::string IncludeDirective = fetchMacroExpanseForDecl(
            Vardec->getSourceRange(),
            &(Context->getSourceManager()));
        CB.storeIncludeStatement(
            Context->getFullLoc(RHSExpr->getExprLoc()).getSpellingLoc(), Context->getSourceManager(),
            IncludeDirective);
      }
    }

    if (LhsD != NULL) {
      auto *Vardec = dyn_cast<VarDecl>(LhsD);
      if(Vardec != NULL && LhsD->getLocation().isMacroID() &&
          LHSExpr->getTaintedScopeSpecifier() != Tainted_None)
      {
        std::cout<<"Visiting a Binary expression where your LHS is "
                  << Vardec->getNameAsString()<<std::endl;
        std::string IncludeDirective = fetchMacroExpanseForDecl(
            Vardec->getSourceRange(),
            &(Context->getSourceManager()));
        CB.storeIncludeStatement(
            Context->getFullLoc(LHSExpr->getExprLoc()).getSpellingLoc(), Context->getSourceManager(),
            IncludeDirective);
      }
    }
    return true;
  }

private:
  ASTContext *Context;
  ProgramInfo &Info;
  FunctionDecl *Function;
  ConstraintResolver CB;
};
// This class visits functions and adds constraints to the
// Constraints instance assigned to it.
// Each VisitXXX method is responsible for looking inside statements
// and imposing constraints on variables it uses
class FunctionVisitor : public RecursiveASTVisitor<FunctionVisitor> {
public:
  explicit FunctionVisitor(ASTContext *C, ProgramInfo &I, FunctionDecl *FD)
      : Context(C), Info(I), Function(FD), CB(Info, Context) {}

  // T x = e
  /*
   * This basically visits every decl involved in each and every statement
   */
  bool VisitDeclStmt(DeclStmt *S) {
    // Process inits even for non-pointers because structs and union values
    // can contain pointers
    for (const auto &D : S->decls()) {
      if(FunctionDecl *FD = dyn_cast<FunctionDecl>(D)){
        std::cout<< FD->getName().str() << std::endl;
        FD->getBody()->dump();
        FD->getReturnType()->dump();
        std::cout << "Is Tainted: " << getDeclaration(FD)->isTainted();
        for(int i = 0 ; i < FD->getNumParams(); i++)
        {
          FD->getParamDecl(i)->dump();
        }
      }
    }
    return true;
  }

  // (T)e
  bool VisitCStyleCastExpr(CStyleCastExpr *C) {
    // Is cast compatible with LHS type?
    QualType SrcT = C->getSubExpr()->getType();
    QualType DstT = C->getType();
    if (!CB.isCastofGeneric(C) && !isCastSafe(DstT, SrcT)
      && !Info.hasPersistentConstraints(C, Context)) {
      auto CVs = CB.getExprConstraintVarsSet(C->getSubExpr());
      std::string Rsn =
          "Cast from " + SrcT.getAsString() + " to " + DstT.getAsString();
      CB.constraintAllCVarsToWild(CVs, Rsn, C);
    }
    return true;
  }

  // x += e
  bool VisitCompoundAssignOperator(CompoundAssignOperator *O) {
    switch (O->getOpcode()) {
    case BO_AddAssign:
    case BO_SubAssign:
      arithBinop(O, true);
      break;
    // rest shouldn't happen on pointers, so we ignore
    default:
      break;
    }
    return true;
  }

  // e(e1,e2,...)
  bool VisitCallExpr(CallExpr *E) {
    if (E != NULL && E->getReferencedDeclOfCallee() != NULL
        && E->getReferencedDeclOfCallee()->getAsFunction() != NULL
        && E->getTaintedScopeSpecifier() != clang::Tainted_None)
    {
      auto FD = E->getReferencedDeclOfCallee()->getAsFunction();
      if(FD->getLocation().isMacroID()) {
        auto FullSourceLocation =
            FullSourceLoc(FD->getSourceRange().getBegin(),
                          FD->getASTContext().getSourceManager());
        auto MacroLoc =
            FullSourceLocation.getSpellingLoc().getLocWithOffset(-25);
        FullSourceLocation =
            FullSourceLoc(MacroLoc, FD->getASTContext().getSourceManager());
        auto spellingLoc =
            FullSourceLocation.getSpellingLoc().getCharacterData();
        auto refinedSpellingLoc = std::string(spellingLoc).substr(0,
        std::string(spellingLoc).find("\n"));
        std::cout<<"Macro in use: "<<refinedSpellingLoc<<std::endl;
      }
      else
      {
        std::cout<<FD->getNameAsString()<<" is not a Macro"<<std::endl;
      }
    }
    PersistentSourceLoc PL = PersistentSourceLoc::mkPSL(E, *Context);
    auto &CS = Info.getConstraints();
    CVarSet FVCons = CB.getCalleeConstraintVars(E);

    // When multiple function variables are used in the same expression, they
    // must have the same type.
    if (FVCons.size() > 1) {
      PersistentSourceLoc PL =
          PersistentSourceLoc::mkPSL(E->getCallee(), *Context);
      auto Rsn = ReasonLoc("Multiple function variables", PL);
      constrainConsVarGeq(FVCons, FVCons, Info.getConstraints(), Rsn,
                          Same_to_Same, false, &Info);
    }

    Decl *D = E->getCalleeDecl();
    FunctionDecl *TFD = dyn_cast_or_null<FunctionDecl>(D);
    std::string FuncName;
    if (auto *DD = dyn_cast_or_null<DeclaratorDecl>(D))
      FuncName = DD->getNameAsString();

    // Collect type parameters for this function call that are
    // consistently instantiated as single type in this function call.
    auto ConsistentTypeParams =
        Info.hasTypeParamBindings(E,Context) ?
          Info.getTypeParamBindings(E,Context) :
          ProgramInfo::CallTypeParamBindingsT();

    // Now do the call: Constrain arguments to parameters (but ignore returns)
    if (FVCons.empty()) {
      // Don't know who we are calling; make args WILD
      constraintAllArgumentsToWild(E);
    } else if (!ConstraintResolver::canFunctionBeSkipped(FuncName)) {
      // FIXME: realloc comparison is still required. See issue #176.
      // If we are calling realloc, ignore it, so as not to constrain the first
      // arg. Else, for each function we are calling ...
      for (auto *TmpC : FVCons) {
        if (PVConstraint *PVC = dyn_cast<PVConstraint>(TmpC)) {
          TmpC = PVC->getFV();
          assert(TmpC != nullptr && "Function pointer with null FVConstraint.");
        }
        std::set<unsigned> PrintfStringArgIndices;
        if (TFD != nullptr)
          getPrintfStringArgIndices(E, TFD, *Context, PrintfStringArgIndices);
        // and for each arg to the function ...
        if (FVConstraint *TargetFV = dyn_cast<FVConstraint>(TmpC)) {
          unsigned I = 0;
          for (const auto &A : E->arguments()) {
            CSetBkeyPair ArgumentConstraints;
            auto ArgPSL = PersistentSourceLoc::mkPSL(A,*Context);
            auto Rsn = ReasonLoc("Constrain arguments to parameters", ArgPSL);
            if (I < TargetFV->numParams()) {
              // When the function has a void* parameter, Clang will
              // add an implicit cast to void* here. Generating constraints
              // will add an extraneous wild constraint to void*. This
              // unnecessarily complicates results and root causes.
              if (TargetFV->getExternalParam(I)->isVoidPtr())
                ArgumentConstraints =
                    CB.getExprConstraintVars(A->IgnoreImpCasts());
              else
                ArgumentConstraints = CB.getExprConstraintVars(A);
            } else
              ArgumentConstraints = CB.getExprConstraintVars(A);

            if (I < TargetFV->numParams()) {
              // Constrain the arg CV to the param CV.
              ConstraintVariable *ParameterDC = TargetFV->getExternalParam(I);

              // We cannot insert a cast if the source location of a call
              // expression is not writable. By using Same_to_Same for calls at
              // unwritable source locations, we ensure that we will not need to
              // insert a cast because this unifies the checked type for the
              // parameter and the argument.
              ConsAction CA = Rewriter::isRewritable(A->getExprLoc())
                                  ? Wild_to_Safe
                                  : Same_to_Same;
              // Do not handle bounds key here because we will be
              // doing context-sensitive assignment next.
              constrainConsVarGeq(ParameterDC, ArgumentConstraints.first, CS,
                                  Rsn, CA, false, &Info, false);

              if (_CheckMateOpts.AllTypes && TFD != nullptr &&
                  I < TFD->getNumParams()) {
                auto *PVD = TFD->getParamDecl(I);
                auto &CSBI = Info.getABoundsInfo().getCtxSensBoundsHandler();
                // Here, we need to handle context-sensitive assignment.
                CSBI.handleContextSensitiveAssignment(
                    PL, PVD, ParameterDC, A, ArgumentConstraints.first,
                    ArgumentConstraints.second, Context, &CB);
              }
            } else {
              // The argument passed to a function ith varargs; make it wild
              if (_CheckMateOpts.HandleVARARGS) {
                CB.constraintAllCVarsToWild(ArgumentConstraints.first,
                                            "Passing argument to a function "
                                            "accepting var args.",
                                            E);
              } else {
                if (PrintfStringArgIndices.find(I) !=
                    PrintfStringArgIndices.end()) {
                  // In `printf("... %s ...", ...)`, the argument corresponding
                  // to the `%s` should be an _Nt_array_ptr
                  // (https://github.com/correctcomputation/checkedc-clang/issues/549).
                  constrainVarsTo(ArgumentConstraints.first, CS.getNTArr(),
                                  ReasonLoc(NT_ARRAY_REASON,ArgPSL));
                }
                if (_CheckMateOpts.Verbose) {
                  std::string FuncName = TargetFV->getName();
                  errs() << "Ignoring function as it contains varargs:"
                         << FuncName << "\n";
                }
              }
            }
            I++;
          }
        }
      }
    }
    return true;
  }

  // e1[e2]
  bool VisitArraySubscriptExpr(ArraySubscriptExpr *E) {
    Constraints &CS = Info.getConstraints();
    auto PSL = PersistentSourceLoc::mkPSL(E,*Context);
    constraintInBodyVariable(E->getBase(), CS.getArr(),
                             ReasonLoc(ARRAY_REASON,PSL));
    return true;
  }

  // return e;
  bool VisitReturnStmt(ReturnStmt *S) {
    // Get function variable constraint of the body
    CVarOption CVOpt = Info.getVariable(Function, Context);

    // Constrain the value returned (if present) against the return value
    // of the function.
    Expr *RetExpr = S->getRetValue();

    CVarSet RconsVar = CB.getExprConstraintVarsSet(RetExpr);
    // Constrain the return type of the function
    // to the type of the return expression.
    if (CVOpt.hasValue()) {
      if (FVConstraint *FV = dyn_cast<FVConstraint>(&CVOpt.getValue())) {
        // This is to ensure that the return type of the function is same
        // as the type of return expression.
        PersistentSourceLoc PL = PersistentSourceLoc::mkPSL(S, *Context);
        auto Rsn = ReasonLoc("Return types must match", PL);
        constrainConsVarGeq(FV->getInternalReturn(), RconsVar,
                            Info.getConstraints(), Rsn, Same_to_Same, false,
                            &Info);
      }
    }
    return true;
  }

  // ++e, --e, e++, and e--
  bool VisitUnaryOperator(UnaryOperator *O) {
    switch (O->getOpcode()) {
    case clang::UO_PreInc:
    case clang::UO_PreDec:
    case clang::UO_PostInc:
    case clang::UO_PostDec:
      constraintPointerArithmetic(O->getSubExpr());
      break;
    default:
      break;
    }
    auto E = O->getExprStmt();
    if (E != NULL && E->getReferencedDeclOfCallee() != NULL
        && O->getTaintedScopeSpecifier() != clang::Tainted_None)
    {
      auto FD = dyn_cast<VarDecl>(E->getReferencedDeclOfCallee());
      if(FD->getLocation().isMacroID()) {
        auto FullSourceLocation =
            FullSourceLoc(FD->getSourceRange().getBegin(),
                          FD->getASTContext().getSourceManager());
        auto MacroLoc =
            FullSourceLocation.getSpellingLoc().getLocWithOffset(-25);
        FullSourceLocation =
            FullSourceLoc(MacroLoc, FD->getASTContext().getSourceManager());
        auto spellingLoc =
            FullSourceLocation.getSpellingLoc().getCharacterData();
        auto refinedSpellingLoc = std::string(spellingLoc).substr(0,
                                                                  std::string(spellingLoc).find("\n"));
        std::cout<<"Macro in use: "<<refinedSpellingLoc<<std::endl;
      }
      else
      {
        std::cout<<FD->getNameAsString()<<" is not a Macro"<<std::endl;
      }
    }
    return true;
  }

  bool VisitBinaryOperator(BinaryOperator *O) {
    switch (O->getOpcode()) {
    // e1 + e2 and e1 - e2
    case clang::BO_Add:
    case clang::BO_Sub:
      arithBinop(O);
      break;
    // x = e
    case clang::BO_Assign:
      CB.constrainLocalAssign(O, O->getLHS(), O->getRHS(), Same_to_Same);
      break;
    default:
      break;
    }

    if(O->getTaintedScopeSpecifier() == clang::Tainted_None)
      return true;

    auto R = O->getRHS();
    if (R != NULL && R->getReferencedDeclOfCallee() != NULL)
    {
      auto FD = dyn_cast<VarDecl>(
          R->getReferencedDeclOfCallee());
      if(FD->getLocation().isMacroID()) {
        auto FullSourceLocation =
            FullSourceLoc(FD->getSourceRange().getBegin(),
                          FD->getASTContext().getSourceManager());
        auto MacroLoc =
            FullSourceLocation.getSpellingLoc().getLocWithOffset(-25);
        FullSourceLocation =
            FullSourceLoc(MacroLoc, FD->getASTContext().getSourceManager());
        auto spellingLoc =
            FullSourceLocation.getSpellingLoc().getCharacterData();
        auto refinedSpellingLoc = std::string(spellingLoc).substr(0,
                                                                  std::string(spellingLoc).find("\n"));
        std::cout<<"Macro in use: "<<refinedSpellingLoc<<std::endl;
      }
      else
      {
        std::cout<<FD->getNameAsString()<<" is not a Macro"<<std::endl;
      }
    }
    auto L = O->getLHS();
    if (L != NULL && L->getReferencedDeclOfCallee() != NULL)
    {
      auto FD = dyn_cast<VarDecl>(
          L->getReferencedDeclOfCallee());
      if(FD->getLocation().isMacroID()) {
        auto FullSourceLocation =
            FullSourceLoc(FD->getSourceRange().getBegin(),
                          FD->getASTContext().getSourceManager());
        auto MacroLoc =
            FullSourceLocation.getSpellingLoc().getLocWithOffset(-25);
        FullSourceLocation =
            FullSourceLoc(MacroLoc, FD->getASTContext().getSourceManager());
        auto spellingLoc =
            FullSourceLocation.getSpellingLoc().getCharacterData();
        auto refinedSpellingLoc = std::string(spellingLoc).substr(0,
                                                                  std::string(spellingLoc).find("\n"));
        std::cout<<"Macro in use: "<<refinedSpellingLoc<<std::endl;
      }
      else
      {
        std::cout<<FD->getNameAsString()<<" is not a Macro"<<std::endl;
      }
    }
    return true;
  }

private:
  // Constraint all the provided vars to be
  // equal to the provided type i.e., (V >= type).
  void constrainVarsTo(CVarSet &Vars, ConstAtom *CAtom, const ReasonLoc &Rsn) {
    Constraints &CS = Info.getConstraints();
    for (const auto &I : Vars)
      if (PVConstraint *PVC = dyn_cast<PVConstraint>(I)) {
        PVC->constrainOuterTo(CS, CAtom, Rsn);
      }
  }

  // Constraint helpers.
  void constraintInBodyVariable(Expr *E, ConstAtom *CAtom, const ReasonLoc &Rsn) {
    CVarSet Var = CB.getExprConstraintVarsSet(E);
    constrainVarsTo(Var, CAtom, Rsn);
  }

  // Constraint all the argument of the provided
  // call expression to be WILD.
  void constraintAllArgumentsToWild(CallExpr *E) {
    PersistentSourceLoc Psl = PersistentSourceLoc::mkPSL(E, *Context);
    for (const auto &A : E->arguments()) {
      // Get constraint from within the function body
      // of the caller.
      CVarSet ParameterEC = CB.getExprConstraintVarsSet(A);

      // Assign WILD to each of the constraint variables.
      FunctionDecl *FD = E->getDirectCallee();
      std::string Rsn = "Argument to function " +
                        (FD != nullptr ? FD->getName().str() : "pointer call");
      Rsn += " with out Constraint vars.";
      CB.constraintAllCVarsToWild(ParameterEC, Rsn, E);
    }
  }

  // Here the flag, ModifyingExpr indicates if the arithmetic operation
  // is modifying any variable.
  void arithBinop(BinaryOperator *O, bool ModifyingExpr = false) {
    constraintPointerArithmetic(O->getLHS(), ModifyingExpr);
    constraintPointerArithmetic(O->getRHS(), ModifyingExpr);
  }

  // Pointer arithmetic constrains the expression to be at least ARR,
  // unless it is on a function pointer. In this case the function pointer
  // is WILD.
  void constraintPointerArithmetic(Expr *E, bool ModifyingExpr = true) {
    if (E->getType()->isFunctionPointerType()) {
      CVarSet Var = CB.getExprConstraintVarsSet(E);
      std::string Rsn = "Pointer arithmetic performed on a function pointer.";
      CB.constraintAllCVarsToWild(Var, Rsn, E);
    } else {
      auto PSL = PersistentSourceLoc::mkPSL(E,*Context);
      if (ModifyingExpr)
        Info.getABoundsInfo().recordArithmeticOperation(E, &CB);
      constraintInBodyVariable(E, Info.getConstraints().getArr(),
                               ReasonLoc(ARRAY_REASON,PSL));
    }
  }

  ASTContext *Context;
  ProgramInfo &Info;
  FunctionDecl *Function;
  ConstraintResolver CB;
};

// This class visits a global declaration, generating constraints
// for functions, variables, types, etc. that are visited.
class ConstraintGenVisitor : public RecursiveASTVisitor<ConstraintGenVisitor> {
public:
  explicit ConstraintGenVisitor(ASTContext *Context, ProgramInfo &I)
      : Context(Context), Info(I), CB(Info, Context) {}

  bool VisitVarDecl(VarDecl *G) {

    if (G->hasGlobalStorage() && isPtrOrArrayType(G->getType())) {
      if (G->hasInit()) {
        CB.constrainLocalAssign(nullptr, G, G->getInit(), Same_to_Same);
      }
    }

    if(G->hasGlobalStorage() && (G->isTaintedDecl() || G->isMirrorDecl()))
    {
      /*
       * You need to check if this variable has been declared as a typedef or not
       * If this variable is a typedef, then no need to pass it once again
       */
      CB.storeTaintMirroredVarDecl(G);
      if (_CheckMateOpts.Verbose)
        errs() << "Analyzing function " << G->getNameAsString() << "\n";

    }
    return true;
  }

  bool VisitInitListExpr(InitListExpr *E) {
    if (E->getType()->isTaintedStructureType()) {
      /*
       * Here is where you need to Create a map and insert the record decl
       */
      /*
     * If the variable is marked as _Tainted or _Mirror, it must be copy pasted to the Tainted file
     *
     */

      E = E->getSemanticForm();
      const RecordDecl *Definition =
          E->getType()->getAsStructureType()->getDecl()->getDefinition();

      if (Definition->getParentFunctionOrMethod() == nullptr)
        CB.storeTaintMirroredStructVarDecl(
            const_cast<RecordDecl *>(Definition));
      else
        return true;

//      unsigned int InitIdx = 0;
//      const auto Fields = Definition->fields();
//      for (auto It = Fields.begin();
//           InitIdx < E->getNumInits() && It != Fields.end(); InitIdx++, It++) {
//        Expr *InitExpr = E->getInit(InitIdx);
//        CB.constrainLocalAssign(nullptr, *It, InitExpr, Same_to_Same, true);
//      }
    }
    return true;
  }

  bool VisitFunctionDecl(FunctionDecl *D) {
    FullSourceLoc FL = Context->getFullLoc(D->getBeginLoc());


    if((D->isTainted()) || (D->isMirror())){
      if (_CheckMateOpts.Verbose)
        errs() << "Storing Tainted function " << D->getName() << "\n";
      CB.storeTaintedFunctionDecl(D);
    }

    if(D->isCallback() && !D->isThisDeclarationADefinition())
    {
      /*
       * For Callbacks We only want the Prototype declaration
       */
      errs() << "Storing Callback function " << D->getName() << "\n";
      CB.storeCallbackFunctionDecl(D);
    }


    /*

    if (FL.isValid()) { // TODO: When would this ever be false?
      if (D->hasBody() && D->isThisDeclarationADefinition()) {
        Stmt *Body = D->getBody();
        FunctionVisitor FV = FunctionVisitor(Context, Info, D);
        FV.TraverseStmt(Body);
        if (_CheckMateOpts.AllTypes) {
          // Only do this, if all types is enabled.
          LengthVarInference LVI(Info, Context, D);
          LVI.Visit(Body);
        }

      }
    }
    */
    if (_CheckMateOpts.Verbose)
      errs() << "Done analyzing function\n";

    return true;
  }
  bool VisitTypedefDecl(TypedefDecl *TD) {
    // TODO: need to handle here
    if(TD->isTaintedDecl() || TD->isMirrorDecl())
    {
      CB.storeTaintMirroredTypedefDecl(TD);
      errs() << "Tainted/Mirrored Typedef Decl: "<<TD->getNameAsString()<<"\n";
    }
    return true;
  }

  bool VisitEnumDecl(EnumDecl *D)
  {
    if(D->isTaintedDecl() || D->isMirrorDecl() )
    {
        CB.storeTaintMirroredEnumDecl(D);
        errs()<< "Printing Enum"<< D->getName()<<"\n";
    }
  }

  bool VisitRecordDecl(RecordDecl *Declaration) {
    if (Declaration->isThisDeclarationADefinition()) {
      if(Declaration->isTaintedStruct())
      {
        /*
         * Yes this works
         */
        CB.storeTaintMirroredStructVarDecl(Declaration);
        errs()<< "Printing Tstruct"<< Declaration->getName()<<"\n";
      }

//      RecordDecl *Definition = Declaration->getDefinition();
//      assert("Declaration is a definition, but getDefinition() is null?" &&
//             Definition);
//      FullSourceLoc FL = Context->getFullLoc(Definition->getBeginLoc());
//      if (FL.isValid())
//        for (auto *const D : Definition->fields())
//          addVariable(D);
//
    }

    return true;
  }

private:
  ASTContext *Context;
  ProgramInfo &Info;
  ConstraintResolver CB;
};

// Some information about variables in the program is required by the type
// variable analysis and constraint building passes. This is gathered by this
// visitor which is executed before both of the other visitors.
class VariableAdderVisitor : public RecursiveASTVisitor<VariableAdderVisitor> {
public:
  explicit VariableAdderVisitor(ASTContext *Context, ProgramVariableAdder &VA)
      : Context(Context), VarAdder(VA) {}

  // Defining this function lets the visitor traverse implicit function
  // declarations. Without this, we wouldn't see declarations for implicit
  // functions. Instead, we would have to create a FVConstraint when we first
  // encountered a call to the function. This became a problem when the call
  // to ProgramInfo::link() was moved to before the ConstraintBuilder pass.
  bool shouldVisitImplicitCode() const { return true; }

  bool VisitTypedefDecl(TypedefDecl *TD) {
    auto PSL = PersistentSourceLoc::mkPSL(TD, *Context);
    // If we haven't seen this typedef before, initialize it's entry in the
    // typedef map. If we have seen it before, and we need to preserve the
    // constraints contained within it
    if (!VarAdder.seenTypedef(PSL))
      // Add this typedef to the program info.
      VarAdder.addTypedef(PSL, TD, *Context);
    return true;
  }

//  if ((G->isTaintedDecl() || G->isMirrorDecl()))
//  {
//    if ((G->getParentFunctionOrMethod() == nullptr))
//      CB.storeTaintMirroredVarDecl(G);
//  }
//  else if (isTaintedStruct(G))
//  {
//    if (G->getParentFunctionOrMethod() == nullptr)
//      CB.storeTaintMirroredStructVarDecl(G);
//  }
  bool VisitVarDecl(VarDecl *D) {

    FullSourceLoc FL = Context->getFullLoc(D->getBeginLoc());
    // ParmVarDecls are skipped here, and are added in ProgramInfo::addVariable
    // as it processes a function
    // TODO: Need to handle here too
    if (FL.isValid() && !isa<ParmVarDecl>(D))
      addVariable(D);
    return true;
  }

  bool VisitFunctionDecl(FunctionDecl *D) {
    FullSourceLoc FL = Context->getFullLoc(D->getBeginLoc());
    if (FL.isValid())
      VarAdder.addVariable(D, Context);
    return true;
  }

  bool VisitRecordDecl(RecordDecl *Declaration) {
    if (Declaration->isThisDeclarationADefinition()) {
      RecordDecl *Definition = Declaration->getDefinition();
      assert("Declaration is a definition, but getDefinition() is null?" &&
             Definition);
      FullSourceLoc FL = Context->getFullLoc(Definition->getBeginLoc());
      if (FL.isValid())
        for (auto *const D : Definition->fields())
          addVariable(D);
    }
    return true;
  }

private:
  ASTContext *Context;
  ProgramVariableAdder &VarAdder;

  void addVariable(DeclaratorDecl *D) {
    VarAdder.addABoundsVariable(D);
    if (isPtrOrArrayType(D->getType()))
      VarAdder.addVariable(D, Context);
  }
};

void VariableAdderConsumer::HandleTranslationUnit(ASTContext &C) {
  Info.enterCompilationUnit(C);
  if (_CheckMateOpts.Verbose) {
    SourceManager &SM = C.getSourceManager();
    FileID MainFileId = SM.getMainFileID();
    const FileEntry *FE = SM.getFileEntryForID(MainFileId);
    if (FE != nullptr)
      errs() << "Analyzing file " << FE->getName() << "\n";
    else
      errs() << "Analyzing\n";
  }

  VariableAdderVisitor VAV = VariableAdderVisitor(&C, Info);
  LowerBoundAssignmentFinder LBF = LowerBoundAssignmentFinder(&C, Info);
  TranslationUnitDecl *TUD = C.getTranslationUnitDecl();
  // Collect Variables.
  for (const auto &D : TUD->decls()) {
    VAV.TraverseDecl(D);
    LBF.TraverseDecl(D);
  }

  if (_CheckMateOpts.Verbose)
    errs() << "Done analyzing\n";

  Info.exitCompilationUnit();
  return;
}

void ConstraintBuilderConsumer::HandleTranslationUnit(ASTContext &C) {
  Info.enterCompilationUnit(C);
    SourceManager &SM = C.getSourceManager();
    FileID MainFileId = SM.getMainFileID();
    const FileEntry *FE = SM.getFileEntryForID(MainFileId);
    if (FE != nullptr)
      errs() << "Analyzing file " << FE->getName() << "\n";
    else
      errs() << "Analyzing\n";

  auto &PStats = Info.getPerfStats();

  PStats.startConstraintBuilderTime();

  TypeVarVisitor TV = TypeVarVisitor(&C, Info);
  ConstraintResolver CSResolver(Info, &C);
  ContextSensitiveBoundsKeyVisitor CSBV =
      ContextSensitiveBoundsKeyVisitor(&C, Info, &CSResolver);
  ConstraintGenVisitor GV = ConstraintGenVisitor(&C, Info);
  MyASTVisitor MyASS = MyASTVisitor(&C, Info);
  TranslationUnitDecl *TUD = C.getTranslationUnitDecl();
  StatsRecorder SR(&C, &Info);

  // Generate constraints.
  for (const auto &D : TUD->decls()) {
    // The order of these traversals CANNOT be changed because the constraint
    // gen visitor requires the type variable information gathered in the type
    // variable traversal.

    CSBV.TraverseDecl(D);
    TV.TraverseDecl(D);
  }

  // Store type variable information for use in rewriting
  TV.setProgramInfoTypeVars();

  for (const auto &D : TUD->decls()) {
    GV.TraverseDecl(D);
    MyASS.TraverseDecl(D);
    SR.TraverseDecl(D);
  }

  if (_CheckMateOpts.Verbose)
    errs() << "Done analyzing\n";

  PStats.endConstraintBuilderTime();

  PStats.endConstraintBuilderTime();

  Info.exitCompilationUnit();
  return;
}
