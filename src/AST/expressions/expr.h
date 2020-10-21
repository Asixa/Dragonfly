#ifndef EXPR_H
#define EXPR_H
#include <llvm/IR/Value.h>
#include "frontend/debug.h"
namespace parser {
	// Base Class for Expressions
	class Expr {
	public:
		int ch, line;
        Expr() {
			ch = Debugger::ch;
			line = Debugger::line;
        }
		virtual ~Expr() = default;
		virtual void ToString() = 0;
		virtual llvm::Value* Gen(int cmd = 0) = 0;
	};
}

#endif
