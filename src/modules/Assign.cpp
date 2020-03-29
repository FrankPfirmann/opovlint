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
#include <core/utility/ClangTemplateMatcher.h>

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

// Example of inner matcher expressions as macros wit "##mode" to enable type checking mode switches of ClangTemplateMatcher
#define asgnDeclCore(mode)                                                                      \
  varDecl(                                                                                      \
      isDefinition(),                                                                           \
      unless(anyOf(isTypedef##mode(type_s), isTypedef##mode(tolerated_type), isDependentType()))\
    , hasInitializer(ofType##mode(type_s))                                                      \
  ).bind("impl_decl")

#define asgnCore(mode)                                                                              \
  binaryOperator(                                                                                   \
    isAssignmentOperator(),                                                                         \
    hasLHS(unless(anyOf(ofType##mode(tolerated_type), ofType##mode(type_s), isDependentType()))),\
    hasRHS(ofType##mode(type_s))                                                                    \
  ).bind("impl_assign")

  auto decl_n = declMatcher(asgnDeclCore);
  auto decl_t = templateClassMatcher(asgnDeclCore);
  auto asgn_f1 = templateFunctionMatcher(asgnDeclCore);
  auto derived1 = derivedMatcher(asgnDeclCore);

  this->addMatcher(derived1);
  this->addMatcher(decl_n);
  this->addMatcher(decl_t);
  this->addMatcher(asgn_f1);

  auto asgn_n = stmtMatcher(asgnCore);
  auto asgn_t = templateClassMatcher(asgnCore);
  auto asgn_f2 = templateFunctionMatcher(asgnCore);
  auto derived2 = derivedMatcher(asgnCore);

  this->addMatcher(asgn_n);
  this->addMatcher(asgn_t);
  this->addMatcher(asgn_f2);
  this->addMatcher(derived2);
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
    ihandle.addIssue(expr, moduleName(), moduleDescription());  //, message.str());
  }
}

std::string Assign::moduleName() {
  return "Assign";
}

std::string Assign::moduleDescription() {
  return "Active types cannot be assigned to variables of non-active type, since the user-defined types do not have\
          conversion functions";
}

Assign::~Assign() = default;

} /* namespace module */
} /* namespace opov */
