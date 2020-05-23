#include "AST/statements/empty-stmt.h"
#include "expressions/binary.h"

namespace parser {
	std::shared_ptr<Empty> Empty::Parse() {
		auto instance = std::make_shared<Empty>();
		instance->value = Binary::Parse();
			Lexer::MatchSemicolon();
			return instance;
	}

	void Empty::Gen() {
		value->Gen();
	}


}
