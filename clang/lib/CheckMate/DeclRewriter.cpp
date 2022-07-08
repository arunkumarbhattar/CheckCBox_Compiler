//=--DeclRewriter.cpp---------------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "clang/CheckMate/DeclRewriter.h"
#include "clang/CheckMate/CheckMateGlobalOptions.h"
#include "clang/CheckMate/RewriteUtils.h"
#include "clang/CheckMate/Utils.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Stmt.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <sstream>
#include <ostream>

using namespace llvm;
using namespace clang;

bool TaintedFunctionConstructor(ASTContext &Context, ProgramInfo &Info,
Rewriter &R, Decl* tainted_D){
  /*
   * Here is where we will construct the function signature for the tainted type
   *
   */
  std::string function_return_signature = "";

}

SourceLocation
end_of_the_end(SourceLocation const & start_of_end, SourceManager & sm){
  LangOptions lopt;
  return Lexer::getLocForEndOfToken(start_of_end, 0, sm, lopt);
}

bool CopyTaintedDefToTaintedFile(ASTContext &Context, ProgramInfo &Info,
                                 Rewriter &R, Decl* FD){
  /*
   * first fetch the output stream buffer
   */
  /*
   * For now lets just do the below, later we
   */

  std::error_code error_code;
  llvm::raw_fd_ostream outFile(Info.tainted_stream_writer[FD],
                               error_code, llvm::sys::fs::OF_Append);
  RewriteBuffer RB;
  if(std::find(Info.tainted_outfiles.begin(),
                Info.tainted_outfiles.end(),
                const_cast<llvm::raw_fd_ostream*>(&outFile)) == Info.tainted_outfiles.end()){
      /*
       * This is a new file being written, hence Initialize it
       */

    RB.Initialize("/* This file is Auto-Generated Using CheckCBox Converter. Please Do Not Directly Modify."
                    "\n */ \n");
  }

  RB.write(outFile);
  SourceRange decl_range(FD->getSourceRange());
  SourceLocation decl_begin(decl_range.getBegin());
  SourceLocation decl_start_end(decl_range.getEnd());

  SourceLocation decl_end_end(end_of_the_end(decl_start_end, Context.getSourceManager()));
  /*
     * Now you can fetch the pointers to the text replacement
     */
  const char* buff_begin(Context.getSourceManager().getCharacterData(decl_begin));
  const char* buff_end(Context.getSourceManager().getCharacterData(decl_end_end));
  std::string const func_string(buff_begin, buff_end);
  RB.Initialize("\n" + func_string + "\n");

  RB.write(outFile);
  Info.tainted_outfiles.push_back(&outFile);

  return true;
}
typedef enum {
  WASM_RT_I32,
  WASM_RT_I64,
  WASM_RT_F32,
  WASM_RT_F64,
} wasm_rt_type_t;

wasm_rt_type_t ReturnWasmArgType(ParmVarDecl *PV){
  if(PV->getType()->isVoidType()){
    return wasm_rt_type_t::WASM_RT_I32;
  }
  else if(PV->getType()->isTaintedPointerType()){
    return wasm_rt_type_t::WASM_RT_I32;
  }
  else if((PV->getType()->isIntegerType() || PV->getType()->isEnumeralType()))
  {
    return wasm_rt_type_t::WASM_RT_I64;
  }
  else if(PV->getType()->isFloatingType()) /* There should also be future support for 64 bit types*/
  {
    return wasm_rt_type_t::WASM_RT_F64;
  }
  else
  {
    assert("Unknown Callback Type Encountered");
  }
}

wasm_rt_type_t ReturnWasmArgType(QualType QV){
  if(QV.getTypePtr()->isVoidType()){
    return wasm_rt_type_t::WASM_RT_I32;
  }
  else if(QV.getTypePtr()->isTaintedPointerType()){
    return wasm_rt_type_t::WASM_RT_I32;
  }
  else if((QV.getTypePtr()->isIntegerType() || QV.getTypePtr()->isEnumeralType()))
  {
    return wasm_rt_type_t::WASM_RT_I64;
  }
  else if(QV.getTypePtr()->isFloatingType()) /* There should also be future support for 64 bit types*/
  {
      return wasm_rt_type_t::WASM_RT_F64;
    }
  else
  {
    assert("Unknown Callback Type Encountered");
  }
}

std::string ReturnRetParamType(ASTContext &Context, ProgramInfo &Info,
                                 Rewriter &R, VarDecl* FD){

  /*
   * Value types. Used to define function signatures.
  typedef enum {
    WASM_RT_I32,
    WASM_RT_I64,
    WASM_RT_F32,
    WASM_RT_F64,
  } wasm_rt_type_t;
  */

  std::string ArgString = "";
  std::vector<std::string> ret_param_entries;
  /*
   * First check the return type
   */
  ret_param_entries.push_back(itostr(ReturnWasmArgType(FD->getAsFunction()->getReturnType())));
  for(int i = 0 ; i < FD->getAsFunction()->getNumParams(); i++)
  {
    ret_param_entries.push_back(itostr(ReturnWasmArgType(FD->getAsFunction()->getParamDecl(i))));
  }

  /*
   * Now iterate through all the argument types and generate the array string
   */
  for(auto arg: ret_param_entries){
    ArgString += arg;
    ArgString += ",";
  }
  /*
   * Remove the trailing character
   */
    ArgString.pop_back();
    return ArgString;
}

