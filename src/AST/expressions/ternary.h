#ifndef TERNARY_H
#define TERNARY_H
#include "AST/expressions/expr.h"

namespace AST {
	// Expression node for Ternary expression like a?b:c
	class Ternary final : public Expr {
		std::shared_ptr<Expr> a;
		std::shared_ptr<Expr> b;
		std::shared_ptr<Expr> c;
	public:
		void ToString() override;
		llvm::Value* Gen(std::shared_ptr<DFContext>,const int cmd = 0) override;
		Ternary(std::shared_ptr<Expr> x, std::shared_ptr<Expr> y, std::shared_ptr<Expr> z) : a(x), b(y), c(z) {}
		static std::shared_ptr<Expr> Parse();
	};
}
#endif