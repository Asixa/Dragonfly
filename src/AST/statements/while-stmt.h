#ifndef WHILE_STMT_H
#define WHILE_STMT_H
#include "AST/statements/statement.h"
#include "AST/expressions/expr.h"

namespace AST {
	namespace stmt {
		class While final : public Statement {
		public:
			std::shared_ptr<expr::Expr> condition;
			std::shared_ptr<Statement> stmts;
			static std::shared_ptr<While> Parse();
			void Analysis(std::shared_ptr<DFContext>) override;
			void Gen(std::shared_ptr<DFContext>) override;
		};
	}
}
#endif