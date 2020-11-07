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
		return instance;
	}

    void Return::Analysis(std::shared_ptr<DFContext>ctx) {
		value->Analysis(ctx);
	}

    void Return::Gen(std::shared_ptr<DFContext> ctx) {
		//////////////////////////////////////////////////////////////////////////////
		/// State 1£¬ Gen the value, and check.
		//////////////////////////////////////////////////////////////////////////////
		if (value == nullptr) {
			ctx->builder->CreateRetVoid();
			return;
		}

		auto val = value->Gen(ctx);
		if (!val) {
			Debugger::ErrorV(line, ch,"Error in return");
			return;
		}

		//////////////////////////////////////////////////////////////////////////////
		/// State 2.a£¬ for generic function
		//////////////////////////////////////////////////////////////////////////////
		auto const function = ctx->builder->GetInsertBlock()->getParent();
	
		auto const expected = function->getReturnType();
		if (ctx->llvm->GetStructName(expected) != ctx->llvm->GetStructName(val)) {
			Debugger::ErrorV(line, ch,"return type not same" );
			return;
		}
		auto const expected_ptr_level = ctx->llvm->GetPtrDepth(expected);
		auto val_ptr_level = ctx->llvm->GetPtrDepth(val);
		while (val_ptr_level > expected_ptr_level) {
			val = ctx->builder->CreateLoad(val);
			val_ptr_level--;

            
		}
		ctx->builder->CreateRet(val);

	}
}
