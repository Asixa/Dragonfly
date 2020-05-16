#ifndef EXPR_H
#define EXPR_H
#include <llvm/IR/Value.h>

namespace parser {
	// Base Class for Expressions
	class Expr {
	public:
		virtual ~Expr() = default;
		virtual void ToString() = 0;
		virtual llvm::Value* Gen(int cmd = 0) = 0;
	};
}

#endif
