#ifndef UNARY_H
#define UNARY_H
#include "AST/expressions/expr.h"

namespace parser {
	// Expression node for factors with prefix or postfix)
	class Unary final : public Expr {
	public:
		void ToString() override;
		llvm::Value* Gen(const int cmd = 0) override;
		int op;
		bool prefix;
		std::shared_ptr<Expr> expr;
		Unary(std::shared_ptr<Expr> expr, const int op, bool pre) : op(op), expr(std::move(expr)), prefix(pre) {}
		static std::shared_ptr<Expr> ParsePrefix();
		static std::shared_ptr<Expr> ParsePostfix();
		static std::shared_ptr<Expr> Parse();
	};
}
#endif