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
		auto has_name = true;
		auto function = std::make_shared<FunctionDecl>();
		function->is_extern = ext;
		if (Lexer::Check(K_dfunc))function->differentiable = true;
		else if (Lexer::Check(K_kernal))function->kernal = true;
		else if (Lexer::Check(K_init)) {
			has_name = false;
			function->name = L"::init";
		}
		else if (Lexer::Check(K_delete))
		{
			has_name = false;
			function->name = L"::delete";
		}
		Lexer::Next();


		VERIFY
			if (has_name==false||Lexer::Check(Id)) {
				if (has_name) {
					function->name = Lexer::string_val;
					Lexer::Match(Id);
					VERIFY
				}
			}
            else {
				Debugger::Alert(L"Need function name");
            }
		Lexer::Match('(');
		VERIFY
			function->args = FuncParam::Parse();
		Lexer::Match(')');
		VERIFY

			function->return_type = L'\0';
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
	void FunctionDecl::SetInternal(llvm::StructType* type) {
		self_type = type;
	}

	void FunctionDecl::GenHeader() {
		std::vector<llvm::Type*> types;
		if (self_type != nullptr) {
			types.push_back(self_type->getPointerTo());
			args->names.insert(args->names.begin(), L"this");
		}
		for (auto i = 0; i < args->size; i++)
			types.push_back(CodeGen::GetType(args->types[i]));

       
        const auto self_name =self_type == nullptr?"":self_type->getStructName().str();
		if (name == L"::init")
			full_name = self_name + "::" + self_name;
		else if (name == L"::delete")
			full_name = self_name + "::~" + self_name;
		else full_name = self_name + "::" + CodeGen::MangleStr(name);

        full_name+= "(";
		for (int i = self_type == nullptr ? 0 : 1,types_size = types.size(); i < types_size; i++)
			full_name += CodeGen::GetTypeStructName(types[i]) + (i == types.size() - 1 ? "" : ", ");
		full_name += ")";

		
		auto the_function = CodeGen::the_module->getFunction(full_name);
		if (!the_function) {

			const auto func_type = llvm::FunctionType::get(CodeGen::GetType(return_type), types, args->isVarArg);
			the_function = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, full_name,
				CodeGen::the_module.get());
			unsigned idx = 0;
			for (auto& arg : the_function->args())
				arg.setName(CodeGen::MangleStr(args->names[idx++]));
		}
		else {
			// *Debugger::out << "function " << std::wstring(full_name.begin(), full_name.end()) << " already defined\n";
			CodeGen::LogErrorV((std::string("function ") + full_name + std::string(" already defined\n")).c_str());
		}
	}

   

	void FunctionDecl::Gen() {
      
		if (is_extern)return;
		auto function = CodeGen::the_module->getFunction(full_name);
		if (!function) {
			*Debugger::out << "function head not found\n";
			return;
		}
		const auto bb = llvm::BasicBlock::Create(CodeGen::the_context, CodeGen::MangleStr(name) + "_entry", function);
		CodeGen::builder.SetInsertPoint(bb);
		

		// CodeGen::This = nullptr;
		for (auto& arg : function->args()) {
			
			auto type_name=CodeGen::GetTypeStructName(arg.getType());
			

			CodeGen::local_fields_table[arg.getName()] = &arg;
		}
	
		 if (self_type != nullptr) {
		 	auto self_decl = CodeGen::types_table[self_type->getStructName()];
		 	if (!self_decl->base_type_name.empty()) {
		
		 		const auto alloca = CodeGen::CreateEntryBlockAlloca(function, CodeGen::GetType(self_decl->base_type_name), "base");
		 		alloca->setAlignment(llvm::MaybeAlign(8));
		 		auto base = CodeGen::FindMemberField(function->getArg(0), L"base");
		 		CodeGen::AlignStore(CodeGen::builder.CreateStore(base, alloca)
		 		);
		 		CodeGen::local_fields_table["base"] = alloca;
		 	}
		 }

      


		if (statements != nullptr)statements->Gen();
		if (function->getReturnType()->getTypeID() == llvm::Type::VoidTyID)CodeGen::builder.CreateRetVoid();
		CodeGen::local_fields_table.clear();
		verifyFunction(*function);

	}

}
