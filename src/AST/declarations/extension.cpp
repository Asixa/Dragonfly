#include "AST/declarations/extension.h"
#include "codegen.h"

namespace parser {

	std::shared_ptr<Extension> Extension::Parse() {
		auto instance = std::make_shared<Extension>();
		Lexer::Next();
		VERIFY
			instance->name = Lexer::string_val;
		Lexer::Match(Id);

		VERIFY
			Lexer::SkipNewlines();
		VERIFY

			const auto brackets = Lexer::Check('{');


		if (brackets) Lexer::Next();
		else Lexer::Match(Arrow);


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
					VERIFY
				}
				if (!brackets)break;
			}
		if (brackets)Lexer::Match('}');
		VERIFY
			return instance;
	}


	void Extension::Gen() {
		const auto the_struct = CodeGen::the_module->getTypeByName(CodeGen::MangleStr(name));
		for (auto& function : functions) {
			function->SetInternal(name, the_struct);
			function->GenHeader();
			function->Gen();
		}
	}
	void Extension::GenHeader() {
		const auto the_struct = CodeGen::the_module->getTypeByName(CodeGen::MangleStr(name));
		if (!the_struct)* Debugger::out << "Type " << name << " is not defined" << std::endl;
	}

}
