#include "AST/expressions/string-expr.h"
#include "debug.h"
#include "codegen.h"

namespace parser {
	void String::ToString() {
	    *Debugger::out << "[\"" << value << "\"]";
	}
	llvm::Value* String::Gen(int cmd) {
		return CodeGen::builder.CreateGlobalStringPtr(llvm::StringRef(Lexer::MangleStr(value)));
	}
}
