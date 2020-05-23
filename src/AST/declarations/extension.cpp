#include "AST/declarations/extension.h"
#include "codegen.h"

namespace parser {

	std::shared_ptr<Extension> Extension::Parse() {
		auto instance = std::make_shared<Extension>();
		Lexer::Next();
		instance->name = CodeGen::MangleStr(Lexer::string_val);
		Lexer::Match(Id);
		Lexer::SkipNewlines();
		const auto brackets = Lexer::Check('{');
		if (brackets) Lexer::Next();
		else Lexer::Match(Arrow);
		while (true) {
			Lexer::SkipNewlines();
			if (Lexer::token->type == '}')break;
			switch (Lexer::token->type) {
			case K_init:
			case K_delete:
			case K_func:
			case K_dfunc:
			case K_kernal:
				instance->functions.push_back(FunctionDecl::Parse());
				break;
			default:
				break;
			}
			if (!brackets)break;
		}
		if (brackets)Lexer::Match('}');
		return instance;
	}

	void Extension::Gen() {
		const auto the_struct = CodeGen::the_module->getTypeByName(name);
		for (auto& function : functions) {
			function->SetInternal(the_struct);
			function->GenHeader();
			function->Gen();
		}
	}
	void Extension::GenHeader() {
		const auto the_struct = CodeGen::the_module->getTypeByName(name);
		if (!the_struct)* Debugger::out << "Type " << name.c_str() << " is not defined" << std::endl;
	}

}
