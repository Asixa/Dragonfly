#include "AST/declarations/field-decl.h"
#include "frontend/debug.h"
#include "frontend/lexer.h"
#include "AST/expressions/binary.h"


namespace AST {

	using namespace AST::decl;
	std::shared_ptr<FieldDecl> FieldDecl::Parse(const bool is_const, const bool skip_keyword_check, const std::string* field_name) {
		auto let = std::make_shared<FieldDecl>();
		let->constant = is_const;
		if(!skip_keyword_check) Lexer::Match(is_const ? K_let : K_var);
		let->name = field_name == nullptr ? Lexer::string_val : *field_name;
		Lexer::Match(Id);
		if (Lexer::Check(':')) {
			Lexer::Next();
			let->type = Type::Match();
		}
		Lexer::Match('=');
		let->value = expr::Binary::Parse();
		Lexer::MatchSemicolon();
		return let;
	}

	void FieldDecl::Gen(std::shared_ptr<DFContext> ctx) {

		const auto val = value->Gen(ctx);
		const auto ty = type->empty() ? val->getType() : ctx->GetType(type);
		if (!val) return;

		if (constant) {
            // TODO ERROR, constant not supported yet! 
			const auto v = ctx->CreateGlob(name, ty);
			// v->setInitializer(val);
			ctx->local_fields_table[name] = v;
		}
		else {
			const auto the_function = ctx->builder->GetInsertBlock()->getParent();
			if (the_function->getName() == "main") {
				// All fields in main function are stored in heap. // TODO
				const auto alloca = ctx->CreateEntryBlockAlloca(ty, name, the_function);
				ctx->AlignStore(ctx->builder->CreateStore(val, alloca));
				ctx->global_fields_table[name] = alloca;
			}
			else {
				// otherwise the local field store on stack.
				const auto alloca = ctx->CreateEntryBlockAlloca(ty, name,the_function);
				ctx->AlignStore(ctx->builder->CreateStore(val, alloca));
				ctx->local_fields_table[name] = alloca;
			}
		}
	}
}