bool WasmSandboxRewriteOp(ASTContext &Context, ProgramInfo &Info,
                          Rewriter &R)
{
  for(auto tainted_function_decls : Info.Tainted_Decls){


    if(!tainted_function_decls->hasBody())
    {
      continue;
    }

    /*
     *The Tainted function definition must move to a different file in order to
     * be compilable to wasm-readable definitions
     * Hence before writing anything, copy the entire original function to a different file
    */

    CopyTaintedDefToTaintedFile(Context, Info, R, tainted_function_decls);

    R.InsertTextAfterToken(tainted_function_decls->getBody()->getBeginLoc()
                               , "/*");
    R.InsertText(tainted_function_decls->getBody()->getEndLoc(), "*/",
                 false, true);
    std::map<VarDecl*, std::string> map_of_params;
    for(int i = 0 ; i < tainted_function_decls->getAsFunction()->getNumParams(); i++)
    {
      map_of_params.insert(std::pair<VarDecl*, std::string>(tainted_function_decls->getAsFunction()->getParamDecl(i),
                                                             tainted_function_decls->getAsFunction()->getNameAsString()));
    }
    //fetch the function name -->
    std::string function_name = tainted_function_decls->getAsFunction()->getNameAsString();
    //for wasm -->
    // append this with the w2c_ extension
    std::string wasm_function_name = "w2c_"+function_name;
    //now generate the return type -->
    std::string returnArg = "";
    std::vector<std::string> VarDecls ;

    bool isTaintedPointerReturn = false;
    if(tainted_function_decls->getAsFunction()->getReturnType()->isTaintedPointerType()){
      returnArg = "c_fetch_pointer_from_offset(";
      isTaintedPointerReturn = true;
      //now we generate appropriate cast because returned pointer is of tainted type -->
      std::string cast_operation = "("+tainted_function_decls->getAsFunction()->getReturnType().getAsString()+")";
      returnArg = cast_operation + returnArg;
    }

    //now generate the parameter list in form of a string -->
    std::string final_param_string = "";
    std::vector<std::string> set_of_params;
    /*
     * The first argument to the tainted function call will always be a
     * fetch_sandbox_address()
     */
    set_of_params.push_back("c_fetch_sandbox_address()");

    for(auto params: map_of_params)
    {
      std::string sbx_instrumented_param;
      if(params.first->getType()->isTaintedPointerType())
      {
        sbx_instrumented_param = "c_fetch_pointer_offset(" +
                                 params.first->getNameAsString()+")";
      }
      else if(params.first->getType()->isFunctionPointerType()){
        /*
         * This argument is a function pointer callback
         * Fetch the callback_trampoline index for this function name and
         * pass it as an argument
         */
         VarDecls.push_back("int ret_param_types[] = {"+
          ReturnRetParamType(Context,Info,R,
          params.first->getInitializingDeclaration())+"};\n");

         int ret_param = 0;
         if(!params.first->getAsFunction()->getReturnType()->isVoidType())
           ret_param = 1;

         sbx_instrumented_param = "sbx_register_callback("+
                                  params.first->getNameAsString() + " , "
                                  + itostr(params.first->getAsFunction()->getNumParams())
                                  + " , "
                                  + itostr(ret_param)
                                  + " , "
                                  + "ret_param_types)";
      }
      else if(isTaintedStruct(params.first))
      {

      }
      else{
        sbx_instrumented_param = params.first->getNameAsString();
      }
      set_of_params.push_back(sbx_instrumented_param);
    }

    //now keep iterating and entering commas
    for(int i = 0; i < set_of_params.size(); i ++)
    {
      final_param_string = final_param_string + set_of_params[i];
      if(i < (set_of_params.size()-1)){
        final_param_string = final_param_string + ",";
      }
    }

    //by here you must have a proper final_param_string -->

    // Now append it to required items and form the final call -->
    std::string FinalBoardingCall = "";
    for (auto init_decl : VarDecls)
    {
      FinalBoardingCall += init_decl;
    }
    FinalBoardingCall = "\n\nreturn " +returnArg + wasm_function_name+ "(" + final_param_string + ");";

    R.InsertTextAfter(tainted_function_decls->getBody()->getEndLoc(), FinalBoardingCall);
  }
}

bool SandboxSpecificRewriteOp(ASTContext &Context, ProgramInfo &Info,
                              Rewriter &R) {
  if (_CheckMateOpts.AddSandbox == "wasm") {
    return WasmSandboxRewriteOp(Context, Info, R);
  }
}
// This function is the public entry point for function body rewriting.
void DeclRewriter::rewriteBody(ASTContext &Context, ProgramInfo &Info,
                                Rewriter &R) {
  SandboxSpecificRewriteOp(Context, Info, R);
}

