#include "AST/declarations/function-decl.h"
#include "AST/statements/statements.h"
#include "codegen.h"

namespace parser {
	std::shared_ptr<FuncParam> FuncParam::Parse() {
		auto param = std::make_shared<FuncParam>();
		while (Lexer::token->type != ')') {
			param->size++;
			if (Lexer::CheckType()) {
				std::wstring t;
				t += static_cast<wchar_t>(Lexer::token->type);
				param->types.push_back(t);
				Lexer::Next();
				VERIFY
			}
			else if (Lexer::token->type == '.')		// Parse three dots '...' for variable argument.
			{
				Lexer::Next();
				VERIFY
					Lexer::Match('.');
				VERIFY
					Lexer::Match('.');
				VERIFY
					param->size--;
				param->isVarArg = true;
				return param;
			}
			else {
				param->types.push_back(Lexer::string_val);
				Lexer::Match(Id);
				VERIFY
			}

			param->names.push_back(Lexer::string_val);
			Lexer::Match(Id);
			VERIFY
				if (Lexer::Check(',')) Lexer::Match(',');
			VERIFY
		}
		return param;
	}

	std::shared_ptr<FunctionDecl> FunctionDecl::Parse(const bool ext) {
		auto function = std::make_shared<FunctionDecl>();
		function->is_extern = ext;
		if (Lexer::Check(K_dfunc))function->differentiable = true;
		else if (Lexer::Check(K_kernal))function->kernal = true;
		Lexer::Next();


		VERIFY
			if (Lexer::Check(Id)) {
				function->name = Lexer::string_val;
				Lexer::Match(Id);
				VERIFY
			}
		Lexer::Match('(');
		VERIFY
			function->args = FuncParam::Parse();
		Lexer::Match(')');
		VERIFY

			function->return_type = '0';
		if (Lexer::Check(':')) {
			Lexer::Next();
			VERIFY
				if (Lexer::CheckType()) {
					function->return_type = static_cast<wchar_t>(Lexer::token->type);
					Lexer::Next();
					VERIFY
				}
				else {
					function->return_type = Lexer::string_val;
					Lexer::Match(Id);
					VERIFY
				}

		}
		if (ext) {
			*Debugger::out << "[Parsed] Extern function declaration\n";
			return function;
		}

		Lexer::SkipNewlines();
		VERIFY

		*Debugger::out << "[Parsed] Function declaration\n";
		Lexer::Match('{');
		VERIFY

			function->statements = Statements::Parse();
		Lexer::SkipNewlines();
		VERIFY

			Lexer::Match('}');
		VERIFY
			* Debugger::out << "[Parsed] Function end\n";
		return function;
	}

	void FunctionDecl::GenHeader() {
		auto the_function = CodeGen::the_module->getFunction(CodeGen::MangleStr(name));

		if (!the_function) {
			std::vector<llvm::Type*> types;
			if (self_type != nullptr) {
				types.push_back(self_type->getPointerTo());
				args->names.insert(args->names.begin(), L"this");
			}
			for (auto i = 0; i < args->size; i++)
				types.push_back(CodeGen::GetType(args->types[i]));

			const auto func_type = llvm::FunctionType::get(CodeGen::GetType(return_type), types, args->isVarArg);
			the_function = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, CodeGen::MangleStr(name),
				CodeGen::the_module.get());

			unsigned idx = 0;
			for (auto& arg : the_function->args())
				arg.setName(CodeGen::MangleStr(args->names[idx++]));

		}
		else *Debugger::out << "function " << name << " already defined";
	}

	void FunctionDecl::Gen() {
		if (is_extern)return;
		auto function = CodeGen::the_module->getFunction(CodeGen::MangleStr(name));
		if (!function) {
			*Debugger::out << "function head not found\n";
			return;
		}
		const auto bb = llvm::BasicBlock::Create(CodeGen::the_context, CodeGen::MangleStr(name) + "_entry", function);
		CodeGen::builder.SetInsertPoint(bb);


		// CodeGen::This = nullptr;
		for (auto& arg : function->args())CodeGen::local_fields_table[arg.getName()] = &arg;

		//load base
		if (self_type != nullptr) {
			auto self_decl = CodeGen::types_table[self_type->getStructName()];
			if (!self_decl->base_type_name.empty()) {

				const auto alloca = CodeGen::CreateEntryBlockAlloca(function, CodeGen::GetType(self_decl->base_type_name), "base");
				alloca->setAlignment(llvm::MaybeAlign(8));
				auto base = CodeGen::FindMemberField(function->getArg(0), L"base");
				CodeGen::AlignStore(CodeGen::builder.CreateStore(
					CodeGen::builder.CreateLoad(base), alloca)
				);
				CodeGen::local_fields_table["base"] = alloca;
			}
		}


		// CodeGen::local_fields_table.clear();
		// CodeGen::This = nullptr;

		// CodeGen::the_function = function;
		if (statements != nullptr)statements->Gen();
		CodeGen::local_fields_table.clear();
		verifyFunction(*function);

	}

}
