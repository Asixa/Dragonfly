#include "AST/declarations/field-decl.h"
#include "frontend/debug.h"
#include "frontend/lexer.h"
#include "AST/expressions/binary.h"
#include "codegen.h"

namespace parser {
	std::shared_ptr<FieldDecl> FieldDecl::Parse(const bool is_const) {
		auto let = std::make_shared<FieldDecl>();
		let->constant = is_const;
		Lexer::Next();
		let->name = Lexer::string_val;
		Lexer::Match(Id);
		if (Lexer::Check(':')) {
			Lexer::Next();
			let->type = Lexer::MatchType();
		}
		Lexer::Match('=');
		
			let->value = Binary::Parse();
		

			Lexer::MatchSemicolon();
		
			// PRINT("[Parsed] %s field declaration\n", is_const ? "Constant" : "Variable");let->value->ToString();PRINT("\n");
			return let;
	}

	void FieldDecl::Gen() {

		const auto val = value->Gen();

		const auto ty = type.empty() ? val->getType() : CodeGen::GetType(type);
		if (!val) return;

		if (constant) {
            // TODO ERROR, constant not supported yet!
			const auto v = CodeGen::CreateGlob(name, ty);
			// v->setInitializer(val);
			CodeGen::local_fields_table[name] = v;
		}
		else {
			const auto the_function = CodeGen::builder.GetInsertBlock()->getParent();
			if (the_function->getName() == "main") {
				// All fields in main function are stored in heap.
				const auto alloca = CodeGen::CreateEntryBlockAlloca(ty, name, the_function);
				CodeGen::AlignStore(CodeGen::builder.CreateStore(val, alloca));
				CodeGen::global_fields_table[name] = alloca;
			}
			else {
				// otherwise the local field store on stack.
				const auto alloca = CodeGen::CreateEntryBlockAlloca(ty, name,the_function);
				CodeGen::AlignStore(CodeGen::builder.CreateStore(val, alloca));
				CodeGen::local_fields_table[name] = alloca;
			}
		}
	}
}
