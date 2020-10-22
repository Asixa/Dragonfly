#ifndef THROW_STMT_H
#define THROW_STMT_H
#include "AST/statements/statement.h"
#include "AST/expressions/expr.h"

namespace AST {
	namespace stmt {
		class Throw : Statement {
			std::shared_ptr<expr::Expr> value;
		public:
			static std::shared_ptr<Throw> Parse();
			void Gen(std::shared_ptr<DFContext>) override;
		};
	}
}
#endif