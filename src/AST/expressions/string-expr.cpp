#include "AST/expressions/string-expr.h"
#include "frontend/debug.h"
#include "codegen.h"
#include "frontend/lexer.h"

namespace parser {
	void String::ToString() {
	    *Debugger::out << "[\"" << value << "\"]";
	}
	llvm::Value* String::Gen(std::shared_ptr<DFContext> ctx,int cmd) {
		return ctx->builder->CreateGlobalStringPtr(llvm::StringRef(Lexer::MangleStr(value)));
	}
}
