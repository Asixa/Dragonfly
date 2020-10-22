#include "AST/statements/return-stmt.h"
#include "frontend/lexer.h"
#include "AST/expressions/binary.h"


namespace AST {
	using namespace stmt;
	std::shared_ptr<Return> Return::Parse() {
		Lexer::Next();
		auto instance = std::make_shared<Return>();
		if (Lexer::token->type == ';' || Lexer::token->type == NewLine) {
			instance->value = nullptr;
			Lexer::Next();
			return instance;
		}
		instance->value = expr::Binary::Parse();
		Lexer::MatchSemicolon();
		*Debugger::out << "[Parsed] Return Statement\n";
		return instance;
	}
	void Return::Gen(std::shared_ptr<DFContext> ctx) {
		//////////////////////////////////////////////////////////////////////////////
		/// State 1�� Gen the value, and check.
		//////////////////////////////////////////////////////////////////////////////
		if (value == nullptr) {
			ctx->builder->CreateRetVoid();
			return;
		}

		auto val = value->Gen(ctx,1);
		if (!val) {
			Debugger::ErrorV("Error in return", line, ch);
			return;
		}

		//////////////////////////////////////////////////////////////////////////////
		/// State 2.a�� for generic function
		//////////////////////////////////////////////////////////////////////////////
		auto const function = ctx->builder->GetInsertBlock()->getParent();
	
		auto const expected = function->getReturnType();
		if (ctx->GetStructName(expected) != ctx->GetStructName(val)) {
			Debugger::ErrorV("return type not same", line, ch);
			return;
		}
		auto const expected_ptr_level = ctx->GetPtrDepth(expected);
		auto val_ptr_level = ctx->GetPtrDepth(val);
		while (val_ptr_level > expected_ptr_level) {
			val = ctx->builder->CreateLoad(val);
			val_ptr_level--;

            
		}
		ctx->builder->CreateRet(val);

	}
}
