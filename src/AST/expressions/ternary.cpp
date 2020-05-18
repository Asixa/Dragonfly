#include "AST/expressions/ternary.h"
#include "debug.h"
#include "parser.h"

namespace parser {

	void Ternary::ToString() {
		*Debugger::out << "[";
		a->ToString();
		*Debugger::out << "?";
		b->ToString();
		*Debugger::out << ":";
		c->ToString();
		*Debugger::out << "]";
	}

	std::shared_ptr<Expr> Ternary::Parse() {
		const auto a = Binary::Sub7();
		VERIFY
			if (Lexer::token->type != '?')return a;
		Lexer::Next();
		VERIFY
			const auto b = Binary::Sub7();
		VERIFY
			Lexer::Match(':');
		VERIFY
			const auto c = Binary::Sub7();
		VERIFY
			return std::make_shared<Ternary>(a, b, c);
	}
	llvm::Value* Ternary::Gen(int cmd) {
		return nullptr;
	}

}