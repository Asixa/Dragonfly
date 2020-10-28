#ifndef SWITCH_STMT_H
#define SWITCH_STMT_H
#include "AST/statements/statement.h"
#include "AST/expressions/expr.h"

namespace AST {
	namespace stmt {
		class Switch final : public Statement {
		public:
			std::shared_ptr<expr::Expr> value;
			std::shared_ptr<Statement> stmts;
			static std::shared_ptr<Switch> Parse();
			void Analysis(std::shared_ptr<DFContext>) override;
			void Gen(std::shared_ptr<DFContext>) override;
		};
	}
}
#endif