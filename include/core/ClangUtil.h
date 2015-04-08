/*
 * ClangUtil.h
 *
 *  Created on: May 13, 2014
 *      Author: ahueck
 */

#ifndef CLANGUTIL_H_
#define CLANGUTIL_H_

#include <core/Logger.h>

#include <clang/AST/AST.h>
#include <clang/Lex/Lexer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <string>
#include <sstream>
//#include <system_error>

namespace opov {

namespace clutil {

template<typename T>
inline clang::SourceRange locOf(const clang::SourceManager& sm, T node, unsigned int offset=0) {
	// offset=1 includes ';' (assuming no whitespaces)
	clang::SourceLocation start(node->getLocStart());
	clang::SourceLocation end(clang::Lexer::getLocForEndOfToken(node->getLocEnd(), 0, sm, clang::LangOptions()));
	return {start, end.getLocWithOffset(offset)};
}

template<typename NODE>
inline std::string typeOf(NODE node) {
	return node->getType().getUnqualifiedType().getAsString();
}

template<typename T>
inline std::string fileOriginOf(const clang::SourceManager& sm, T node) {
	const auto range = locOf(sm, node);
	const std::pair<clang::FileID, unsigned> DecomposedLocation =
			sm.getDecomposedLoc(range.getBegin());
	const clang::FileEntry *Entry = sm.getFileEntryForID(
			DecomposedLocation.first);
	if (Entry != NULL) {
		llvm::SmallString<256> FilePath(Entry->getName());
		llvm::error_code EC = llvm::sys::fs::make_absolute(FilePath);
		return EC ? FilePath.c_str() : Entry->getName();
	} else {
		return "";
	}
}

/*
 * Taken from the official clang documentation:
 * 	see: http://clang.llvm.org/docs/LibASTMatchersTutorial.html
 */
inline bool areSameExpr(clang::ASTContext* context, const clang::Expr* first,
		const clang::Expr* second) {
	if (!first || !second) {
		return false;
	}
	llvm::FoldingSetNodeID FirstID, SecondID;
	first->Profile(FirstID, *context, true);
	second->Profile(SecondID, *context, true);
	return FirstID == SecondID;
}

/*
inline std::string posOf(const clang::SourceManager& sm, const clang::Stmt* expr) {
	std::stringstream ss;
	ss << "Location -- Line (S/E): "
			<< sm.getPresumedLineNumber(expr->getLocStart()) << "/"
			<< sm.getPresumedLineNumber(expr->getLocEnd())
		<< " Column (S/E): "
			<< sm.getPresumedColumnNumber(expr->getLocStart()) << "/"
			<< sm.getPresumedColumnNumber(expr->getLocEnd());
	return ss.str();
}
*/

template<typename T>
inline std::tuple<unsigned, unsigned> rowOf(const clang::SourceManager& sm, T node) {
	auto range = locOf(sm, node);
	return std::make_tuple(sm.getPresumedLineNumber(range.getBegin()), sm.getPresumedLineNumber(range.getEnd()));
}

template<typename T>
inline std::tuple<unsigned, unsigned> colOf(const clang::SourceManager& sm, T node) {
	auto range = locOf(sm, node);
	return std::make_tuple(sm.getPresumedColumnNumber(range.getBegin()), sm.getPresumedColumnNumber(range.getEnd()));
}

template<typename T>
inline std::tuple<unsigned, unsigned, unsigned, unsigned> posOf(const clang::SourceManager& sm, T node) {
	return std::tuple_cat(rowOf(sm, node), colOf(sm, node));
}

template<typename NODE>
inline std::string node2str(const clang::SourceManager& sm, NODE expr) {
	auto range = locOf(sm, expr);
	auto startData = sm.getCharacterData(range.getBegin());
	const std::string source_code = std::string(startData,
			sm.getCharacterData(range.getEnd()) - startData);

	return source_code;
}

class TypeDeducer: public clang::RecursiveASTVisitor<TypeDeducer> {
private:
	bool subtreeHasType;
	std::string type;
	bool is_builtin;

public:
	TypeDeducer(const std::string& type) :
			subtreeHasType(false), type(type), is_builtin(isBuiltin(type)) {
	}

	template<typename NODE>
	bool hasType(NODE node) {
		this->subtreeHasType = false;
		this->TraverseStmt(node);
		return subtreeHasType;
	}

	bool TraverseStmt(clang::Stmt* S) {
		clang::Expr* expr = clang::dyn_cast<clang::Expr>(S);
		if (expr != nullptr) {
			if (typeFound(expr)) {
				subtreeHasType = true;
				return false;
			} else if (terminate(expr)) {
				return false;
			}
		}
		return RecursiveASTVisitor<TypeDeducer>::TraverseStmt(S);
	}

private:
	inline static bool isBuiltin(const std::string& type) {
		return type == "double" || type == "float";
	}

	inline bool typeFound(clang::Expr* expr) {
		// we cont. if binary/parens returns double/float, happens even with typedef types
		// TODO: possibly extend to unary ops too!
		return typeOf(expr) == type
				&& !(is_builtin
						&& (clang::isa<clang::BinaryOperator>(expr)
							|| clang::isa<clang::ImplicitCastExpr>(expr)
							|| clang::isa<clang::ParenExpr>(expr)));
	}

	inline bool terminate(clang::Expr* expr) {
		// TODO should we terminate on binary expr?
		const std::string typeOfE = typeOf(expr);
		return ((clang::isa<clang::UnaryOperator>(expr)
				|| clang::isa<clang::BinaryOperator>(expr))
				&& typeOfE == "_Bool")
				|| (clang::isa<clang::ExplicitCastExpr>(expr) && typeOfE != type);
	}
};

} // namespace clutil
} // namespace opov

#endif /* CLANGUTIL_H_ */