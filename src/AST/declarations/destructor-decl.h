#ifndef DESTRUCTOR_DECL_H
#define DESTRUCTOR_DECL_H
#include "declaration.h"
#include "function-decl.h"

namespace AST::decl {
		/**
		 * \brief <span style="color:red">**TODO**</span>\n
		 * Class for matching destructor function.\n
		 * ``delete(){...}``\n
		 * ``delete()=>STATEMENT``
		 */
		class DestructorDecl final :public Declaration {
			std::shared_ptr<FuncParam> args;
			std::shared_ptr<Statement> statements;
			void Gen(std::shared_ptr<DFContext> context) override;
			void GenHeader(std::shared_ptr<DFContext> context) override;
			static std::shared_ptr<DestructorDecl> Parse();
		};
}
#endif
