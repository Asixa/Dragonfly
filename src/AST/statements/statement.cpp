#include "AST/statements/statement.h"
#include "frontend/parser.h"

namespace AST {
	using namespace stmt;
	std::shared_ptr<Statement> Statements::Parse() {
		Lexer::SkipNewlines();
		if (Lexer::Check('}')) return nullptr;
		const auto left = Statement::Parse();
		const auto right = Statements::Parse();
		if (right == nullptr)return left;
		return std::make_shared<Statements>(left, right);
	}
}
