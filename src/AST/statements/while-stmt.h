#ifndef WHILE_STMT_H
#define WHILE_STMT_H
#include "AST/statements/statement.h"
#include "AST/expressions/expr.h"

namespace parser {
	class While final : public Statement {
	public:
		std::shared_ptr<Expr> condition;
		std::shared_ptr<Statement> stmts;
		static std::shared_ptr<While> Parse();
		void Gen(std::shared_ptr<DFContext>) override;
	};
}
#endif