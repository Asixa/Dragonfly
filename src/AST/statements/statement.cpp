#include "AST/statements/statement.h"
#include "parser.h"

namespace parser {
	std::shared_ptr<Statement> Statements::Parse() {
		Lexer::SkipNewlines(); VERIFY
			if (Lexer::Check('}')) return nullptr;
		const auto left = Statement::Parse(); VERIFY
			const auto right = Statements::Parse(); VERIFY
			if (right == nullptr)return left;
		return std::make_shared<Statements>(left, right);
	}
}
