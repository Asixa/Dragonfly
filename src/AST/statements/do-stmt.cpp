#include "AST/statements/do-stmt.h"
#include "AST/statements/statements.h"
#include "AST/expressions/binary.h"
#include "codegen.h"

namespace parser {
	std::shared_ptr<Do> Do::Parse() {
		const auto instance = std::make_shared<Do>();
		Lexer::Next();
		if (Lexer::Check('{')) {
			Lexer::Next();
			instance->stmts = Statements::Parse();
			Lexer::Match('}');
		}
		else instance->stmts = Statement::Parse();

		Lexer::Match(K_do);
		Lexer::Match('(');
		instance->condition = Binary::Parse();
		Lexer::Match(')');
		return instance;
	}
	void Do::Gen() {
		auto cond_v = condition->Gen();
		if (!cond_v) {
			Debugger::ErrorNonBreak(L"Error in condititon");
			return;
		}
		cond_v = CodeGen::builder.CreateICmpEQ(cond_v, CodeGen::True, "cond");
		const auto function = CodeGen::builder.GetInsertBlock()->getParent();
		const auto while_bb = llvm::BasicBlock::Create(CodeGen::the_context, "do", function);
		const auto end_bb = llvm::BasicBlock::Create(CodeGen::the_context, "end_do");
		CodeGen::builder.SetInsertPoint(while_bb);
		stmts->Gen();
		CodeGen::builder.CreateCondBr(cond_v, while_bb, end_bb);
		CodeGen::builder.CreateBr(while_bb);
	}

}
