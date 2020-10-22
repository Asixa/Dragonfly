#ifndef RETURN_STMT_H
#define RETURN_STMT_H
#include "AST/statements/statement.h"
#include "AST/expressions/expr.h"

namespace AST {
	class Return : public Statement {
		std::shared_ptr<Expr> value;
	public:
		static std::shared_ptr<Return> Parse();
		void Gen(std::shared_ptr<DFContext>) override;
	};
}
#endif