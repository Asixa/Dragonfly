#include "AST/statements/empty-stmt.h"
#include "AST/expressions/binary.h"

namespace AST {
	std::shared_ptr<Empty> Empty::Parse() {
		auto instance = std::make_shared<Empty>();
		instance->value = Binary::Parse();
			Lexer::MatchSemicolon();
			return instance;
	}

	void Empty::Gen(std::shared_ptr<DFContext> context) {
		value->Gen(context);
	}


}
