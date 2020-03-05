//
// Created by frank on 9/7/19.
//
#include <modules/Assign.h>
#include <core/configuration/Configuration.h>
#include <core/issue/IssueHandler.h>
#include <core/module/ModuleContext.h>
#include <core/module/ModuleRegistry.h>
#include <core/transformation/TransformationHandler.h>
#include <core/transformation/TransformationUtil.h>
#include <core/utility/ClangMatcherExt.h>
#include <core/utility/ClangUtil.h>
#include <iostream>

namespace opov {
namespace module {

REGISTER_MODULE(Assign)

using namespace clang;
using namespace clang::ast_matchers;

Assign::Assign() {
}

void Assign::setupOnce(const Configuration *config) {
  config->getValue("Assign:toleratedType", tolerated_type, "Foam::scalar");
}

void Assign::setupMatcher() {
  DeclarationMatcher impl_decl =
    varDecl(
      isDefinition()
      , unless(anyOf(isTypedef(type_s), isTypedef(tolerated_type)))
      , hasInitializer(
          ofType(type_s)
      )
    ).bind("impl_decl");


  StatementMatcher impl_asgn =
    binaryOperator(
      allOf(
      isAssignmentOperator(),
      unless(hasLHS(anyOf(ofType(tolerated_type), ofType(type_s)))),
      hasRHS(ofType(type_s))
      )
    ).bind("impl_assign");
  this->addMatcher(impl_decl);
  this->addMatcher(impl_asgn);
}

void Assign::run(const clang::ast_matchers::MatchFinder::MatchResult &result) {
  auto& ihandle = context->getIssueHandler();
  if (result.Nodes.getNodeAs<Expr>("impl_assign") == nullptr) {
    const auto expr = result.Nodes.getNodeAs<VarDecl>("impl_decl");
    auto type = clutil::typeOf(expr);
    if (type.find(type_s) == std::string::npos) {
      static const std::string module = moduleName() + "Decl";
      ihandle.addIssue(expr, module, moduleDescription());}  //, message.str());
  } else {
    const Expr* expr = result.Nodes.getNodeAs<Expr>("impl_assign");
    ihandle.addIssue(expr, moduleName(), moduleDescription());  //, message.str());*/
  }
}

std::string Assign::moduleName() {
  return "Assign";
}

std::string Assign::moduleDescription() {
  return "Non-scalar types cannot be assigned to scalar types";
}

Assign::~Assign() = default;

} /* namespace module */
} /* namespace opov */
