#include "AST/expressions/factor.h"
#include "parser.h"
#include <sstream>

namespace parser {
	void Factor::ToString() {
	    *Debugger::out << "[Factor]";
	}

	std::shared_ptr<Expr> Factor::Parse() {

		std::shared_ptr<Expr> factor;
		switch (Lexer::token->type) {
		case '(': {
			Lexer::Next();
			// const auto point = src;
			// auto is_lambda = false;
			// Find('(', ')');
			// Next();												VERIFY
			// PRINT("ahead token is :"<<Token::Name(token->type)<<"\n", );
			// is_lambda = token->type == Arrow;
			// 	
			// PRINT("isLambda:%d   ", is_lambda);
			// src = point;
			// Next();												VERIFY
			// PRINT("current token is :"<<Token::Name(token->type)<<"\n");
			// system("PAUSE");
			// if (is_lambda)
			// {
			//
			// }
			// else {

			factor = Binary::Parse();
			VERIFY
				Lexer::Match(')');
			VERIFY
				return factor;
			// }
		}
		case Str: {
			auto str = std::make_shared<String>(Lexer::string_val);
			Lexer::Next();
			VERIFY
				return str;
		}

		case Num: {
			const auto ty = Lexer::token->value;
			Lexer::Next();
			VERIFY
				return std::make_shared<NumberConst>(Lexer::number_val, ty);
		}
		case K_true:
			Lexer::Next();
			VERIFY
				return std::make_shared<Boolean>(true);
		case K_false:
			Lexer::Next();
			VERIFY
				return std::make_shared<Boolean>(false);


		case Id: {
			return Field::Parse();
		}
		case K_int: case K_short: case K_long: case K_float: case K_double:
		case K_uint: case K_ushort: case K_ulong: case K_string:
		default:
			Debugger::Alert(
				(std::wstringstream() << "unexpected \"" << Lexer::Token::Name(Lexer::token->type) << "\" ").str());
			return nullptr;
		}
	}


	llvm::Value* Factor::Gen(int cmd) {
		return nullptr;
	}
}
