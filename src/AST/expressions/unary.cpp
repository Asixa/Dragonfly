#include "AST/expressions/unary.h"
#include "lexer.h"
#include "AST/expressions/factor.h"
#include "codegen.h"

namespace parser {
	void Unary::ToString() {
		*Debugger::out << "<" << Lexer::Token::Name(op) << ">";
		expr->ToString();
	}

	std::shared_ptr<Expr> Unary::ParsePostfix() {
		const auto factor = ParsePrefix();
		switch (Lexer::token->type) {
		case Inc:
		case Dec:
			Lexer::Next();
			return std::make_shared<Unary>(factor, Lexer::token->type, false);
		default:
			return factor;
		}
	}

	std::shared_ptr<Expr> Unary::Parse() {
		return ParsePostfix();
	}

	std::shared_ptr<Expr> Unary::ParsePrefix() {
		switch (Lexer::token->type) {
		case NewLine:
			Debugger::CatchNewline();
			Debugger::Error(L"unexpected EndOfLine");
			return nullptr;
		case '-':
		case '!':
		case Inc:
		case Dec: {
			Lexer::Next();
			const auto parsed = Parse();
			return std::make_shared<Unary>(parsed, Lexer::token->type, true);
		}
		default:
			return Factor::Parse();
		}
	}
	llvm::Value* Unary::Gen(int cmd) {
		const auto v = expr->Gen();
		if (!v)return nullptr;
		switch (op) {
		case '-':
		case '~':
		case '!':
		case Inc:
		case Dec:
		default:
			return Debugger::ErrorV("invalid unary operator",line,ch);
		}
	}

}
