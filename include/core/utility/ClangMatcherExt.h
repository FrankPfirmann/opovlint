/*
 * ClangMatcherExt.h
 *
 *  Created on: May 26, 2014
 *      Author: ahueck
 */

#ifndef CORE_UTILITY_CLANGMATCHEREXT_H
#define CORE_UTILITY_CLANGMATCHEREXT_H

#include <core/logging/Logger.h>
#include <core/utility/ClangUtil.h>

#include <clang/ASTMatchers/ASTMatchers.h>

namespace clang {
namespace ast_matchers {

#define OPOV_CKIND(__NAME__)                    \
  AST_MATCHER(CastExpr, is##__NAME__) {         \
    return Node.getCastKind() == CK_##__NAME__; \
  }
OPOV_CKIND(FloatingToBoolean)
OPOV_CKIND(ConstructorConversion)
OPOV_CKIND(LValueToRValue)
#undef OPOV_CKIND


AST_POLYMORPHIC_MATCHER_P(isTypedef, AST_POLYMORPHIC_SUPPORTED_TYPES(Expr, Decl), std::string, type) {
  const auto typeOf_expr = Node.getType().getUnqualifiedType().getAsString();
  return type == typeOf_expr;
}

AST_POLYMORPHIC_MATCHER_P(isTypedefBase, AST_POLYMORPHIC_SUPPORTED_TYPES(Expr, Decl), std::string, type) {
  const auto typeOf_expr = Node.getType().getUnqualifiedType().getAsString();
  return type == typeOf_expr;
}

AST_POLYMORPHIC_MATCHER_P(isTypedefTemplate, AST_POLYMORPHIC_SUPPORTED_TYPES(Expr, Decl), std::string, type) {
  const auto typeOf_expr = Node.getType().getUnqualifiedType().getAsString();
  QualType ty = Node.getType();
  auto tys = ty->getAs<SubstTemplateTypeParmType>();
  return (tys != nullptr || type == typeOf_expr);
}

AST_MATCHER(Stmt, notABinaryExpr) {
  return !isa<BinaryOperator>(Node);
}

// const internal::VariadicDynCastAllOfMatcher<Stmt, ParenExpr> parenExpr;

//#define descendant_or_self(NODE) anyOf(NODE, hasDescendant(NODE))
//#define ancestor_or_self(NODE) anyOf(NODE, hasAncestor(NODE))

template <typename T>
inline auto children_or_self(const T& node) -> decltype(anyOf(node, has(node))) {
  return anyOf(node, has(node));
}

template <typename T>
inline auto ancestor_or_self(const T& node) -> decltype(anyOf(node, hasAncestor(node))) {
  return anyOf(node, hasAncestor(node));
}

template <typename T>
inline auto descendant_or_self(const T& node) -> decltype(anyOf(node, hasDescendant(node))) {
  return anyOf(node, hasDescendant(node));
}


inline QualType unravelTypedef(const QualType type){
    auto mtype = type;
    while(mtype->getAs<TypedefType>() != nullptr){
      mtype = mtype->getAs<TypedefType>()->desugar();
    }
    return mtype;
  }

// TODO remove this code duplication (hasThen) once backwards compatibility is not necessary
AST_MATCHER_P(IfStmt, hasThenStmt, internal::Matcher<Stmt>, InnerMatcher) {
  // Taken from the current version of Clangs ASTMatchers.h file: Line 2922
  const Stmt* const Then = Node.getThen();
  return (Then != nullptr && InnerMatcher.matches(*Then, Finder, Builder));
}

// TODO remove this code duplication (hasElse) once backwards compatibility is not necessary
AST_MATCHER_P(IfStmt, hasElseStmt, internal::Matcher<Stmt>, InnerMatcher) {
  // Taken from the current version of Clangs ASTMatchers.h file: Line 2934
  const Stmt* const Else = Node.getElse();
  return (Else != nullptr && InnerMatcher.matches(*Else, Finder, Builder));
}

AST_MATCHER(Stmt, isResolved){
  opov::clutil::DependentChecker depChecker;
  const bool is_dep = depChecker.hasType(const_cast<Stmt*>(&Node));
  return !is_dep;
  }

AST_POLYMORPHIC_MATCHER(isDependentType, AST_POLYMORPHIC_SUPPORTED_TYPES(ValueDecl, Expr)){
    return Node.getType()->isDependentType();
  }
// AST_MATCHER(TagDecl, isUnion) {
//  return Node.isUnion();
//}
AST_MATCHER(TemplateArgument, isSubstTempArg){
    if(Node.getKind() != clang::TemplateArgument::ArgKind::Type){
      return false;
    }
    else{
      auto retm = substTemplateTypeParmType();
      return retm.matches(*(Node.getAsType()), Finder, Builder);
    }
  }


AST_MATCHER_P(CallExpr, callExprWithTemplateType, std::string, type_s) {
  const clang::CXXMemberCallExpr *memcall = clang::dyn_cast<clang::CXXMemberCallExpr>(&Node);
  const clang::CXXOperatorCallExpr *opcall = clang::dyn_cast<clang::CXXOperatorCallExpr>(&Node);
  auto fdecl = Node.getDirectCallee();
  if(fdecl == nullptr){
    return false;
  }
  auto calltype = fdecl->getReturnType().getNonReferenceType();
  auto retm = substTemplateTypeParmType();
  auto tm =
    templateSpecializationType(hasAnyTemplateArgument(refersToType(asString(type_s))));
  QualType calleeType;
  if(memcall != nullptr){
    calleeType = memcall->getImplicitObjectArgument()->IgnoreImpCasts()->getType();
  }else if(opcall != nullptr){
    calleeType = opcall->getArg(0)->IgnoreImpCasts()->getType();
  }else{
    return false;
  }
  //auto& ct = Finder->getASTContext();
  //LOG_MSG("ofType: " << " : "  << " Statement: " << opov::clutil::node2str(ct, &Node));
  return (tm.matches(*(unravelTypedef(calleeType)), Finder, Builder))
  && retm.matches(*calltype, Finder, Builder);
}


AST_MATCHER_P(Stmt, ofType, std::string, type) {
  StatementMatcher ce = callExpr(callExprWithTemplateType(type));
  if(ce.matches(Node, Finder, Builder)){
    return true;
  }
  opov::clutil::TypeDeducer deducer(type);
  const bool is_type = deducer.hasType(const_cast<Stmt*>(&Node));

  return is_type;
}

AST_MATCHER_P2_OVERLOAD(Stmt, ofType, std::string, type, bool, isTemplate, 1) {
  StatementMatcher ce = callExpr(callExprWithTemplateType(type));
  if(ce.matches(Node, Finder, Builder)){
    return true;
  }
  auto deducer = isTemplate ? opov::clutil::TypeDeducer(type, true) : opov::clutil::TypeDeducer(type, false);
  const bool is_type = deducer.hasType(const_cast<Stmt*>(&Node));
  return is_type;
}

AST_MATCHER_P(Stmt, ofTypeBase, std::string, type){
  StatementMatcher ce = callExpr(callExprWithTemplateType(type));
  if(ce.matches(Node, Finder, Builder)){
    return true;
  }
  auto deducer = opov::clutil::TypeDeducer(type, false);
  const bool is_type = deducer.hasType(const_cast<Stmt*>(&Node));
  return is_type;
}

AST_MATCHER_P(Stmt, ofTypeTemplate, std::string, type){
  StatementMatcher ce = callExpr(callExprWithTemplateType(type));
  if(ce.matches(Node, Finder, Builder)){
    return true;
  }
  auto deducer = opov::clutil::TypeDeducer(type, true);
  const bool is_type = deducer.hasType(const_cast<Stmt*>(&Node));
  return is_type;
}


AST_MATCHER_P(CastExpr, hasSubExpr, internal::Matcher<Expr>, InnerMatcher) {
  const Expr* const SubExpression = Node.getSubExpr()->IgnoreParenImpCasts();
  return (SubExpression != nullptr && InnerMatcher.matches(*SubExpression, Finder, Builder));
}

AST_MATCHER_P(CXXConstructExpr, hasImplicitConversion, std::string, type) {
  const auto constr = Node.getConstructor();
  unsigned int ctor_pos = 0;
  const unsigned int num = Node.getNumArgs();
  opov::clutil::TypeDeducer deducer(type);
  for (auto ctor_param : constr->parameters()) {
    if (ctor_pos >= num) {
      // Will this ever be true? If yes: assume that default arg is used
      return ctor_param->hasDefaultArg();
    }
    Expr* arg_expr = const_cast<Expr*>(Node.getArg(ctor_pos)->IgnoreParenImpCasts());
    if (opov::clutil::typeOf(ctor_param) == type && !deducer.hasType(arg_expr)) {
      return true;
    }
    ++ctor_pos;
  }
  return false;
}

AST_MATCHER(CXXConstructorDecl, isCopyOrMoveCtor) {
  return Node.isCopyOrMoveConstructor();
}

AST_MATCHER_P(MaterializeTemporaryExpr, hasTemporary, internal::Matcher<Expr>, InnerMatcher) {
  return InnerMatcher.matches(*Node.GetTemporaryExpr(), Finder, Builder);
}


AST_MATCHER_P(NamedDecl, hasNameIn, std::vector<std::string>, names) {
  return std::find(names.begin(), names.end(), Node.getNameAsString()) != names.end();
}

AST_POLYMORPHIC_MATCHER(isAssignmentOperator,
                        AST_POLYMORPHIC_SUPPORTED_TYPES(BinaryOperator,
                                                        CXXOperatorCallExpr)) {
  return Node.isAssignmentOp();
}

AST_MATCHER(NestedNameSpecifier, isGlobalNamespace) {
  return Node.getKind() == NestedNameSpecifier::SpecifierKind::Global;
}



AST_MATCHER_P(TypedefDecl, hasScalarT, std::string, type) {
  auto q = Node.getUnderlyingType();
  TypeMatcher ttm = templateSpecializationType(hasAnyTemplateArgument(refersToType(asString(type))));
  return (ttm.matches(q, Finder, Builder));
}

AST_MATCHER_P(ClassTemplateSpecializationDecl, location, std::string, word) {
  auto& ct = Finder->getASTContext();
  auto& sm = ct.getSourceManager();
  auto s = Node.getPointOfInstantiation().printToString(sm);
  if (s.find("OpenFOAM") != std::string::npos){
    LOG_MSG(Node.getNameAsString());
    LOG_MSG(s);
  }
}
AST_MATCHER_P(DeclRefExpr, hasExplicitTemplateType, std::string, type) {
  auto temps = Node.template_arguments();
  for(auto t: temps){
    if (t.getArgument().getKind() == clang::TemplateArgument::ArgKind::Type
    &&t.getArgument().getAsType().getAsString() == type){
      return true;
    }
  }
  return false;
}
AST_MATCHER(CallExpr, callIsDerived){
  auto matcher = callExpr(anyOf(
  has(implicitCastExpr(hasCastKind(clang::CastKind::CK_UncheckedDerivedToBase))),
  has(memberExpr(has(implicitCastExpr(hasCastKind(clang::CastKind::CK_UncheckedDerivedToBase)))))
  ));
  return matcher.matches(Node, Finder, Builder);
}
AST_MATCHER_P(TypedefNameDecl, hasScalarBase, std::string, type){
    LOG_MSG(Node.getNameAsString());
  auto realtype = unravelTypedef(Node.getUnderlyingType());
  LOG_MSG(realtype.getAsString());
  auto temptype = templateSpecializationType(hasAnyTemplateArgument(refersToType(asString(type))));
  auto binder = templateSpecializationType(hasUnqualifiedDesugaredType(recordType().bind("active_base")));
  if(temptype.matches(*realtype, Finder, Builder)){
    binder.matches(*realtype, Finder, Builder);
    return true;
  }
}
AST_MATCHER_P(Stmt, ofTypeD, std::string, type) {
  StatementMatcher ce = callExpr(callExprWithTemplateType(type));
  auto hasDerived = stmt(hasDescendant(implicitCastExpr(hasCastKind(clang::CastKind::CK_UncheckedDerivedToBase),
  hasType(recordType(equalsBoundNode("active_base"))))));
  if(ce.matches(Node, Finder, Builder) || hasDerived.matches(Node, Finder, Builder)){
    return true;
  }
  opov::clutil::TypeDeducer deducer(type);
  const bool is_type = deducer.hasType(const_cast<Stmt*>(&Node));

  return is_type;
}
inline bool matchAllBases(CXXRecordDecl node, internal::BoundNodesTreeBuilder *whole, internal::Matcher<Type> inner, internal::ASTMatchFinder *finder) {
  bool matched = false;
  internal::BoundNodesTreeBuilder single;
  for (auto base: node.bases()) {
    auto type = unravelTypedef(base.getType());
    auto btype = type->getAsCXXRecordDecl();
    if(btype != nullptr){
      matched = matchAllBases(*btype, whole, inner, finder) || matched;
    }

    if(inner.matches(*type, finder, &single)){
      whole->addMatch(single);
      matched = true;
    }
  }
  return matched;
}


AST_MATCHER_P(CXXRecordDecl, forEachBase, internal::Matcher<Type>, InnerMatcher) {
  BoundNodesTreeBuilder whole;
  bool res = matchAllBases(Node, &whole, InnerMatcher, Finder);
  *Builder = std::move(whole);
  return res;
}
AST_MATCHER_P(Stmt, identity,internal::Matcher<Stmt>, InnerMatcher){
    return InnerMatcher.matches(Node, Finder, Builder);
  }
} /* namespace ast_matchers */
} /* namespace clang */

#endif  // CORE_UTILITY_CLANGMATCHEREXT_H
