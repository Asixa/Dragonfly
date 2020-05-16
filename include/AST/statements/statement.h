#ifndef STATEMENT_H
#define STATEMENT_H
#include "lexer.h"

namespace parser {
	// Base Class for Statements
	class Statement {
	public:
		virtual ~Statement() = default;
		virtual void Gen() = 0;
		static std::shared_ptr<Statement> Parse();
	};
}

#endif
