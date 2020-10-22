#include "AST/expressions/string-expr.h"
#include "frontend/debug.h"

#include "frontend/lexer.h"

namespace AST {
	void String::ToString() {
	    *frontend::Debugger::out << "[\"" << value << "\"]";
	}
	llvm::Value* String::Gen(std::shared_ptr<DFContext> ctx,int cmd) {
		return ctx->builder->CreateGlobalStringPtr(llvm::StringRef(frontend::Lexer::MangleStr(value)));
	}
}
