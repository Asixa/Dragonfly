#include "AST/declarations/field-decl.h"
#include "debug.h"
#include "lexer.h"
#include "AST/expressions/binary.h"
#include "codegen.h"

namespace parser {
	std::shared_ptr<FieldDecl> FieldDecl::Parse(const bool is_const) {
		auto let = std::make_shared<FieldDecl>();
		let->constant = is_const;
		Lexer::Next();
		VERIFY
			let->name = Lexer::string_val;
		Lexer::Match(Id);
		if (Lexer::Check(':')) {
			Lexer::Next();
			VERIFY
				if (Lexer::CheckType()) {
					let->type = static_cast<wchar_t>(Lexer::token->type);
					Lexer::Next();
					VERIFY
				}
				else {
					let->type = Lexer::string_val;
					Lexer::Match(Id);
					VERIFY
				}
		}
		Lexer::Match('=');
		VERIFY
			let->value = Binary::Parse();
		VERIFY

			Lexer::MatchSemicolon();
		VERIFY
			// PRINT("[Parsed] %s field declaration\n", is_const ? "Constant" : "Variable");let->value->ToString();PRINT("\n");
			return let;
	}

	void FieldDecl::Gen() {
		const auto mangled_name = CodeGen::MangleStr(name);
		const auto val = value->Gen();

		const auto ty = type.empty() ? val->getType() : CodeGen::GetType(type);
		if (!val) return;

		if (constant) {
			const auto const_v = static_cast<llvm::ConstantFP*>(val);
			const auto v = CodeGen::CreateGlob(CodeGen::builder, mangled_name, CodeGen::builder.getDoubleTy());
			v->setInitializer(const_v);

			CodeGen::local_fields_table[mangled_name] = v;
		}
		else {
			const auto the_function = CodeGen::builder.GetInsertBlock()->getParent();
			if (the_function->getName() == "main") {

				// All fields in main function are stored in heap.
				const auto alloca = CodeGen::CreateEntryBlockAlloca(the_function, ty, mangled_name);
				alloca->setAlignment(llvm::MaybeAlign(8));
				CodeGen::AlignStore(CodeGen::builder.CreateStore(val, alloca));
				CodeGen::global_fields_table[mangled_name] = alloca;
			}
			else {
				// otherwise the local field store on stack.
				const auto alloca = CodeGen::CreateEntryBlockAlloca(the_function, ty, mangled_name);
				alloca->setAlignment(llvm::MaybeAlign(8));
				CodeGen::AlignStore(CodeGen::builder.CreateStore(val, alloca));
				CodeGen::local_fields_table[mangled_name] = alloca;
			}
		}
	}
}
