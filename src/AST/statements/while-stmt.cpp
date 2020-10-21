#include "AST/statements/while-stmt.h"
#include "AST/expressions/binary.h"
#include "AST/statements/statements.h"
#include "codegen.h"

namespace parser {
	std::shared_ptr<While> While::Parse() {
		const auto instance = std::make_shared<While>();
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
		return instance;
	}
	void While::Gen(std::shared_ptr<DFContext> context) {

		auto cond_v = condition->Gen(context);
		if (!cond_v) {
			Debugger::ErrorNonBreak(L"Error in condititon");
			return;
		}
		cond_v = CodeGen::builder.CreateICmpEQ(cond_v, CodeGen::True, "cond");
		const auto function = CodeGen::builder.GetInsertBlock()->getParent();
		const auto while_bb = llvm::BasicBlock::Create(CodeGen::the_context, "while", function);
		const auto end_bb = llvm::BasicBlock::Create(CodeGen::the_context, "end_while");
		CodeGen::builder.SetInsertPoint(while_bb);
		CodeGen::builder.CreateCondBr(cond_v, while_bb, end_bb);
		stmts->Gen(context);
		CodeGen::builder.CreateBr(while_bb);
	}
}
