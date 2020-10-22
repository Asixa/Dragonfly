#ifndef DO_STMT_H
#define DO_STMT_H
#include "AST/statements/statement.h"
#include "AST/expressions/expr.h"

namespace AST {
	namespace stmt {
		class Do final : public Statement {
		public:
			std::shared_ptr<expr::Expr> condition;
			std::shared_ptr<Statement> stmts;
			static std::shared_ptr<Do> Parse();
			void Gen(std::shared_ptr<DFContext>) override;
		};
	}
}
#endif