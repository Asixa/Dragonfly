#ifndef STATEMENT_H
#define STATEMENT_H
#include "frontend/lexer.h"
#include "frontend/debug.h"
#include "LLVM/context.h"
#include "frontend/ast-symbol.h"

namespace AST {
	namespace stmt {
		// Base Class for Statements
		class Statement {
		public:
			int ch, line;
			Statement() {
				ch = frontend::Debugger::chp;
				line = frontend::Debugger::line;
			}
			virtual ~Statement() = default;
			virtual void Gen(std::shared_ptr<DFContext>) = 0;
			virtual void Analysis(std::shared_ptr<DFContext> ctx);
			static std::shared_ptr<Statement> Parse();
		};
	}
}

#endif
