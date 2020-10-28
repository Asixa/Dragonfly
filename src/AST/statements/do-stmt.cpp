#include "AST/statements/do-stmt.h"
#include "AST/statements/statements.h"
#include "AST/expressions/binary.h"


namespace AST {
	using namespace  stmt;
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
		instance->condition = expr::Binary::Parse();
		Lexer::Match(')');
		return instance;
	}

    void Do::Analysis(std::shared_ptr<DFContext>) {}

    void Do::Gen(std::shared_ptr<DFContext> ctx) {
		auto cond_v = condition->Gen(ctx);
		if (!cond_v) {
			Debugger::ErrorNonBreak(L"Error in condititon");
			return;
		}
		cond_v = ctx->builder->CreateICmpEQ(cond_v, ctx->True, "cond");
		const auto function = ctx->builder->GetInsertBlock()->getParent();
		const auto while_bb = llvm::BasicBlock::Create(ctx->context, "do", function);
		const auto end_bb = llvm::BasicBlock::Create(ctx->context, "end_do");
		ctx->builder->SetInsertPoint(while_bb);
		stmts->Gen(ctx);
		ctx->builder->CreateCondBr(cond_v, while_bb, end_bb);
		ctx->builder->CreateBr(while_bb);
	}

}
