#include "AST/Type.h"
#include "frontend/lexer.h"

AST::Type AST::Type::Match() {
	AST::Type type;
	if (frontend::Lexer::IsBasicType()) {
		type.ty = frontend::Lexer::token->type;
		type.str = "";
        frontend::Lexer::Next();
	}
	else {
		type.ty = 0;
		type.str = frontend::Lexer::string_val;
        frontend::Lexer::Match(Id);
		while (frontend::Lexer::Check('.')) {
            frontend::Lexer::Next();
			type.str += "." + frontend::Lexer::string_val;
            frontend::Lexer::Match(Id);
		}
		if (frontend::Lexer::Check('<')) {
            frontend::Lexer::Next();
			type.str += "<";
			if (frontend::Lexer::Check(Id)) {
                frontend::Lexer::Next();
				type.str += frontend::Lexer::string_val;
				while (frontend::Lexer::Check(',')) {
                    frontend::Lexer::Next();
                    frontend::Lexer::Match(Id);
					type.str += "," + frontend::Lexer::string_val;
				}
			}
            frontend::Lexer::Match('>');
			type.str += ">";
		}
	}
	if (frontend::Lexer::Check('[')) {
        frontend::Lexer::Next();
		if (frontend::Lexer::Check(Num)) {
			if (frontend::Lexer::token->value != K_int) {
                frontend::Debugger::Error(L"Value in side a array operator shoule be integer.");
				return type;
			}
			type.array = frontend::Lexer::number_val;
            frontend::Lexer::Next();
		}
		else {
			type.array = -2;
		}
        frontend::Lexer::Match(']');
	}
	return type;
}
