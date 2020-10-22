#include "AST/statements/throw-stmt.h"
#include "AST/expressions/factor.h"

namespace AST {
	std::shared_ptr<Throw> Throw::Parse() {
        frontend::Lexer::Next();
		auto instance = std::make_shared<Throw>();
		instance->value = Factor::Parse();
		return instance;
	}

	void Throw::Gen(std::shared_ptr<DFContext> context) { }
}
