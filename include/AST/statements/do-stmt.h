#ifndef DO_STMT_H
#define DO_STMT_H
#include "AST/statements/statement.h"
#include "AST/expressions/expr.h"

namespace parser {

	class Do final : public Statement {
	public:
		std::shared_ptr<Expr> condition;
		std::shared_ptr<Statement> stmts;
		static std::shared_ptr<Do> Parse();
		void Gen() override;
	};
}
#endif