#include "AST/statements/throw-stmt.h"
#include "AST/expressions/factor.h"

namespace AST {
	using namespace stmt;
	std::shared_ptr<Throw> Throw::Parse() {
        frontend::Lexer::Next();
		auto instance = std::make_shared<Throw>();
		instance->value = expr::Factor::Parse();
		return instance;
	}

    void Throw::Analysis(std::shared_ptr<DFContext>) {}

    void Throw::Gen(std::shared_ptr<DFContext> context) { }
}
