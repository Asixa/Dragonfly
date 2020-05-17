#include "AST/declarations/class-decl.h"
#include "codegen.h"

namespace parser {
	std::shared_ptr<ClassDecl> ClassDecl::Parse(bool interface) {
		auto instance = std::make_shared<ClassDecl>();
		instance->is_interface = interface;
		Lexer::Next();
		VERIFY
			instance->name = Lexer::string_val;
		Lexer::Match(Id);

		if (Lexer::Check(':')) {
			Lexer::Next();
			instance->interfaces.push_back(Lexer::string_val);
			Lexer::Next();
			while (Lexer::Check(',')) {
				Lexer::Next();
				instance->interfaces.push_back(Lexer::string_val);
				Lexer::Match(Id);
			}
		}

		VERIFY
			Lexer::SkipNewlines();
		VERIFY
			Lexer::Match('{');
		VERIFY


			while (true) {
				Lexer::SkipNewlines();
				VERIFY
					if (Lexer::token->type == '}')break;
				switch (Lexer::token->type) {
				case K_init:
				case K_deinit:
				case K_func:
				case K_dfunc:
				case K_kernal:
					instance->functions.push_back(FunctionDecl::Parse());
					VERIFY
						break;
				default: 
					if (instance->is_interface)break;
					instance->fields.push_back(Lexer::string_val);
					Lexer::Match(Id);
					VERIFY
						Lexer::Match(':');

					if (Lexer::CheckType()) {
						std::wstring t;
						t += static_cast<wchar_t>(Lexer::token->type);
						instance->types.push_back(t);
						Lexer::Next();
						VERIFY
					}
					else {
						instance->types.push_back(Lexer::string_val);
						Lexer::Match(Id);
						VERIFY
					}

					Lexer::MatchSemicolon();
					Lexer::SkipNewlines();
					VERIFY
				}
			}
		Lexer::Match('}');
		VERIFY
			return instance;
	}

	void ClassDecl::GenHeader() {
		auto the_struct = CodeGen::the_module->getTypeByName(CodeGen::MangleStr(name));
		if (!the_struct) the_struct = llvm::StructType::create(CodeGen::the_context, CodeGen::MangleStr(name));
		else *Debugger::out << "Type " << name << " already defined" << std::endl;
	}

	void ClassDecl::Gen() {
		auto the_struct = CodeGen::the_module->getTypeByName(CodeGen::MangleStr(name));
		std::vector<llvm::Type*> field_tys;

		if (!interfaces.empty()) {
			llvm::Type* baseType = nullptr;
			for (const auto& interface : interfaces) {
				auto mangled_name = CodeGen::MangleStr(interface);
				if (CodeGen::types_table.find(mangled_name) != CodeGen::types_table.end()) {
					const auto decl = CodeGen::types_table[mangled_name];
					if (!decl->is_interface) {
						if (baseType == nullptr) {
							baseType = CodeGen::the_module->getTypeByName(CodeGen::MangleStr(interface));
							base_type_name = interface;
						}
						else CodeGen::LogErrorV("Inherit multiple classes is not allowed");
					}
				}
				else  CodeGen::LogErrorV((std::string(mangled_name) + " is not defined").c_str());
			}
			if (baseType != nullptr) {

				fields.insert(fields.begin(), L"base");
				field_tys.insert(field_tys.begin(), baseType);
			}
		}

		for (const auto& type : types)field_tys.push_back(CodeGen::GetType(type));
		the_struct->setBody(field_tys);
		CodeGen::types_table[the_struct->getName().str()] = this;

		//Create a Constructor function
		const auto func_type = llvm::FunctionType::get(the_struct->getPointerTo(), std::vector<llvm::Type*>(), false);
		auto function = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, CodeGen::MangleStr(name)+"()",
			CodeGen::the_module.get());
		const auto bb = llvm::BasicBlock::Create(CodeGen::the_context, CodeGen::MangleStr(name) + "_entry", function);
		function->setCallingConv(llvm::CallingConv::C);
		CodeGen::builder.SetInsertPoint(bb);
		//Constructor Body

		// const auto alloca = CreateEntryBlockAlloca(the_function, the_struct, "struct");
		// alloca->setAlignment(MaybeAlign(8));

		CodeGen::builder.CreateRet(CodeGen::Malloc(the_struct));
		// auto field1= CodeGen::builder.CreateStructGEP(the_struct, alloca, 1);
		// CodeGen::builder.CreateStore(llvm::ConstantInt::get(Type::getInt32Ty(CodeGen::the_context), 233),field1);
		verifyFunction(*function);

		for (auto& function : functions) {
			function->SetInternal(the_struct);
			function->GenHeader();
			function->Gen();
		}
	}
}
