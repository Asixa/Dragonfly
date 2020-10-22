#ifndef DECLARATION_H
#define DECLARATION_H
#include "AST/statements/statement.h"

namespace AST {
	namespace decl {
		/**
		 * \brief Base Class for declaration.\n
		 * All declarations will be generated before Statements
		 */
		class Declaration : public stmt::Statement {
		public:
			std::shared_ptr<Declaration>parent; ///< [untested] parent of this declaration. nullptr if there is no parent.

            /**
			 * \brief Generate the IR code.\n
			 * will always be called before ``Gen()`` \n
			 * This function is used to generate the declaration header, NOT the body.
			 * \param ctx the DFContext object that requried to generate IR.
			 */
			virtual void GenHeader(std::shared_ptr<DFContext> ctx) = 0;
		};
	}
}
#endif