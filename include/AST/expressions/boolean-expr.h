#ifndef BOOLEAN_H
#define BOOLEAN_H
#include "AST/expressions/expr.h"

namespace parser {
	// Expression node for literal booleans.
	class Boolean final : public Expr {
	public:
		void ToString() override;
		llvm::Value* Gen(const int cmd = 0) override;
		bool value;
		explicit Boolean(const bool d) : value(d) {}
	};
}
#endif