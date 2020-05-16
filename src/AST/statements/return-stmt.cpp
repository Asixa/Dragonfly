#include "AST/statements/return-stmt.h"
#include "lexer.h"
#include "AST/expressions/binary.h"
#include "codegen.h"

namespace parser {
	std::shared_ptr<Return> Return::Parse() {
		Lexer::Next();
		auto instance = std::make_shared<Return>();
		if (Lexer::token->type == ';' || Lexer::token->type == NewLine) {
			instance->value = nullptr;
			Lexer::Next();
			return instance;
		}
		instance->value = Binary::Parse();
		Lexer::MatchSemicolon();
		*Debugger::out << "[Parsed] Return Statement\n";
		return instance;
	}
	void Return::Gen() {
		if (value == nullptr) {
			CodeGen::builder.CreateRetVoid();
			return;
		}
		const auto val = value->Gen();
		if (!val) CodeGen::LogErrorV("Error in return");
		else CodeGen::builder.CreateRet(val);
	}
}
