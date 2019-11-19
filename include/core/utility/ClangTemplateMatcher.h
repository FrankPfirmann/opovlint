//
// Created by frank on 11/4/19.
//

#ifndef OPOV_CLANGTEMPLATEMATCHER_H
#define OPOV_CLANGTEMPLATEMATCHER_H
//TODO: Überschneidungen ignoriert, heuristical
//TODO: no way to exclude all specializations. weird one way relation. clang not intended for this mode thing
//TODO: macros together as one (so much redundancy ugh)
//Difference in matching from decl to call

#define declMatcher(inner)                              \
  decl(\
    inner(Base),\
    unless(hasAncestor(classTemplateSpecializationDecl())),\
    unless(hasAncestor(functionTemplateDecl())),\
    unless(hasAncestor(cxxMethodDecl(ofClass(forEachBase(templateSpecializationType(hasAnyTemplateArgument(refersToType(asString(type_s)))))))))\
  )

#define stmtMatcher(inner)                              \
  stmt(\
    inner(Base),\
    unless(hasAncestor(classTemplateSpecializationDecl())),\
    unless(hasAncestor(functionTemplateDecl())),\
    unless(hasAncestor(cxxMethodDecl(ofClass(forEachBase(templateSpecializationType(hasAnyTemplateArgument(refersToType(asString(type_s)))))))))\
  )

#define applyToCtsd(inner, mode) hasUnqualifiedDesugaredType(\
      recordType(\
        hasDeclaration(\
          classTemplateSpecializationDecl(\
            forEachDescendant(\
              inner(mode)\
            )\
          )\
        )\
      )\
    )
#define templateClassMatcher(inner)\
  templateSpecializationType(\
    anyOf(\
      allOf(\
        hasAnyTemplateArgument(refersToType(asString(type_s)))\
        , applyToCtsd(inner, Template)\
      )\
      , allOf(\
        unless(hasAnyTemplateArgument(refersToType(asString(type_s))))\
        , applyToCtsd(inner, Base)\
        , hasUnqualifiedDesugaredType(recordType(hasDeclaration(cxxRecordDecl(unless(forEachBase(templateSpecializationType(hasAnyTemplateArgument(refersToType(asString(type_s))))))))))\
      )\
    )\
  )

#define applyToCallee(inner, mode) callee(functionDecl(forEachDescendant(inner(mode))))
#define hasImpDeclRefExpr(inner) has(expr(ignoringImpCasts(declRefExpr(inner))))
#define templateFunctionMatcher(inner)\
  callExpr(\
    anyOf(\
      allOf(\
        hasImpDeclRefExpr(hasExplicitTemplateType(type_s))\
        , applyToCallee(inner, Template)\
       )\
      , allOf(\
        hasImpDeclRefExpr(unless(hasExplicitTemplateType(type_s)))\
        , applyToCallee(inner, Base)\
       )\
    )\
  )
#define activeRecordCTSD recordType(hasDeclaration(classTemplateSpecializationDecl(equalsBoundNode("active_base"))))

#define ofTypeDerived(type) \
  anyOf(\
    ofType(type)\
    , hasDescendant(\
      implicitCastExpr(\
        hasCastKind(clang::CastKind::CK_UncheckedDerivedToBase)\
        , anyOf(hasType(activeRecordCTSD), hasType(pointsTo(activeRecordCTSD)))\
      )\
    )\
  )

#define isTypedefDerived(type)\
  anyOf(\
    isTypedef(type)\
    , hasDescendant(\
      implicitCastExpr(\
        hasCastKind(clang::CastKind::CK_UncheckedDerivedToBase)\
        , anyOf(hasType(activeRecordCTSD), hasType(pointsTo(activeRecordCTSD)))\
      )\
    )\
  )

#define derivedMatcher(inner)\
  cxxMethodDecl(ofClass(\
    forEachBase(\
      templateSpecializationType(\
        allOf(\
          hasAnyTemplateArgument(refersToType(asString(type_s)))\
          , hasUnqualifiedDesugaredType(recordType(hasDeclaration(classTemplateSpecializationDecl().bind("active_base"))))\
        )\
    )))\
    , forEachDescendant(inner(Derived))\
  )

#endif //OPOV_CLANGTEMPLATEMATCHER_H
