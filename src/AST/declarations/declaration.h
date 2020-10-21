#ifndef DECLARATION_H
#define DECLARATION_H
#include "AST/statements/statement.h"

namespace parser {
	// Base Class for Declaration, which will generate codes before Statments
	class Declaration : public Statement {
	public:
		std::shared_ptr<Declaration>parent();
		virtual void GenHeader(std::shared_ptr<DFContext>) = 0;
	};
}
#endif