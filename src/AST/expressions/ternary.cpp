#include "AST/expressions/ternary.h"
#include "frontend/debug.h"
#include "frontend/parser.h"

namespace AST {

	void expr::Ternary::ToString() {
		*Debugger::out << "[";
		a->ToString();
		*Debugger::out << "?";
		b->ToString();
		*Debugger::out << ":";
		c->ToString();
		*Debugger::out << "]";
	}

	std::shared_ptr<expr::Expr> expr::Ternary::Parse() {
		const auto a = Binary::Sub7();
		if (Lexer::token->type != '?')return a;
		Lexer::Next();
		const auto b = Binary::Sub7();
		Lexer::Match(':');
		const auto c = Binary::Sub7();
		return std::make_shared<Ternary>(a, b, c);
	}
	llvm::Value* expr::Ternary::Gen(std::shared_ptr<DFContext> context,int cmd) {
		return nullptr;
	}

}
