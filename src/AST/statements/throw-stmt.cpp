#include "AST/statements/throw-stmt.h"
#include "AST/expressions/factor.h"

namespace parser {
	std::shared_ptr<Throw> Throw::Parse() {
		Lexer::Next();
		auto instance = std::make_shared<Throw>();
		instance->value = Factor::Parse();
		return instance;
	}

	void Throw::Gen(std::shared_ptr<DFContext> context) { }
}
