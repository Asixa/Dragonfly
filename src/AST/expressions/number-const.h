#ifndef NUMBER_CONST_H
#define NUMBER_CONST_H
#include "frontend/lexer.h"
#include "AST/expressions/expr.h"

namespace AST {
	// Expression node for literal numbers.
	class NumberConst final : public Expr {
	public:
		int type;
		double value;
		explicit NumberConst(const double d, const int t) : value(d) { type = t; }
		void ToString() override;
		llvm::Value* Gen(std::shared_ptr<DFContext>,const int cmd = 0) override;
	};
}
#endif