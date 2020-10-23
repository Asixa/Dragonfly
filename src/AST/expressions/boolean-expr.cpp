#include "AST/expressions/boolean-expr.h"
#include "frontend/debug.h"

#include<iostream>

namespace AST {
	using namespace expr;
	void Boolean::ToString() {
	    *frontend::Debugger::out << "[" << (value ? "true" : "false") << "]";
	}
	llvm::Value* Boolean::Gen(std::shared_ptr<DFContext> ctx,int cmd) {
		return value ? ctx->True : ctx->False;;
	}
}
