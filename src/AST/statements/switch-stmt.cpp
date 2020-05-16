#include "AST/statements/switch-stmt.h"
namespace parser {

	std::shared_ptr<Switch> Switch::Parse() {
		Lexer::Next();
		Lexer::Match('(');
		Lexer::Match(')');
		Lexer::Match('{');

		while (true) {
			break;
		}

		Lexer::Match('}');
		return nullptr;
	}
}