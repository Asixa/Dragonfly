#include "AST/statements/for-stmt.h"
#include "AST/expressions/binary.h"
#include "AST/statements/statements.h"

namespace AST {
	std::shared_ptr<For> For::Parse() {
		const auto instance = std::make_shared<For>();
		Lexer::Next();
		Lexer::Match('(');
		instance->condition = Binary::Parse();
		Lexer::Match(')');
		if (Lexer::Check('{')) {
			Lexer::Next();
			instance->stmts = Statements::Parse();
			Lexer::Match('}');
		}
		else instance->stmts = Statement::Parse();
		return instance;
	}

	void For::Gen(std::shared_ptr<DFContext> context) {
		init->Gen( context);
	}

}