void DeclRewriter::rewrite(RSet &ToRewrite) {
  for (auto Pair : ToRewrite) {
    DeclReplacement *N = Pair.second;
    assert(N->getDecl() != nullptr);

    errs() << "Replacing type of decl:\n";
    N->getDecl()->dump();
    errs() << "with " << N->getReplacement() << "\n";


    // Exact rewriting procedure depends on declaration type
    if (auto *MR = dyn_cast<MultiDeclMemberReplacement>(N)) {
//      MultiDeclInfo *MDI =
//          Info.TheMultiDeclsInfo.findContainingMultiDecl(MR->getDecl());
//      assert("Missing MultiDeclInfo for multi-decl member" && MDI);
//      // A multi-decl can only be rewritten as a unit. If at least one member
//      // needs rewriting, then the first MultiDeclMemberReplacement in iteration
//      // order of ToRewrite (which need not have anything to do with member
//      // order of the multi-decl) triggers rewriting of the entire multi-decl,
//      // and rewriteMultiDecl checks ToRewrite for a MultiDeclMemberReplacement
//      // for each member of the multi-decl and applies it if found.
//      if (!MDI->AlreadyRewritten)
//        //rewriteMultiDecl(*MDI, ToRewrite);
//
      } else if (auto *FR = dyn_cast<FunctionDeclReplacement>(N)) {
      //rewriteFunctionDecl(FR);
    } else {
      assert(false && "Unknown replacement type");
    }
  }
}
//TagDecl Represents the declaration of a class/union/struct/TStruct
void DeclRewriter::denestTagDecls() {
  // When there are multiple levels of nested TagDecls, we need to process
  // all the children of a TagDecl TD before TD itself so that (1) the
  // definitions of the children end up before the definition of TD (since the
  // rewriter preserves order of insertions) and (2) the definitions of the
  // children have been removed from the body of TD before we read the body of
  // TD to move it. In effect, we want to process the TagDecls in postorder.
  // The easiest way to achieve this is to process them in order of their _end_
  // locations.
  std::sort(TagDeclsToDenest.begin(), TagDeclsToDenest.end(),
            [&](TagDecl *TD1, TagDecl *TD2) {
              return A.getSourceManager().isBeforeInTranslationUnit(
                  TD1->getEndLoc(), TD2->getEndLoc());
            });
  for (TagDecl *TD : TagDeclsToDenest) {
    // rewriteMultiDecl replaced the final "}" in the original source range with
    // "};\n", so the new content of the source range should include the ";\n",
    // which is what we want here. Except the rewriter has a bug where it
    // adjusts the token range to include the final token _after_ mapping the
    // offset to account for previous edits (it should be before). We work
    // around the bug by adjusting the token range before calling the rewriter
    // at all.
    CharSourceRange CSR = Lexer::makeFileCharRange(
        CharSourceRange::getTokenRange(TD->getSourceRange()), R.getSourceMgr(),
        R.getLangOpts());
    std::string DefinitionStr = R.getRewrittenText(CSR);
    // Delete the definition from the old location.
    rewriteSourceRange(R, CSR, "");
    // We want to find the highest ancestor DeclContext of TD that is a TagDecl
    // (call it TopTagDecl) and insert TD just before TopTagDecl.
    //
    // As of this writing, it seems that if TD is named, `TD->getDeclContext()`
    // returns the parent of TopTagDecl due to the code at
    // https://github.com/correctcomputation/checkedc-clang/blob/fd4d8af4383d40af10ee8bc92b7bf88061a11035/clang/lib/Sema/SemaDecl.cpp#L16980-L16981,
    // But that code doesn't run if TD is unnamed (which makes some sense
    // because name visibility isn't an issue for TagDecls that have no name),
    // and we want to de-nest TagDecls with names we assigned just like ones
    // that were originally named, so we can't just use `TD->getDeclContext()`.
    // In any event, maybe we wouldn't want to rely on this kind of internal
    // Clang behavior.
    TagDecl *TopTagDecl = TD;
    while (TagDecl *ParentTagDecl =
               dyn_cast<TagDecl>(TopTagDecl->getLexicalDeclContext()))
      TopTagDecl = ParentTagDecl;
    // If TopTagDecl is preceded by qualifiers, ideally we'd like to insert TD
    // before those qualifiers. If TopTagDecl is actually part of a multi-decl
    // with at least one member, then we can just use the begin location of that
    // multi-decl as the insertion point.
    //
    // If there are no members (so the qualifiers have no effect and generate a
    // compiler warning), then there isn't an easy way for us to find the source
    // location before the qualifiers. In that case, we just insert TD at the
    // begin location of TopTagDecl (after the qualifiers) and hope for the
    // best. In the cases we're aware of so far (storage qualifiers, including
    // `typedef`), this just means that the qualifiers end up applying to the
    // first TagDecl de-nested from TopTagDecl instead of to TopTagDecl itself,
    // and they generate the same compiler warning as before but on a different
    // TagDecl. However, we haven't confirmed that there aren't any obscure
    // cases that could lead to an error, such as if a qualifier is valid on one
    // kind of TagDecl but not another.
    SourceLocation InsertLoc;
    if (MultiDeclInfo *MDI =
            Info.TheMultiDeclsInfo.findContainingMultiDecl(TopTagDecl))
      InsertLoc = MDI->Members[0]->getBeginLoc();
    else
      InsertLoc = TopTagDecl->getBeginLoc();
    // TODO: Use a wrapper like rewriteSourceRange that tries harder with
    // macros, reports failure, etc.
    // (https://github.com/correctcomputation/checkedc-clang/issues/739)
    R.InsertText(InsertLoc, DefinitionStr);
  }
}

