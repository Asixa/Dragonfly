#ifndef FOR_STMT_H
#define FOR_STMT_H
#include "AST/statements/statement.h"
#include "AST/expressions/expr.h"

namespace parser {
	class For final : public Statement {
	public:
		std::shared_ptr<Statement> init, post;
		std::shared_ptr<Expr> condition;
		std::shared_ptr<Statement> stmts;
		static std::shared_ptr<For> Parse();
		void Gen() override;
	};

}
#endif