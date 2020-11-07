#include "AST/expressions/boolean-expr.h"
#include "frontend/debug.h"

#include<iostream>
#include "AST/Type.h"

namespace AST {
	using namespace expr;
	void Boolean::ToString() {
	    *frontend::Debugger::out << "[" << (value ? "true" : "false") << "]";
	}

	std::shared_ptr<AST::Type> Boolean::Analysis(std::shared_ptr<DFContext>) {
	    return BasicType::Boolean;
	}

    llvm::Value* Boolean::Gen(std::shared_ptr<DFContext> ctx, bool is_ptr) {
		return value ? ctx->constant.True : ctx->constant.False;;
	}
}