void DeclRewriter::rewriteFunctionBody(FunctionDeclReplacement *N) {

  /*
  rewriteSourceRange(R, N->getSourceRange(A.getSourceManager()),
                     N->getReplacement());
  */ // Since we will not be changing the declaration, we dont have to worry about rewriting the source range.
  if (N->getDecl()->isThisDeclarationADefinition()) {
    Stmt *S = N->getDecl()->getBody();
    assert("Supplementary declarations should only exist on rewritings for "
           "function definitions." && S != nullptr);
    // Insert supplementary declarations after the opening curly brace of the
    // function body.
//    emitTaintedFunctionInstrumentation(N->getTaintedFunctionInstrumentations(),
//                                  S->getBeginLoc());
  }

/*
 * Here you would need the FunctionBodyReplacement to provide you the following
 * 1.) Scope/SourceRange of the FunctionBody
 * 2.) getDecl --> to find out if the function body is of a tainted function or not
 * 3.) You should be able to fetch all teh function arguments and return types
 * 4.) You should be able to create a new Struct pointer and perform malloc and assignment operations
 * 5.) Basically you should be able to create calls
 */

}

void DeclRewriter::emitSupplementaryDeclarations(
  const std::vector<std::string> &SDecls, SourceLocation Loc) {
  // There are no supplementary declarations to emit. The AllDecls String
  // will remain empty, so insertText should no-op, but it's still an error to
  // insert an empty string at an invalid source location, so short circuit here
  // to be safe.
  if (SDecls.empty())
    return;

  std::string AllDecls;
  for (std::string D : SDecls)
    AllDecls += "\n" + D;

  R.InsertText(getLocationAfterToken(Loc, R.getSourceMgr(), R.getLangOpts()),
               AllDecls);
}

void DeclRewriter::emitTaintedFunctionInstrumentation(
    const std::vector<std::string> &TInsts, SourceLocation Loc) {
  // There are Tainted instructions to emit. The AllInsts String
  // will remain empty, so insertText should no-op, but it's still an error to
  // insert an empty string at an invalid source location, so short circuit here
  // to be safe.
  if (TInsts.empty())
    return;

  std::string AllInsts;
  for (std::string D : TInsts)
    AllInsts += "\n" + D + ";";

  R.InsertText(getLocationAfterToken(Loc, R.getSourceMgr(), R.getLangOpts()),
               AllInsts);
}

// Uses clangs lexer to find the location of the next comma or semicolon after
// the given source location. This is used to find the end of each declaration
// within a multi-declaration.
SourceLocation DeclRewriter::getNextCommaOrSemicolon(SourceLocation L) {
  SourceManager &SM = A.getSourceManager();
  auto Tok = Lexer::findNextToken(L, SM, A.getLangOpts());
  while (Tok.hasValue() && !Tok->is(clang::tok::eof)) {
    if (Tok->is(clang::tok::comma) || Tok->is(clang::tok::semi))
      return Tok->getLocation();
    Tok = Lexer::findNextToken(Tok->getEndLoc(), A.getSourceManager(),
                               A.getLangOpts());
  }
  llvm_unreachable("Unable to find comma or semicolon at source location.");
}

