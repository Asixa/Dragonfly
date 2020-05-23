#ifndef EMPTY_STMT_H
#define EMPTY_STMT_H
#include "AST/statements/statement.h"
#include "AST/expressions/expr.h"

namespace parser {
	class Empty : public Statement {
		std::shared_ptr<Expr> value;
	public:
		static std::shared_ptr<Empty> Parse();
		void Gen() override;
		Empty(){}
        explicit Empty(const std::shared_ptr<Expr> v):value(v){}

	};

}
#endif