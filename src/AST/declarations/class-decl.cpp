#include "AST/declarations/class-decl.h"
#include "codegen.h"

namespace parser {
	std::shared_ptr<ClassDecl> ClassDecl::Parse(int ty) {
		auto instance = std::make_shared<ClassDecl>();
		instance->category = ty;
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
				
					
				case K_delete: {
					auto func = FunctionDecl::Parse();
					if (instance->destructor == nullptr)instance->destructor = func;
					else Debugger::Alert(L"Mutiple destructors is not allowed");
					instance->functions.push_back(func);
					VERIFY
					break;
				}
				case K_init:
					// instance->constructor.push_back(FunctionDecl::Parse());
					// break;
				case K_func:
				case K_dfunc:
				case K_kernal:
					instance->functions.push_back(FunctionDecl::Parse());
					VERIFY
						break;
				default: 
					if (instance->category==kInterface)break;
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
		auto mangled_name = CodeGen::MangleStr(name);
		auto the_struct = CodeGen::the_module->getTypeByName(mangled_name);
		std::vector<llvm::Type*> field_tys;

		if (!interfaces.empty()) {
			llvm::Type* baseType = nullptr;
			for (const auto& interface : interfaces) {
				auto mangled_name = CodeGen::MangleStr(interface);
				if (CodeGen::IsCustomType(mangled_name)) {
					const auto decl = CodeGen::types_table[mangled_name];
					if (!decl->category==kInterface) {
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

		for (const auto& type : types)field_tys.push_back(CodeGen::GetTypeByName(type));
		the_struct->setBody(field_tys);
		CodeGen::types_table[the_struct->getName().str()] = this;

		// //Create FAKE Constructor function 
		// auto func_type = llvm::FunctionType::get(the_struct->getPointerTo(), std::vector<llvm::Type*>(), false);
		// auto function = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, mangled_name +"()",
		// 	CodeGen::the_module.get());
		// auto bb = llvm::BasicBlock::Create(CodeGen::the_context, mangled_name + "_entry", function);
		// function->setCallingConv(llvm::CallingConv::C);
		// CodeGen::builder.SetInsertPoint(bb);
		// CodeGen::builder.CreateRet(CodeGen::Malloc(the_struct));
		// verifyFunction(*function);
		//
		// //Create REALL Constructor function 
		// func_type = llvm::FunctionType::get(llvm::Type::getVoidTy(CodeGen::the_context), std::vector<llvm::Type*>{the_struct->getPointerTo()}, false);
		// function = llvm::Function::Create(func_type, 
		// 	llvm::Function::ExternalLinkage, mangled_name+"::"+mangled_name + "(" +")",
		// 	CodeGen::the_module.get());
		// bb = llvm::BasicBlock::Create(CodeGen::the_context, mangled_name + "_entry", function);
		// function->setCallingConv(llvm::CallingConv::C);
		// CodeGen::builder.SetInsertPoint(bb);

  //       //test init
		// if (types.size() == 2) {
		//
		// 	auto a=CodeGen::builder.CreateStructGEP(function->getArg(0), 1);
  //           const auto v = llvm::ConstantInt::get(llvm::Type::getInt32Ty(CodeGen::the_context), static_cast<int>(233));
		// 	CodeGen::builder.CreateStore(v,a);
		// }

		// CodeGen::builder.CreateRetVoid();
  //
  //
		// // auto field1= CodeGen::builder.CreateStructGEP(the_struct, alloca, 1);
		// // CodeGen::builder.CreateStore(llvm::ConstantInt::get(Type::getInt32Ty(CodeGen::the_context), 233),field1);
  //       llvm::verifyFunction(*function);

		for (auto& function : functions) {
			function->SetInternal(the_struct);
			function->GenHeader();
			function->Gen();
		}
	}
}
