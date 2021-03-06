#include "AST/statements/while-stmt.h"
#include "AST/expressions/binary.h"
#include "AST/statements/statements.h"


namespace AST {
	using namespace stmt;
	std::shared_ptr<While> While::Parse() {
		const auto instance = std::make_shared<While>();
		Lexer::Next();
		Lexer::Match('(');
		instance->condition = expr::Binary::Parse();
		Lexer::Match(')');
		if (Lexer::Check('{')) {
			Lexer::Next();
			instance->stmts = Statements::Parse();
			Lexer::Match('}');
		}
		else instance->stmts = Statement::Parse();
		return instance;
	}

    void While::Analysis(std::shared_ptr<DFContext>) {}

    void While::Gen(std::shared_ptr<DFContext> ctx) {

		auto cond_v = condition->Gen(ctx);
		if (!cond_v) {
			Debugger::ErrorNonBreak("Error in condititon");
			return;
		}
		cond_v = ctx->builder->CreateICmpEQ(cond_v, ctx->constant.True, "cond");
		const auto function = ctx->builder->GetInsertBlock()->getParent();
		const auto while_bb = llvm::BasicBlock::Create(ctx->context, "while", function);
		const auto end_bb = llvm::BasicBlock::Create(ctx->context, "end_while");
		ctx->builder->SetInsertPoint(while_bb);
		ctx->builder->CreateCondBr(cond_v, while_bb, end_bb);
		stmts->Gen(ctx);
		ctx->builder->CreateBr(while_bb);
	}
}
