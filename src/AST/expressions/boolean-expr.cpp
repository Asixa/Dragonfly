#include "AST/expressions/boolean-expr.h"
#include "frontend/debug.h"
#include "codegen.h"


namespace parser {
	void Boolean::ToString() {
	    *Debugger::out << "[" << (value ? "true" : "false") << "]";
	}
	llvm::Value* Boolean::Gen(std::shared_ptr<DFContext> context,int cmd) {
		return value ? CodeGen::True : CodeGen::False;;
	}
}
