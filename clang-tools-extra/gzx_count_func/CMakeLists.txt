set(LLVM_LINK_COMPONENTS
  FrontendOpenMP
  Support
  )

add_clang_executable(gzx-count-func
  GzxCountFunc.cpp
  )

clang_target_link_libraries(gzx-count-func
  PRIVATE
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangTooling
  clangToolingRefactoring
  )
