#ifndef CONSTRUCTOR_DECL_H
#define CONSTRUCTOR_DECL_H
#include "AST/declarations/declaration.h"
#include "frontend/lexer.h"
#include "AST/declarations/function-decl.h"

namespace AST {
	namespace decl
    {
        /**
		 * \brief class for matching constructor function.\n
		 * ``init(..){...}``\n
		 * ``init(..)=>STATEMENT``
		 */
		class ConstructorDecl final :public Declaration {
			std::shared_ptr<FuncParam> args;
			std::shared_ptr<Statement> statements;
			void Gen(std::shared_ptr<DFContext>) override;
			void GenHeader(std::shared_ptr<DFContext>) override;
			static std::shared_ptr<ConstructorDecl> Parse();
		};
	}
}
#endif