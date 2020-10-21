#ifndef STATEMENTS_H
#define STATEMENTS_H
#include "frontend/lexer.h"
#include "AST/statements/statement.h"

namespace parser {
	// 'Statements' Class will match all statements in a binary-tree structure
	class Statements final : public Statement {
		std::shared_ptr<Statement> stmt1;
		std::shared_ptr<Statement> stmt2;
	public:
		Statements(std::shared_ptr<Statement> a, std::shared_ptr<Statement> b) : stmt1(a), stmt2(b) {}
		static std::shared_ptr<Statement> Parse();
		void Gen() override;
	};
}
#endif