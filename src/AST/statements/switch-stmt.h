#ifndef SWITCH_STMT_H
#define SWITCH_STMT_H
#include "AST/statements/statement.h"
#include "AST/expressions/expr.h"

namespace parser {
	class Switch final : public Statement {
	public:
		std::shared_ptr<Expr> value;
		std::shared_ptr<Statement> stmts;
		static std::shared_ptr<Switch> Parse();
		void Gen() override;
	};
}
#endif