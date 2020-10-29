#ifndef FIELD_DECL_H
#define FIELD_DECL_H
#include "AST/statements/statement.h"
#include "AST/expressions/expr.h"

namespace AST {
	namespace decl {
		/**
		 * \brief class for matching variable declaration.\n
		 * ``var NAME : TYPE = EXPR``\n
		 * ``var NAME = EXPR``\n
		 * ``let NAME : TYPE = EXPR`` <span style="color:red">**TODO**</span> \n
		 */
		class VariableDecl : public stmt::Statement {
			bool constant;
			std::string name;
			std::shared_ptr<AST::Type> type;
			std::shared_ptr<expr::Expr> value;
		public:
			VariableDecl() { type = std::make_shared<AST::Type>(); }
			void Analysis(std::shared_ptr<DFContext>) override;
			void Gen(std::shared_ptr<DFContext>) override;
			static std::shared_ptr<VariableDecl> Parse();

		
		};
	}
}
#endif