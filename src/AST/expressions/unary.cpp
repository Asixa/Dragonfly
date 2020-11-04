#include "AST/expressions/unary.h"
#include "frontend/lexer.h"
#include "AST/expressions/factor.h"

using namespace frontend;
namespace AST {
	void expr::Unary::ToString() {
		*Debugger::out << "<" << Lexer::Token::Name(op) << ">";
		expr->ToString();
	}

	std::shared_ptr<expr::Expr> expr::Unary::ParsePostfix() {
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

	std::shared_ptr<expr::Expr> expr::Unary::Parse() {
		return ParsePostfix();
	}

	std::shared_ptr<AST::Type> expr::Unary::Analysis(const std::shared_ptr<DFContext> ctx) { return expr->Analysis(ctx); }

	std::shared_ptr<expr::Expr> expr::Unary::ParsePrefix() {
		switch (Lexer::token->type) {
		case NewLine:
			Debugger::CatchNewline();
			Debugger::Error("unexpected EndOfLine");
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
	llvm::Value* expr::Unary::Gen(std::shared_ptr<DFContext> context, bool is_ptr) {
		const auto v = expr->Gen(context);
		if (!v)return nullptr;
		switch (op) {
		case '-':
		case '~':
		case '!':
		case Inc:
		case Dec:
		default:
			return Debugger::ErrorV(line, ch,"invalid unary operator");
		}
	}

}
