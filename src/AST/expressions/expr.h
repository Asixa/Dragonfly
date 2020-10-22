#ifndef EXPR_H
#define EXPR_H
#include <llvm/IR/Value.h>
#include "frontend/debug.h"
#include "LLVM/context.h"

namespace AST {
	// Base Class for Expressions
	class Expr {
	public:
		int ch, line;
        Expr() {
			ch = frontend::Debugger::ch;
			line = frontend::Debugger::line;
        }
		virtual ~Expr() = default;
		virtual void ToString() = 0;
		virtual llvm::Value* Gen(std::shared_ptr<DFContext>,int cmd = 0) = 0;
	};
}

#endif
