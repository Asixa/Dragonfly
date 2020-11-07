#include "AST/statements/for-stmt.h"

#include "empty-stmt.h"
#include "AST/expressions/binary.h"
#include "AST/expressions/factor.h"
#include "AST/statements/statements.h"

namespace AST {
	using namespace  stmt;
	std::shared_ptr<Statement> For::Parse() {
		auto* milestone = Lexer::src;
		Lexer::Next();
		Lexer::Match('(');
		if(Lexer::Check(K_var))
		{
			Lexer::Next();
			Lexer::Next();
			if(Lexer::Check(K_of))
			{
				// Parse for of
				Lexer::src = milestone;
				return ForIterator::Parse();
			}
		}

		// Parse for i
		Lexer::src = milestone;
		const auto instance = std::make_shared<For>();
		Lexer::Next();
		Lexer::Match('(');
		instance->init = Statement::Parse();
		auto* t = Lexer::token;
		instance->condition = expr::Binary::Parse();
		Lexer::Match(';');
		instance->post = expr::Binary::Parse();
		Lexer::Match(')');
		if (Lexer::Check('{')) {
			Lexer::Next();
			instance->stmts = Statements::Parse();
			Lexer::Match('}');
		}
		else instance->stmts = Statement::Parse();
		return instance;
	}

    void For::Analysis(std::shared_ptr<DFContext>) {}

    void For::Gen(std::shared_ptr<DFContext> context) {
		auto* const function = context->builder->GetInsertBlock()->getParent();

		init->Gen(context);

		auto* const cond_bb = llvm::BasicBlock::Create(context->context, "cond", function);
		auto* const stmt_bb = llvm::BasicBlock::Create(context->context, "stmt", function);
		auto* const end_bb = llvm::BasicBlock::Create(context->context, "end", function);

		context->builder->CreateBr(cond_bb);
		context->builder->SetInsertPoint(cond_bb);
		auto* cond_v = condition->Gen(context);
		
		cond_v = context->builder->CreateICmpEQ(cond_v, context->constant.True, "forcond");
		context->builder->CreateCondBr(cond_v, stmt_bb, end_bb);

		context->builder->SetInsertPoint(stmt_bb);
		stmts->Gen(context);
		post->Gen(context);
		context->builder->CreateBr(cond_bb);

		context->builder->SetInsertPoint(end_bb);
	}

	std::shared_ptr<ForIterator> ForIterator::Parse() {
		auto instance = std::make_shared<ForIterator>();

		Lexer::Next();
		Lexer::Match('(');
		Lexer::Match(K_var);
		instance->name = Lexer::string_val;
		Lexer::Match(Id);

		Lexer::Match(K_of);
		instance->collection = expr::Binary::Parse();
		Lexer::Match(')');

		if (Lexer::Check('{')) {
			Lexer::Next();
			instance->stmts = Statements::Parse();
			Lexer::Match('}');
		}
		else instance->stmts = Statement::Parse();
		
		return instance;
	}

    void ForIterator::Analysis(std::shared_ptr<DFContext>) {}

    void ForIterator::Gen(std::shared_ptr<DFContext>) {
		
	}
}