// This function checks how to re-write a function declaration.
// NOTE: this is an Overridden method, so dont go screwin around with this method
bool FunctionBodyBuilder::VisitFunctionDecl(FunctionDecl *FD) {

  // Get the constraint variable for the function.
  //*********************************************3C*****************************
  // For the return value and each of the parameters, do the following:
  //   1. Get a constraint variable representing the definition (def) and the
  //      uses ("arguments").
  //   2. If arguments could be wild but def is not, we insert a bounds-safe
  //      interface.
  // If we don't have a definition in scope, we can assert that all of
  // the constraint variables are equal.
  // Finally, we need to note that we've visited this particular function, and
  // that we shouldn't make one of these visits again.
  //*********************************************3C*****************************
  //*********************************************CheckMate*****************************
  // For the return value and each of the parameters, do the following:
  //   1. If this function is tainted, then comment out the entire function body
  //   2. Insert calls at the end of the function body that are sandbox specific
  // and that perform sandbox operations
  // If we don't have a definition in scope, we simply move-on
  // Finally, we need to note that we've visited this particular function, and
  // that we shouldn't make one of these visits again.
  auto FuncName = FD->getNameAsString();
  auto IsTainted = FD->isTainted();

  FVConstraint *FDConstraint = Info.getFuncConstraint(FD, Context);
  if (!FDConstraint)
    return true;

  // If this is an external function, there is no need to rewrite the
  // declaration. We cannot change the signature of external functions.
  // Under the flag -infer-types-for-undef, however, undefined functions do need
  // to be rewritten. If the rest of the CheckMate inference and rewriting code is
  // correct, short-circuiting here shouldn't be necessary; the rest of the
  // logic in this function should successfully not rewrite undefined functions
  // when -infer-types-for-undef is not passed. This assumption could be
  // transformed into an assertion if we're confident it won't fail in too many
  // places.
  if (!_CheckMateOpts.InferTypesForUndefs && !FDConstraint->hasBody())
    return true;

  // RewriteParams and RewriteReturn track if we will need to rewrite the
  // parameter and return type declarations on this function. They are first
  // set to true if any changes are made to the types of the parameter and
  // return. If a type has changed, then it must be rewritten. There are then
  // some special circumstances which require rewriting the parameter or return
  // even when the type as not changed.

  /*
   * Since we wouldnt be modifying the function Signature, we neednt need to do these
   */
  bool RewriteParams = false;
  bool RewriteReturn = false;

  // RewriteGeneric is similar to the above, but we need to further check
  // if the potential generic variables were set to wild by the constraint
  // resolver. In that case don't rewrite.
  bool RewriteGeneric = false;

  /*
   *
   * I am not modifying function signature, so I dont care about this
  bool DeclIsTypedef = false;
  TypeSourceInfo *TS = FD->getTypeSourceInfo();
  if (TS != nullptr) {
    // This still could possibly be a typedef type if TS was NULL.
    // TypeSourceInfo is null for implicit function declarations, so if a
    // implicit declaration uses a typedef, it will be missed. That's fine
    // since an implicit declaration can't be rewritten anyways.
    // There might be other ways it can be null that I'm not aware of.
    DeclIsTypedef = isa<TypedefType>(TS->getType());
  }
   */

  // If we've made this generic we need add "_For_any" or "_Itype_for_any"
  //  if (FDConstraint->getGenericParams() > 0
  //      && !FD->isGenericFunction() && !FD->isItypeGenericFunction())
  //    RewriteGeneric = true;

  // This will keep track of the supplementary declarations that are required by
  // function parameters. The new declarations will be emitted inside the
  // function body in the order of function parameters that generated them.
  /*
   * Okay, so this is what I want, and this is what I care about -->
   *
   */
  /*
   * So this vector will carry all the string information like calls,
   * tainted declarations etc to be emitted in the function body,
   * but before this, I need to comment out the entire function body
   */
  std::vector<std::string> SInsts;
  std::vector<std::string> TaintedFuncArgInsts;
  bool GenerateSInsts = FD->isThisDeclarationADefinition();

  // Get rewritten parameter variable declarations. Try to use
  // the source for as much as possible.
  std::vector<std::string> ParmStrs;

  // Needed to distinguish between Itype_for_any and For_any
  bool ProtoHasItype = false;

  // Typedefs must be expanded for now, so allow interpret them as rewritable
  // by ignoring their special case code.
  // See the FIXME below for more info.
  //  if (DeclIsTypedef) {
  //    // typedef: don't rewrite
  //  } else
  /*
   *
   * Now here below we iterate through all the parameters and for the tainted pointer
   * parameters, we generate Tainted Instrumentation calls
   */
  if (FD->getParametersSourceRange().isValid()) {
    // has its own params: alter them as necessary
    for (unsigned I = 0; I < FD->getNumParams(); ++I) {
      ParmVarDecl *PVDecl = FD->getParamDecl(I);
      if (isTaintedStruct(PVDecl) ||
          PVDecl->getType()->isTaintedPointerType()) {
        // there you go..
      }

      /*
       * Here we insert a string that will basically insert a call that would
       * fetch the pointer offset
       */
      if (PVDecl->getType()->isTaintedPointerType()) {
        std::string fetch_tptr_arg_offset =
            "c_fetch_pointer_offset(" + PVDecl->getNameAsString() + ")";
        TaintedFuncArgInsts.push_back(fetch_tptr_arg_offset);
      }

      const FVComponentVariable *CV = FDConstraint->getCombineParam(I);
      RewrittenBody RD = this->buildBodyVar(
          CV, PVDecl, PVDecl->getQualifiedNameAsString(), FD->isStatic());

      for (auto precall_actions : RD.PreCallInitialization) {
        ParmStrs.push_back(precall_actions);
      }

      ParmStrs.push_back(RD.CallSiteInstrumentation);

      for (auto postcall_actions : RD.PostCallInstrumentation) {
        ParmStrs.push_back(postcall_actions);
      }
    }
  } else if (FDConstraint->numParams() != 0) {
    // lacking params but the constraint has them: mirror the constraint
    //basically do the same thing as above
    for (unsigned I = 0; I < FDConstraint->numParams(); ++I) {
      ParmVarDecl *PVDecl = FD->getParamDecl(I);
      if (isTaintedStruct(PVDecl) ||
          PVDecl->getType()->isTaintedPointerType()) {
        // there you go..
      }

      /*
       * Here we insert a string that will basically insert a call that would
       * fetch the pointer offset
       */
      if (PVDecl->getType()->isTaintedPointerType()) {
        std::string fetch_tptr_arg_offset =
            "c_fetch_pointer_offset(" + PVDecl->getNameAsString() + ")";
        TaintedFuncArgInsts.push_back(fetch_tptr_arg_offset);
      }

      const FVComponentVariable *CV = FDConstraint->getCombineParam(I);
      RewrittenBody RD = this->buildBodyVar(
          CV, PVDecl, PVDecl->getQualifiedNameAsString(), FD->isStatic());

      for (auto precall_actions : RD.PreCallInitialization) {
        ParmStrs.push_back(precall_actions);
      }

      ParmStrs.push_back(RD.CallSiteInstrumentation);

      for (auto postcall_actions : RD.PostCallInstrumentation) {
        ParmStrs.push_back(postcall_actions);
      }
    }
    //  } else {
    //    // Functions in CheckedC need prototypes, so replace empty parameter lists
    //    // with an explict (void). This updates the parameter list; the rewrite flag
    //    // will be set once it is known if the return needs to be rewritten.
    //    ParmStrs.push_back("void");
    //  }

    // Get rewritten return variable.
    // For now we still need to check if this needs rewriting, see FIXME below
    // if (!DeclIsTypedef)
    /*
   * I dont care about re-writing the return variable as well -->
   */
    /*
  RewrittenDecl RewrittenReturn =
    this->buildDeclVar(FDConstraint->getCombineReturn(), FD, "", RewriteGeneric,
                       RewriteParams, RewriteReturn, FD->isStatic(),
                       GenerateSDecls);
  assert("Supplementary declarations should not be generated for return." &&
         RewrittenReturn.SupplementaryDecl.empty());

  ProtoHasItype |= !RewrittenReturn.IType.empty();

  // Generic forany and return are in the same rewrite location, so
  // we must rewrite the return if rewriting generic
  if (RewriteGeneric)
    RewriteReturn = true;
*/
    // If the return is a function pointer, we need to rewrite the whole
    // declaration even if no actual changes were made to the parameters because
    // the parameter for the function pointer type appear later in the source than
    // the parameters for the function declaration. It could probably be done
    // better, but getting the correct source locations is painful.
    /*
   * I am currently not handling function pointers
   */
    /*
  if (FD->getReturnType()->isFunctionPointerType() && RewriteReturn)
    RewriteParams = true;
  */
    // If we're making this into a generic function, we'll
    // rewrite parameters in case there's an itype in there that won't trigger
    // a normal rewrite. Temp fix for #678 in generics case.
    //  if (RewriteGeneric) {
    //    RewriteParams = true;
    //  }

    // If this function was declared without a prototype, then we must add one
    // to be able to give it a checked return type. This was done by adding "void"
    // to the parameter list above. Here we indicate the parameter list should be
    // rewritten to include "void" only if the return is already being rewritten.
    // This avoids unnecessarily adding void to empty parameter lists on unchecked
    // functions.
    //  if (TS && !TS->getType()->isFunctionProtoType() && RewriteReturn)
    //    RewriteParams = true;

    // If the function is declared using a typedef for the function type, then we
    // need to rewrite parameters and the return if either would have been
    // rewritten. What this does is expand the typedef to the full function type
    // to avoid the problem of rewriting inside the typedef.
    // FIXME: If issue #437 is fixed in way that preserves typedefs on function
    //        declarations, then this conditional should be removed to enable
    //        separate rewriting of return type and parameters on the
    //        corresponding definition.
    //        https://github.com/correctcomputation/checkedc-clang/issues/437
    //  if ((RewriteReturn || RewriteParams) && DeclIsTypedef) {
    //    RewriteParams = true;
    //    RewriteReturn = true;
    //  }

    // Mirrors the check above that sets RewriteGeneric to true.
    // If we've decided against making this generic, remove the generic params
    // so later rewrites (of typeparams) don't happen
    //  if (!RewriteGeneric && FDConstraint->getGenericParams() > 0
    //      && !FD->isGenericFunction() && !FD->isItypeGenericFunction())
    //    FDConstraint->resetGenericParams();

    // If this was an itype but is now checked, we'll be changing
    // "_Itype_for_any" to "_For_any"
    //  if (!RewriteGeneric && FD->isItypeGenericFunction() && !ProtoHasItype) {
    //    RewriteGeneric = true;
    //    RewriteReturn = true;
    //  }

    // Combine parameter and return variables rewritings into a single rewriting
    // for the entire function declaration.
    /*
   * For the millionth time, i dont intend to rewrite function return variables
   * or function parameter variables
   * But its quite interesting to see what is basically happeneing down below -->
   */

    std::string NewSig = "";
    //  if (RewriteGeneric) {
    //    if (ProtoHasItype)
    //      NewSig += "_Itype_for_any(T";
    //    else
    //      NewSig += "_For_any(T";
    //    for (int i = 0; i < FDConstraint->getGenericParams() - 1; i++) {
    //      assert(i < 2 &&
    //             "Need an unexpected number of type variables");
    //      NewSig += std::begin({",U",",V"})[i];
    //    }
    //    NewSig += ") ";
    //  }

    //  if (RewriteReturn)
    NewSig += getStorageQualifierString(FD) + FD->getReturnType().getAsString();
    //
    //  if (RewriteReturn && RewriteParams)
    NewSig += FDConstraint->getName();

    /*
   * Since _Tainted functions wont be accepting varargs as parameter type
   */
    //  if (RewriteParams && !ParmStrs.empty()) {
    //    // Gather individual parameter strings into a single buffer
    //    std::ostringstream ConcatParamStr;
    //    copy(ParmStrs.begin(), ParmStrs.end() - 1,
    //         std::ostream_iterator<std::string>(ConcatParamStr, ", "));
    //    ConcatParamStr << ParmStrs.back();
    //
    //    NewSig += "(" + ConcatParamStr.str();
    //    // Add varargs.
    //    if (functionHasVarArgs(FD))
    //      NewSig += ", ...";
    //    NewSig += ")";
    //  }
    //  NewSig = NewSig + RewrittenReturn.IType;

    // Add new declarations to RewriteThese if it has changed
    //  if (RewriteReturn || RewriteParams) {
    //    auto *FDR = new FunctionDeclReplacement(FD, NewSig, SDecls, RewriteReturn,
    //                                            RewriteParams, RewriteGeneric);
    //    RewriteThese.insert(std::make_pair(FD, FDR));
    //  }

    auto *FBR = new FunctionBodyReplacement(FD, SInsts);
    return true;
  }
}

  //RewrittenDecl
  //FunctionDeclBuilder::buildCheckedDecl(PVConstraint *Defn, DeclaratorDecl *Decl,
  //                                      std::string UseName, bool &RewriteParm,
  //                                      bool &RewriteRet, bool GenerateSDecls) {
  //  RewrittenDecl RD = DeclRewriter::buildCheckedDecl(Defn, Decl, UseName, Info,
  //                                                    ABRewriter, GenerateSDecls);
  //  RewriteParm |= getExistingIType(Defn).empty() != RD.IType.empty() ||
  //                 isa_and_nonnull<ParmVarDecl>(Decl);
  //  RewriteRet |= isa_and_nonnull<FunctionDecl>(Decl);
  //  return RD;
  //}
  //
  //RewrittenDecl
  //FunctionDeclBuilder::buildItypeDecl(PVConstraint *Defn, DeclaratorDecl *Decl,
  //                                    std::string UseName, bool &RewriteParm,
  //                                    bool &RewriteRet, bool GenerateSDecls,
  //                                    bool SDeclChecked) {
  //  Info.getPerfStats().incrementNumITypes();
  //  RewrittenDecl RD = DeclRewriter::buildItypeDecl(
  //      Defn, Decl, UseName, Info, ABRewriter, GenerateSDecls, SDeclChecked);
  //  RewriteParm = true;
  //  RewriteRet |= isa_and_nonnull<FunctionDecl>(Decl);
  //  return RD;
  //}

  // Note: For a parameter, Type + IType will give the full declaration (including
  // the name) but the breakdown between Type and IType is not guaranteed. For a
  // return, Type will be what goes before the name and IType will be what goes
  // after the parentheses.
  //RewrittenDecl
  //FunctionDeclBuilder::buildDeclVar(const FVComponentVariable *CV,
  //                                  DeclaratorDecl *Decl, std::string UseName,
  //                                  bool &RewriteGen, bool &RewriteParm,
  //                                  bool &RewriteRet, bool StaticFunc,
  //                                  bool GenerateSDecls) {
  //
  //  bool CheckedSolution = CV->hasCheckedSolution(Info.getConstraints());
  //  bool ItypeSolution = CV->hasItypeSolution(Info.getConstraints());
  //  if (ItypeSolution ||
  //      (CheckedSolution && _CheckMateOpts.ItypesForExtern && !StaticFunc)) {
  //    return buildItypeDecl(CV->getExternal(), Decl, UseName, RewriteParm,
  //                          RewriteRet, GenerateSDecls, CheckedSolution);
  //  }
  //  if (CheckedSolution) {
  //    return buildCheckedDecl(CV->getExternal(), Decl, UseName, RewriteParm,
  //                            RewriteRet, GenerateSDecls);
  //  }
  //
  //  // Don't add generics if one of the potential generic params is wild,
  //  // even if it could have an itype
  //  if (!CheckedSolution && CV->getExternal()->isGenericChanged())
  //    RewriteGen = false;
  //
  //// If the type of the pointer hasn't changed, then neither of the above
  //  // branches will be taken, but it's still possible for the bounds of an array
  //  // pointer to change.
  //  if (ABRewriter.hasNewBoundsString(CV->getExternal(), Decl)) {
  //    RewriteParm = true;
  //    RewriteRet |= isa_and_nonnull<FunctionDecl>(Decl);
  //  }
  //  std::string BoundsStr = ABRewriter.getBoundsString(
  //      CV->getExternal(), Decl, !getExistingIType(CV->getExternal()).empty());
  //
  //  // Variables that do not need to be rewritten fall through to here.
  //  // Try to use the source.
  //  std::string Type, IType;
  //  ParmVarDecl *PVD = dyn_cast_or_null<ParmVarDecl>(Decl);
  //  if (PVD && !PVD->getName().empty()) {
  //    SourceRange Range = PVD->getSourceRange();
  //    if (PVD->hasBoundsExpr())
  //      Range.setEnd(PVD->getBoundsExpr()->getEndLoc());
  //    if (Range.isValid() && !inParamMultiDecl(PVD))
  //      Type = getSourceText(Range, *Context);
  //    // Otherwise, reconstruct the name and type, and reuse the code below for
  //    // the itype and bounds.
  //    // TODO: Do we care about `register` or anything else this doesn't handle?
  //    if (Type.empty())
  //      Type = qtyToStr(PVD->getOriginalType(), PVD->getNameAsString());
  //  } else {
  //    Type = CV->mkTypeStr(Info.getConstraints(), true,
  //                         CV->getExternal()->getName());
  //  }
  //  IType = getExistingIType(CV->getExternal()) + BoundsStr;
  //  return RewrittenDecl(Type, IType, "");
  //}

  // Note: For a parameter, Type + IType will give the full declaration (including
  // the name) but the breakdown between Type and IType is not guaranteed. For a
  // return, Type will be what goes before the name and IType will be what goes
  // after the parentheses.
  RewrittenBody FunctionBodyBuilder::buildBodyVar(
      const FVComponentVariable *CV, DeclaratorDecl *Decl, std::string UseName,
    bool StaticFunc) {

//    bool CheckedSolution = CV->hasCheckedSolution(Info.getConstraints());
//    bool ItypeSolution = CV->hasItypeSolution(Info.getConstraints());
//    if (ItypeSolution ||
//        (CheckedSolution && _CheckMateOpts.ItypesForExtern && !StaticFunc)) {
//      return buildItypeDecl(CV->getExternal(), Decl, UseName, RewriteParm,
//                            RewriteRet, GenerateSDecls, CheckedSolution);
//    }
//    if (CheckedSolution) {
//      return buildCheckedDecl(CV->getExternal(), Decl, UseName, RewriteParm,
//                              RewriteRet, GenerateSDecls);
//    }

    // Don't add generics if one of the potential generic params is wild,
    // even if it could have an itype
//    if (!CheckedSolution && CV->getExternal()->isGenericChanged())
//      RewriteGen = false;

    // If the type of the pointer hasn't changed, then neither of the above
    // branches will be taken, but it's still possible for the bounds of an array
    // pointer to change.
//    if (ABRewriter.hasNewBoundsString(CV->getExternal(), Decl)) {
//      RewriteParm = true;
//      RewriteRet |= isa_and_nonnull<FunctionDecl>(Decl);
//    }

/*
 * We are not dealing with bounds too -->
 */
//    std::string BoundsStr = ABRewriter.getBoundsString(
//        CV->getExternal(), Decl, !getExistingIType(CV->getExternal()).empty());

    // Variables that do not need to be rewritten fall through to here.
    // Try to use the source.
//    std::string Type, IType;
//    ParmVarDecl *PVD = dyn_cast_or_null<ParmVarDecl>(Decl);
//    if (PVD && !PVD->getName().empty()) {
//      SourceRange Range = PVD->getSourceRange();
//      if (PVD->hasBoundsExpr())
//        Range.setEnd(PVD->getBoundsExpr()->getEndLoc());
//      if (Range.isValid() && !inParamMultiDecl(PVD))
//        Type = getSourceText(Range, *Context);
//      // Otherwise, reconstruct the name and type, and reuse the code below for
//      // the itype and bounds.
//      // TODO: Do we care about `register` or anything else this doesn't handle?
//      if (Type.empty())
//        Type = qtyToStr(PVD->getOriginalType(), PVD->getNameAsString());
//    } else {
//      Type = CV->mkTypeStr(Info.getConstraints(), true,
//                           CV->getExternal()->getName());
//    }
//    IType = getExistingIType(CV->getExternal()) + BoundsStr;
    //return RewrittenBody(Type, IType, "");
  // You have to generate your callback strings from here -->
  return RewrittenBody();
  }

  //std::string FunctionDeclBuilder::getExistingIType(ConstraintVariable *DeclC) {
  //  auto *PVC = dyn_cast<PVConstraint>(DeclC);
  //  if (PVC != nullptr && !PVC->getItype().empty())
  //    return " : " + PVC->getItype();
  //  return "";
  //}

  // Check if the function is handled by this visitor.
  bool FunctionBodyBuilder::isFunctionVisited(std::string FuncName) {
    return VisitedSet.find(FuncName) != VisitedSet.end();
  }

  //// K&R style function declarations can declare multiple parameter variables in
  //// a single declaration statement. The source ranges for these parameters
  //// overlap, so we cannot copy the declaration from source code to output code
  //bool FunctionDeclBuilder::inParamMultiDecl(const ParmVarDecl *PVD) {
  //  const DeclContext *DCtx = PVD->getDeclContext();
  //  if (DCtx) {
  //    SourceRange SR = PVD->getSourceRange();
  //    SourceManager &SM = Context->getSourceManager();
  //    for (auto *D : DCtx->decls())
  //      if (D != PVD && D->getBeginLoc().isValid() &&
  //          SM.isPointWithin(D->getBeginLoc(), SR.getBegin(), SR.getEnd()))
  //        return true;
  //  }
  //  return false;
  //}
