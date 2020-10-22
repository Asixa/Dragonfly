#include "AST/statements/statements.h"
#include "frontend/parser.h"

namespace AST {
	std::shared_ptr<Statement> Statement::Parse() {
		Lexer::SkipNewlines();
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

	void Statements::Gen(std::shared_ptr<DFContext> context) {
		stmt1->Gen(context);
		stmt2->Gen(context);
	}
}
