set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  Option
  Support
  )

if (MSVC)
  set_source_files_properties(ArrayBoundsInferenceConsumer.cpp PROPERTIES COMPILE_FLAGS /bigobj)
  set_source_files_properties(ConstraintBuilder.cpp PROPERTIES COMPILE_FLAGS /bigobj)
  set_source_files_properties(DeclRewriter.cpp PROPERTIES COMPILE_FLAGS /bigobj)
  set_source_files_properties(RewriteUtils.cpp PROPERTIES COMPILE_FLAGS /bigobj)
endif()

add_clang_library(clangCheckMate
  ABounds.cpp
  AVarBoundsInfo.cpp
  AVarGraph.cpp
  ArrayBoundsInferenceConsumer.cpp
  CastPlacement.cpp
  CheckMate.cpp
  CheckMateInteractiveData.cpp
  CheckMateStats.cpp
  CheckedRegions.cpp
  ConstraintBuilder.cpp
  ConstraintResolver.cpp
  Constraints.cpp
  ConstraintsGraph.cpp
  ConstraintVariables.cpp
  CtxSensAVarBounds.cpp
  DeclRewriter.cpp
  IntermediateToolHook.cpp
  LowerBoundAssignment.cpp
  MappingVisitor.cpp
  MultiDecls.cpp
  PersistentSourceLoc.cpp
  ProgramInfo.cpp
  ProgramVar.cpp
  RewriteUtils.cpp
  PlantC4.cpp
  StructInit.cpp
  TypeVariableAnalysis.cpp
  Utils.cpp
  ${five_c_source}
  LINK_LIBS
  clangAST
  clangAnalysis
  clangBasic
  clangDriver
  clangFrontend
  clangRewriteFrontend
  clangStaticAnalyzerFrontend
  clangTooling
  clangTransformer
)
