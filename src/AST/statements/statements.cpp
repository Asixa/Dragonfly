#include "AST/statements/statements.h"
#include "parser.h"

namespace parser {
	std::shared_ptr<Statement> Statement::Parse() {
		Lexer::SkipNewlines();
		VERIFY
			switch (Lexer::token->type) {
			case K_let:         return FieldDecl::Parse(true);
			case K_var:         return FieldDecl::Parse(false);
			case K_if:          return If::Parse();
			case K_return:      return Return::Parse();
			case K_do:          return For::Parse();
			case K_while:       return While::Parse();
			case K_for:         return For::Parse();
			case K_continue:    return Break::Parse();
			case K_break:       return Continue::Parse();
			default:            return Empty::Parse();
			}
	}

	void Statements::Gen() {
		stmt1->Gen();
		stmt2->Gen();
	}
}
