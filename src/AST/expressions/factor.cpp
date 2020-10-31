#include "AST/expressions/factor.h"
#include "AST/program.h"
#include <sstream>
#include "AST/expressions/new-expr.h"

namespace AST {
	using namespace expr;
	void Factor::ToString() {
	    *Debugger::out << "[Factor]";
	}

    std::shared_ptr<AST::Type> Factor::Analysis(std::shared_ptr<DFContext>) { return nullptr; }

    std::shared_ptr<Expr> Factor::Parse() {
		std::shared_ptr<Expr> factor;
		switch (Lexer::token->type) {
		case '(': {
			Lexer::Next();
			// const auto point = src;
			// auto is_lambda = false;
			// Find('(', ')');
			// Next();												
			// PRINT("ahead token is :"<<Token::Name(token->type)<<"\n", );
			// is_lambda = token->type == Arrow;
			// 	
			// PRINT("isLambda:%d   ", is_lambda);
			// src = point;
			// Next();												
			// PRINT("current token is :"<<Token::Name(token->type)<<"\n");
			// system("PAUSE");
			// if (is_lambda)
			// {
			//
			// }
			// else {

			factor = Binary::Parse();
			Lexer::Match(')');
			return factor;
			// }
		}
		case Str: {
			auto str = std::make_shared<String>(Lexer::wstring_val);
			Lexer::Next();
			return str;
		}

		case Num: {
			const auto ty = Lexer::token->value;
			Lexer::Next();
			return std::make_shared<NumberConst>(Lexer::number_val, ty);
		}
		case K_true:
			Lexer::Next();
			return std::make_shared<Boolean>(true);
		case K_false:
			Lexer::Next();
			return std::make_shared<Boolean>(false);
			// case K_new: {
			// 	return New::Parse();
			// }
		case Id: {
			printf("[Parsed field]\n");
			return Field::Parse();
		}
		case K_int: case K_short: case K_long: case K_float: case K_double:
		case K_uint: case K_ushort: case K_ulong: case K_string:
		default:
			Debugger::Error(
				(std::wstringstream() << "unexpected \"" << Lexer::Token::Name(Lexer::token->type) << "\" ").str());
			return nullptr;
		}

       
	}


	llvm::Value* Factor::Gen(std::shared_ptr<DFContext> context, bool is_ptr) {
		return nullptr;
	}
}
