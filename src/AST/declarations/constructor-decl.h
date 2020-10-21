#ifndef CONSTRUCTOR_DECL_H
#define CONSTRUCTOR_DECL_H
#include "AST/declarations/declaration.h"
#include "frontend/lexer.h"
#include "AST/declarations/function-decl.h"

namespace parser {
	class ConstructorDecl final :public Declaration {
		std::shared_ptr<FuncParam> args;
		std::shared_ptr<Statement> statements;
		void Gen() override;
		void GenHeader() override;
		static std::shared_ptr<ConstructorDecl> Parse();
	};
}
#endif