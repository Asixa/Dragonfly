#ifndef DECLARATION_H
#define DECLARATION_H
#include "AST/statements/statement.h"

namespace parser {
	// Base Class for Declaration, which will generate codes before Statments
	class Declaration : public Statement {
	public:
		virtual void GenHeader() = 0;
	};
}
#endif