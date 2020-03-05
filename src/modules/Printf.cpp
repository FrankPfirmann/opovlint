//
// Created by frank on 7/31/19.
//

#include <modules/Printf.h>
#include <core/configuration/Configuration.h>
#include <core/issue/IssueHandler.h>
#include <core/module/ModuleContext.h>
#include <core/module/ModuleRegistry.h>
#include <core/transformation/TransformationHandler.h>
#include <core/transformation/TransformationUtil.h>
#include <core/utility/ClangMatcherExt.h>

namespace opov {
namespace module {

REGISTER_MODULE(Printf)

using namespace clang;
using namespace clang::ast_matchers;

Printf::Printf() {
}

void Printf::setupOnce(const Configuration *config) {
  config->getVector("Printf:functions", functions_n);
}

void Printf::setupMatcher() {
  // clang-format off
  StatementMatcher printf_matcher =
      callExpr(
        callee(
        functionDecl(
        hasNameIn(functions_n)))
        ,hasAnyArgument(ofType(type_s))
      ).bind("printf");
  // clang-format on
  this->addMatcher(printf_matcher);
}

void Printf::run(const clang::ast_matchers::MatchFinder::MatchResult &result) {
  //Only match functions with names specified in the config
  auto call = result.Nodes.getNodeAs<CallExpr>("printf");
  auto &ihandle = context->getIssueHandler();
  ihandle.addIssue(call, moduleName(), moduleDescription());
}

std::string Printf::moduleName() {
  return "Printf";
}

std::string Printf::moduleDescription() {
  return "Certain functions (e.g. *printf) dont implicitly work with overloaded complex types";
}

Printf::~Printf() = default;

} /* namespace module */
} /* namespace opov */
