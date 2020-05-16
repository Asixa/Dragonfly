#ifndef FIELD_DECL_H
#define FIELD_DECL_H
#include "AST/statements/statement.h"
#include "AST/expressions/expr.h"

namespace parser {
	// class for matching variable declaration.
	class FieldDecl : public Statement {
		bool constant;
		std::wstring name, type;
		std::shared_ptr<Expr> value;
	public:
		void Gen() override;
		static std::shared_ptr<FieldDecl> Parse(bool is_const);
	};
}
#endif