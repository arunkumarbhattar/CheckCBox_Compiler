//=--3C.h---------------------------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// The main interface for invoking TT tool.
// This provides various methods that can be used to access different
// aspects of the TT tool.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TT_TT_H
#define LLVM_CLANG_TT_TT_H

#include "clang/TT/TTInteractiveData.h"
#include "clang/TT/PersistentSourceLoc.h"
#include "clang/TT/ProgramInfo.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include <mutex>

// The main interface exposed by the TT to interact with the tool.
//
// See clang/docs/checkedc/TT/clang-tidy.md#_3c-name-prefix
// NOLINTNEXTLINE(readability-identifier-naming)
class _TTInterface {

public:
  ProgramInfo GlobalProgramInfo;
  // Mutex for this interface.
  std::mutex InterfaceMutex;

  // If the parameters are invalid, this function prints an error message to
  // stderr and returns null.
  //
  // There's no way for a constructor to report failure (we do not use
  // exceptions), so use a factory method instead. Ideally we'd use an
  // "optional" datatype that doesn't force heap allocation, but the only such
  // datatype that is accepted in our codebase
  // (https://llvm.org/docs/ProgrammersManual.html#fallible-constructors) seems
  // too unwieldy to use right now.
  static std::unique_ptr<_TTInterface>
  create(const struct _TTOptions &CCopt,
         const std::vector<std::string> &SourceFileList,
         clang::tooling::CompilationDatabase *CompDB);

  ~_TTInterface();

  // Call clang to provide the data
  bool parseASTs();

  // Constraints

  // Create ConstraintVariables to hold constraints
  bool addVariables();
//
//  // Build initial constraints.
//  bool buildInitialConstraints();
//
//  // Constraint Solving.
//  bool solveConstraints();

  // Interactivity.

  // Get all the WILD pointers and corresponding reason why they became WILD.
  ConstraintsInfo &getWildPtrsInfo();

  // Given a constraint key make the corresponding constraint var
  // to be non-WILD.
  bool makeSinglePtrNonWild(ConstraintKey TargetPtr);

  // Make the provided pointer non-WILD and also make all the
  // pointers, which are wild because of the same reason, as non-wild
  // as well.
  bool invalidateWildReasonGlobally(ConstraintKey PtrKey);

  // Rewriting.

  // Write all converted versions of the files in the source file list
  // to disk
  bool writeAllConvertedFilesToDisk();

  // Dump all stats related to performance.
  bool dumpStats();

  // Determine the exit code that the `3c` tool should exit with after the last
  // 3C stage, considering diagnostic verification. Must be called exactly once
  // before the _3CInterface is destructed (unless construction failed).
  int determineExitCode();

private:
  _TTInterface(const struct _TTOptions &CCopt,
               const std::vector<std::string> &SourceFileList,
               clang::tooling::CompilationDatabase *CompDB);

  bool ConstructionFailed = false;
  bool DeterminedExitCode = false;

  bool HadNonDiagnosticError = false;

  // Determine whether TT can continue to the next stage of processing. Checks
  // HadNonDiagnosticError and error diagnostics but ignores diagnostic
  // verification.
  bool isSuccessfulSoFar();

  // saved ASTs
  std::vector<std::unique_ptr<ASTUnit>> ASTs;

  // Are constraints already built?
  bool ConstraintsBuilt;
//  void invalidateAllConstraintsWithReason(Constraint *ConstraintToRemove);
};

#endif // LLVM_CLANG_TT_TT_H
