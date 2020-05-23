#ifndef EXTENSION_H
#define EXTENSION_H
#include "AST/declarations/declaration.h"
#include "lexer.h"
#include "AST/declarations/function-decl.h"

namespace parser {
	class Extension final :public Declaration {
	public:
		std::string name;
		std::vector<std::shared_ptr<FunctionDecl>> functions;
		static std::shared_ptr<Extension> Parse();
		void Gen() override;
		void GenHeader() override;
	};
}
#endif