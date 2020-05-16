#ifndef THROW_STMT_H
#define THROW_STMT_H
#include "AST/statements/statement.h"
#include "AST/expressions/expr.h"

namespace parser {
	class Throw : Statement {
		std::shared_ptr<Expr> value;
	public:
		static std::shared_ptr<Throw> Parse();
		void Gen() override;
	};
}
#endif