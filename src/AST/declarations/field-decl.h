#ifndef FIELD_DECL_H
#define FIELD_DECL_H
#include "AST/statements/statement.h"
#include "AST/expressions/expr.h"

namespace AST {
	namespace decl {
		// class for matching variable declaration.
		class FieldDecl : public stmt::Statement {
			bool constant;
			std::string name;
			AST::Type type;
			std::shared_ptr<expr::Expr> value;
		public:
			void Gen(std::shared_ptr<DFContext>) override;
			static std::shared_ptr<FieldDecl> Parse(bool is_const);
		};
	}
}
#endif