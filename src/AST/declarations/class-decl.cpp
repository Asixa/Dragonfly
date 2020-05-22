#include "AST/declarations/class-decl.h"
#include "codegen.h"

namespace parser {
	std::shared_ptr<ClassDecl> ClassDecl::Parse(int ty) {
		auto instance = std::make_shared<ClassDecl>();
		instance->category = ty;
		Lexer::Next();
		instance->name = Lexer::string_val;
		Lexer::Match(Id);
		if (Lexer::Check('<'))
			instance->generic = GenericParam::Parse();
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
		Lexer::SkipNewlines();
		Lexer::Match('{');

		while (true) {
			Lexer::SkipNewlines();
			if (Lexer::token->type == '}')break;
			switch (Lexer::token->type) {
			case K_delete: {
				auto func = FunctionDecl::Parse();
				if (instance->destructor == nullptr)instance->destructor = func;
				else Debugger::Error(L"Mutiple destructors is not allowed");
				instance->functions.push_back(func);
				break;
			}
			case K_init:
				// instance->constructor.push_back(FunctionDecl::Parse());
				// break;
			case K_func:
			case K_dfunc:
			case K_kernal:
				instance->functions.push_back(FunctionDecl::Parse());
				break;
			default:
				if (instance->category == kInterface)break;
				instance->fields.push_back(Lexer::string_val);
				Lexer::Match(Id);
				Lexer::Match(':');
				if (Lexer::CheckType()) {
					std::wstring t;
					t += static_cast<wchar_t>(Lexer::token->type);
					instance->types.push_back(t);
					Lexer::Next();
				}
				else {
					instance->types.push_back(Lexer::string_val);
					Lexer::Match(Id);
				}
				Lexer::MatchSemicolon();
				Lexer::SkipNewlines();
			}
		}
		Lexer::Match('}');
		return instance;
	}

	void ClassDecl::GenHeader() {
		auto the_struct = CodeGen::the_module->getTypeByName(CodeGen::MangleStr(name));
		if (!the_struct) the_struct = llvm::StructType::create(CodeGen::the_context, CodeGen::MangleStr(name));
		else {
			*Debugger::out << "Type " << name << " already defined" << std::endl;
			return;
		}
		CodeGen::types_table[the_struct->getName().str()] = this;
		for (auto& function : functions) {
			function->SetInternal(the_struct);
			CodeGen::program->declarations.push_back(function);
		}
	}

	void ClassDecl::Gen() {
		const auto mangled_name = CodeGen::MangleStr(name);
		auto the_struct = CodeGen::the_module->getTypeByName(mangled_name);
		std::vector<llvm::Type*> field_tys;

		if (!interfaces.empty()) {
			llvm::Type* baseType = nullptr;
			for (const auto& interface : interfaces) {
				auto mangled_name = CodeGen::MangleStr(interface);
				if (CodeGen::IsCustomType(mangled_name)) {
					const auto decl = CodeGen::types_table[mangled_name];
					if (!decl->category == kInterface) {
						if (baseType == nullptr) {
							baseType = CodeGen::the_module->getTypeByName(CodeGen::MangleStr(interface));
							base_type_name = interface;
						}
						else Debugger::ErrorV("Inherit multiple classes is not allowed",line,ch);
					}
				}
				else  Debugger::ErrorV((std::string(mangled_name) + " is not defined").c_str(), line, ch);
			}
			if (baseType != nullptr) {

				fields.insert(fields.begin(), L"base");
				field_tys.insert(field_tys.begin(), baseType);
			}
		}

		for (const auto& type : types)field_tys.push_back(CodeGen::GetTypeByName(type));
		the_struct->setBody(field_tys);


		// a->setAttributes()
	}
}