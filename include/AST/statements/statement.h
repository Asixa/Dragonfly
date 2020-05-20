#ifndef STATEMENT_H
#define STATEMENT_H
#include "lexer.h"
#include "debug.h"
namespace parser {
	// Base Class for Statements
	class Statement {
	public:
	    int ch, line;
		Statement() {
			ch = Debugger::chp;
			line = Debugger::line;
        }
		virtual ~Statement() = default;
		virtual void Gen() = 0;
		static std::shared_ptr<Statement> Parse();
	};
}

#endif
