#ifndef IF_STMT_H
#define IF_STMT_H
#include "AST/statements/statement.h"
#include "AST/expressions/expr.h"
#include "frontend/lexer.h"

namespace AST {
	namespace stmt {
		// class for if statement.
		class If : public Statement {
			std::shared_ptr<expr::Expr> condition;
			std::shared_ptr<Statement> stmts;
			std::shared_ptr<Statement> else_stmts;
		public:
			static std::shared_ptr<If> Parse();
			void Analysis(std::shared_ptr<DFContext>) override;
			void Gen(std::shared_ptr<DFContext>) override;
		};
	}
}
#endif