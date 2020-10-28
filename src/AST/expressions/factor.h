#ifndef FACTOR_H
#define FACTOR_H
#include "AST/expressions/expr.h"
#include "frontend/lexer.h"

namespace AST {
	namespace expr {
		// Expression node for All general factors (including all expression nodes above)
		class Factor final : public Expr {
		public:
			void ToString() override;
			std::shared_ptr<AST::Type> Analysis(std::shared_ptr<DFContext>) override;
			llvm::Value* Gen(std::shared_ptr<DFContext>, const int cmd = 0) override;
			frontend::Lexer::Token* tok;
			explicit Factor(frontend::Lexer::Token* t) : tok(t) {}
			static std::shared_ptr<Expr> Parse();

		};
	}
}
#endif