/*
 * ImplicitConversion.cpp
 *
 *  Created on: Jun 25, 2014
 *      Author: ahueck
 */

#include <modules/ImplicitConversion.h>
#include <core/utility/ClangMatcherExt.h>
#include <core/module/ModuleContext.h>
#include <core/utility/ClangUtil.h>
#include <core/utility/Util.h>
#include <core/configuration/Configuration.h>
#include <core/issue/IssueHandler.h>
#include <core/transformation/TransformationHandler.h>
//#include <modules/ExplicitCastVisitor.h>
#include <core/transformation/TransformationUtil.h>

namespace opov {
namespace module {

using namespace clang;
using namespace clang::ast_matchers;

ImplicitConversion::ImplicitConversion() {
}

void ImplicitConversion::setupOnce(const Configuration* config) {
}

void ImplicitConversion::setupMatcher() {
  StatementMatcher impl_conversion = materializeTemporaryExpr(hasTemporary(ignoringImpCasts(
      constructExpr(hasImplicitConversion(type_s), unless(temporaryObjectExpr())).bind("conversion"))));

  auto impl_assign = implicitCastExpr(
                hasSourceExpression(
                    allOf(ofType(type_s), unless(anyOf(floatLiteral(), ignoringParenImpCasts(explicitCastExpr()))))
                ), unless(anyOf(isLValueToRValue(), isTypedef(type_s)))).bind("impl_assign");

  this->addMatcher(impl_conversion);
  this->addMatcher(impl_assign);
}

void ImplicitConversion::run(const clang::ast_matchers::MatchFinder::MatchResult& result) {
  const Expr* expr = result.Nodes.getNodeAs<Expr>("conversion") == nullptr
                      ? result.Nodes.getNodeAs<Expr>("impl_assign")
                          : result.Nodes.getNodeAs<Expr>("conversion");

  auto& ihandle = context->getIssueHandler();
  ihandle.addIssue(expr, moduleName(), moduleDescription());  //, message.str());

  if (transform) {
    auto& thandle = context->getTransformationHandler();
    thandle.addReplacements(trutil::castTheExpr(context->getASTContext(), expr, type_s));
  }
}

std::string ImplicitConversion::moduleName() {
  return "ImplicitConversion";
}

std::string ImplicitConversion::moduleDescription() {
  return "Implicit conversions are problematic, since only one user-defined "
         "conversion is allowed on a single value. WIth the complex type, this "
         "can be easily violated.";
}

ImplicitConversion::~ImplicitConversion() {
}

} /* namespace module */
} /* namespace opov */
