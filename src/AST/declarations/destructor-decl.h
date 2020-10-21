#ifndef DESTRUCTOR_DECL_H
#define DESTRUCTOR_DECL_H
#include "declaration.h"
#include "function-decl.h"

namespace parser {
	class DestructorDecl final :public Declaration {
		std::shared_ptr<FuncParam> args;
		std::shared_ptr<Statement> statements;
		void Gen() override;
		void GenHeader() override;
		static std::shared_ptr<DestructorDecl> Parse();
	};
}
#endif