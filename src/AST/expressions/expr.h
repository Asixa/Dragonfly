#ifndef EXPR_H
#define EXPR_H
#include <llvm/IR/Value.h>
#include "frontend/debug.h"
#include "LLVM/context.h"
#include "frontend/symbol.h"

namespace AST {
	namespace expr {
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

			virtual std::shared_ptr<AST::Type> Analysis(std::shared_ptr<DFContext> ctx)=0;
			virtual llvm::Value* Gen(std::shared_ptr<DFContext>, bool is_ptr=false) = 0;
		};
	}
}

#endif
