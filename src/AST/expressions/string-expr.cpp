#include "AST/expressions/string-expr.h"
#include "frontend/debug.h"

#include "frontend/lexer.h"

namespace AST {
	void expr::String::ToString() {
	    *frontend::Debugger::out << "[\"" << value << "\"]";
	}
	llvm::Value* expr::String::Gen(std::shared_ptr<DFContext> ctx,int cmd) {
		return ctx->builder->CreateGlobalStringPtr(llvm::StringRef(frontend::Lexer::MangleStr(value)));
	}

    std::shared_ptr<AST::Type> expr::String::Analysis(std::shared_ptr<DFContext>) { return nullptr; }
}
