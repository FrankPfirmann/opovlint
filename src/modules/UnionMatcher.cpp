/*
 * UnionMatcher.cpp
 *
 *  Created on: Jun 9, 2014
 *      Author: ahueck
 */

#include <modules/UnionMatcher.h>
#include <modules/FieldDeclCollector.h>
#include <core/ClangMatcherExt.h>
#include <core/ModuleContext.h>
#include <core/Logger.h>
#include <core/ClangUtil.h>
#include <core/Util.h>
#include <core/IssueHandler.h>
#include <core/TransformationHandler.h>
#include <core/TransformationUtil.h>
#include <core/Configuration.h>

namespace opov {
namespace module {

using namespace clang;
using namespace clang::ast_matchers;

UnionMatcher::UnionMatcher() {

}

void UnionMatcher::setupOnce(const Configuration* config) {
	config->getValue("global:type", type_s);
	visitor = opov::util::make_unique<FieldDeclCollector>(type_s);
}

void UnionMatcher::setupMatcher() {
	LOG_DEBUG("Setup matcher!");
	DeclarationMatcher invalid_unions = recordDecl(isUnion(),
			hasDescendant(fieldDecl(isTypedef(type_s)))).bind("union");

	this->addMatcher(invalid_unions);
}

void UnionMatcher::run(
		const clang::ast_matchers::MatchFinder::MatchResult& result) {
	const CXXRecordDecl* inv_union = result.Nodes.getDeclAs<CXXRecordDecl>(
			"union");
	const bool is_anon = inv_union->isAnonymousStructOrUnion();
	auto fieldDecls = visitor->extractDecl(
			const_cast<CXXRecordDecl*>(inv_union));

	std::stringstream message;
	message << (is_anon ? "Anonymous union" : "Union")
			<< " with typedef member. " << fieldDecls.size()
			<< " fieldDecl violate the convention.";
	LOG_DEBUG(message.str());

	auto& thandle = context->getTransformationHandler();
	auto& ihandle = context->getIssueHandler();
	auto& sm = context->getSourceManager();
	ihandle.addIssue(sm, inv_union, moduleName(), moduleDescription(), message.str());
	if(is_anon) {
		const bool remove_union = std::distance(inv_union->field_begin(), inv_union->field_end()) == 1;
		if(remove_union) {
			// Highly unlikely to ever happen
			thandle.addReplacements(trutil::removeNode(sm, inv_union));
			thandle.addReplacements(trutil::insertNode(sm, fieldDecls.front(), inv_union));
		} else {
			for(auto fd : fieldDecls) {
				thandle.addReplacements(trutil::removeNode(sm, fd));
				thandle.addReplacements(trutil::insertNode(sm, fd, inv_union));
			}
		}
	} else {
		thandle.addReplacements(tooling::Replacement(sm, inv_union->getLocStart(), 5, "struct"));
	}
}

std::string UnionMatcher::moduleName() {
	return "UnionMatcher";
}

std::string UnionMatcher::moduleDescription() {
	return "Unions do not support complex types (complex constructor, destructor etc.).";
}

UnionMatcher::~UnionMatcher() {
}

} // namespace module
} /* namespace opov */