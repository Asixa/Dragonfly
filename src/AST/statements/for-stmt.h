#ifndef FOR_STMT_H
#define FOR_STMT_H
#include "AST/statements/statement.h"
#include "AST/expressions/expr.h"

namespace AST {
	namespace stmt {
		class For final : public Statement {
		public:
			std::shared_ptr<Statement> init;
			std::shared_ptr<expr::Expr> condition, post;
			std::shared_ptr<Statement> stmts;
			static std::shared_ptr<Statement> Parse();
			void Gen(std::shared_ptr<DFContext>) override;
		};

		class ForIterator final : public Statement {
		public:
			bool kernel;
			std::string name;
			std::shared_ptr<expr::Expr> collection;
			std::shared_ptr<Statement> stmts;
			static std::shared_ptr<ForIterator> Parse();
			void Gen(std::shared_ptr<DFContext>) override;
		};
		
	}

}
#endif