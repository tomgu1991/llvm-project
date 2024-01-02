//
// Created by guzuxing on 2024/1/2.
//

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Driver/Options.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include <iostream>
#include <string>
#include <unordered_set>

using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

#define LOG_MSG "gzx-count-func:"

// Apply a custom category to all command-line options so that they are the only
// ones displayed.
static llvm::cl::OptionCategory MyToolCategory("gzx-count-func options");
static const opt::OptTable &Options = clang::driver::getDriverOptTable();
static cl::opt<bool>
    Print_ALL("print-all", cl::desc("print all names, only number by default"),
              cl::cat(MyToolCategory));

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp
    MoreHelp("\nusage: gzx-count-func [--print-all] test.cpp --\n");

static std::unordered_set<std::string> Names;

// visitor to collect func name
class CountFuncVisitor : public clang::RecursiveASTVisitor<CountFuncVisitor> {
public:
  explicit CountFuncVisitor(clang::ASTContext *Context) : Context(Context) {}
  bool VisitFunctionDecl(clang::FunctionDecl *D) {
    std::string Name = D->getQualifiedNameAsString();
    Names.insert(Name);
    std::cout << LOG_MSG << "visit func: " << Name << std::endl;
    return true;
  }

private:
  clang::ASTContext *Context;
};

// consumer to call visitor
class CountFuncConsumer : public clang::ASTConsumer {
public:
  explicit CountFuncConsumer(clang::ASTContext *Context) : Visitor(Context) {}
  void HandleTranslationUnit(clang::ASTContext &Context) override {
    Context.getTranslationUnitDecl()->dump();
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  CountFuncVisitor Visitor;
};

// front end action to visit AST and count func num
class CountFuncAction : public clang::ASTFrontendAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &Compiler,
                    llvm::StringRef InFile) override {
    return std::make_unique<CountFuncConsumer>(&Compiler.getASTContext());
  }
};

int main(int argc, const char **argv) {
  std::cout << LOG_MSG << "start" << std::endl;
  auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyToolCategory);
  if (!ExpectedParser) {
    // Fail gracefully for unsupported options.
    llvm::errs() << ExpectedParser.takeError();
    return 1;
  }
  CommonOptionsParser &OptionsParser = ExpectedParser.get();
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());
  for (auto &Name : OptionsParser.getSourcePathList()) {
    std::cout << LOG_MSG << "file name: " << Name << std::endl;
  }
  std::cout << LOG_MSG << "run front end" << std::endl;
  int r = Tool.run(newFrontendActionFactory<CountFuncAction>().get());
  std::cout << LOG_MSG << "result: " << Names.size() << std::endl;
  if (Print_ALL) {
    std::cout << LOG_MSG << " with --print-all:" << std::endl;
    for (auto &N : Names) {
      std::cout << N << std::endl;
    }
  }
  std::cout << LOG_MSG << "finish" << std::endl;
  return r;
}