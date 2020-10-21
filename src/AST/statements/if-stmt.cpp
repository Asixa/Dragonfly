#include "AST/statements/if-stmt.h"
#include "AST/expressions/binary.h"
#include "AST/statements/statements.h"
#include "codegen.h"

namespace parser {
	std::shared_ptr<If> If::Parse() {
		const auto instance = std::make_shared<If>();
		Lexer::Next();
		Lexer::Match('(');
		instance->condition = Binary::Parse();
		Lexer::Match(')');
		if (Lexer::Check('{')) {
			Lexer::Next();
			instance->stmts = Statements::Parse();
			Lexer::Match('}');
		}
		else instance->stmts = Statement::Parse();


		if (Lexer::Check(K_else)) {
			Lexer::Next();
			if (Lexer::Check('{')) {
				Lexer::Next();
				instance->else_stmts = Statements::Parse();
				Lexer::Match('}');
			}
			else instance->else_stmts = Statement::Parse();
		}
		else if (Lexer::Check(K_elif))instance->else_stmts = Parse();

		return instance;
	}

	void If::Gen(std::shared_ptr<DFContext>ctx) {
		auto cond_v = condition->Gen(ctx);
		if (!cond_v) {
			Debugger::ErrorNonBreak(L"Error in condititon");
			return;
		}

		cond_v = ctx->builder->CreateICmpEQ(cond_v, ctx->True, "ifcond");
		auto function = ctx->builder->GetInsertBlock()->getParent();

		auto then_bb = llvm::BasicBlock::Create(ctx->context, "then", function);
		auto else_bb = llvm::BasicBlock::Create(ctx->context, "else");
		const auto merge_bb = llvm::BasicBlock::Create(ctx->context, "ifcont");

		ctx->builder->CreateCondBr(cond_v, then_bb, else_bb);

		ctx->builder->SetInsertPoint(then_bb);


		stmts->Gen(ctx);


		ctx->builder->CreateBr(merge_bb);
		// Codegen of 'Then' can change the current block, update ThenBB for the PHI.
		then_bb = ctx->builder->GetInsertBlock();

		// Emit else block.
		function->getBasicBlockList().push_back(else_bb);
		ctx->builder->SetInsertPoint(else_bb);

		else_stmts->Gen(ctx);


		ctx->builder->CreateBr(merge_bb);
		// Codegen of 'Else' can change the current block, update ElseBB for the PHI.
		else_bb = ctx->builder->GetInsertBlock();

		// Emit merge block.
		function->getBasicBlockList().push_back(merge_bb);
		ctx->builder->SetInsertPoint(merge_bb);

		// PHINode* PN = ctx->builder->CreatePHI(Type::getDoubleTy(ctx->the_context), 2, "iftmp");
		// PN->addIncoming(ThenV, ThenBB);
		// PN->addIncoming(ElseV, ElseBB);
		return;

	}
}
