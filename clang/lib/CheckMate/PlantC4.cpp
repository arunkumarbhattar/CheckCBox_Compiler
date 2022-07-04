//=--RewriteUtils.cpp---------------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This class contains implementation of the functions and
// classes of PlantC4.h
//===----------------------------------------------------------------------===//

#include "clang/CheckMate/PlantC4.h"
#include "clang/CheckMate/CheckMateGlobalOptions.h"
#include "clang/CheckMate/TypeVariableAnalysis.h"
#include "clang/Tooling/Transformer/SourceCode.h"
#include <stdlib.h>
#include <unistd.h>
using namespace llvm;
using namespace clang;

bool PlantC4::ConvertTaintedToVanilla() {

  // Fetch the tainted directory
  Info.getC4PerfStats().startRewritingTime();

  auto list_of_tainted_files = Info.Tainted_rewrite_file_vector;
  for(auto tainted_file : list_of_tainted_files){
    std::string file_name_arg = tainted_file;
    // switch to C4 directory to execute the script
    chdir("../tools/C4");
    std::string command = "./exec.sh " + file_name_arg;
    std::system(command.c_str());
  }

  Info.getC4PerfStats().endRewritingTime();
  return true;
}
