#include "AST/declarations/variable-decl.h"
#include "frontend/debug.h"
#include "frontend/lexer.h"
#include "AST/expressions/binary.h"


namespace AST {

	using namespace AST::decl;
	std::shared_ptr<VariableDecl> VariableDecl::Parse() {
		auto decl = std::make_shared<VariableDecl>();

		if (Lexer::Check(K_let)) decl->constant = true;
		else if (Lexer::Check(K_var)) decl->constant = false;
		Lexer::Match(decl->constant ? K_let : K_var);
		
		decl->name = Lexer::string_val;
		Lexer::Match(Id);
		
		if (Lexer::Check(':')) {
			Lexer::Next();
			decl->type = Type::Match();
		}

		Lexer::Match('=');
		decl->value = expr::Binary::Parse();
		Lexer::MatchSemicolon();
		return decl;
	}

	void VariableDecl::Analysis(std::shared_ptr<DFContext> ctx) {
		if (ctx->ast->GetField(name) != nullptr)
		{
            //ERROR
			Debugger::Error(L"name already existed");
            return;
		}
		ctx->ast->AddField(name, value->Analysis(ctx));
	}

	void VariableDecl::Gen(std::shared_ptr<DFContext> ctx) {

		const auto val = value->Gen(ctx);
		const auto ty = type==nullptr ? val->getType() : type->ToLLVM(ctx);
		if (!val) return;

		
		if (constant) {
            // TODO ERROR, constant not supported yet!  current problem is constant object
			const auto v = ctx->CreateGlobal(name, ty); 
			// v->setInitializer(val);
			ctx->llvm->AddField(name,v);
		}
		else {
			const auto the_function = ctx->builder->GetInsertBlock()->getParent();
			if (the_function->getName() == "main") {
				// All fields in main function are stored in heap. 
				const auto alloca = ctx->CreateEntryBlockAlloca(ty, name, the_function);
				ctx->AlignStore(ctx->builder->CreateStore(val, alloca));
				ctx->llvm->AddField(name,alloca);
			}
			else {
				// otherwise the local field store on stack. // TODO
				const auto alloca = ctx->CreateEntryBlockAlloca(ty, name,the_function);
				ctx->AlignStore(ctx->builder->CreateStore(val, alloca));
				ctx->llvm->AddField(name, alloca);
			}
		}
	}
}
