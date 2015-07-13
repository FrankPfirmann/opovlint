/*
 * GlobalScope.cpp
 *
 *  Created on: June 09, 2015
 *      Author: ahueck
 */

#include <modules/GlobalScope.h>
#include <core/ClangMatcherExt.h>
#include <core/module/ModuleContext.h>
#include <core/Logger.h>
#include <core/ClangUtil.h>
#include <core/issue/IssueHandler.h>
#include <core/transformation/TransformationHandler.h>
#include <core/transformation/TransformationUtil.h>
#include <core/configuration/Configuration.h>

namespace opov {
namespace module {

using namespace clang;
using namespace clang::ast_matchers;

GlobalScope::GlobalScope() {

}

void GlobalScope::setupOnce(const Configuration* config) {
  config->getValue("global:type", type_s);
}

void GlobalScope::setupMatcher() {
  auto declref_matcher = declRefExpr(eachOf(to(functionDecl()), has(nestedNameSpecifier(isGlobalNamespace())))).bind("global");
  this->addMatcher(declref_matcher);
}

void GlobalScope::run(
  const clang::ast_matchers::MatchFinder::MatchResult& result) {
  const Expr* call = result.Nodes.getStmtAs<Expr>("global");
  auto& ihandle = context->getIssueHandler();
  auto& thandle = context->getTransformationHandler();
  auto& sm = context->getSourceManager();
  std::string exprStr;
  llvm::raw_string_ostream s(exprStr);
  call->printPretty(s, 0, context->getASTContext().getPrintingPolicy());
  ihandle.addIssue(sm, call, s.str(), moduleName(), moduleDescription());
}

std::string GlobalScope::moduleName() {
  return "GlobalScope";
}

std::string GlobalScope::moduleDescription() {
  return "Qualified lookup with ::.";
}

GlobalScope::~GlobalScope() {
}

} // namespace module
} /* namespace opov */