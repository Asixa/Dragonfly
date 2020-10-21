#ifndef FACTOR_H
#define FACTOR_H
#include "AST/expressions/expr.h"
#include "frontend/lexer.h"

namespace parser {
	// Expression node for All general factors (including all expression nodes above)
	class Factor final : public Expr {
	public:
		void ToString() override;
		llvm::Value* Gen(std::shared_ptr<DFContext>,const int cmd = 0) override;
		Lexer::Token* tok;
		explicit Factor(Lexer::Token* t) : tok(t) {}
		static std::shared_ptr<Expr> Parse();

	};
}
#endif