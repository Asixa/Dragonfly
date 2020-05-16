#ifndef CLASS_DECL_H
#define CLASS_DECL_H
#include "declaration.h"
#include "AST/expressions/expr.h"
#include "lexer.h"
#include "function-decl.h"

namespace parser {
	// class for matching class declaration.
	class ClassDecl final : public Declaration {
	public:
		bool is_interface;
		std::wstring name;
		std::wstring base_type_name;
		std::vector<std::wstring> fields;
		std::vector<std::wstring> types;
		std::vector<std::wstring> interfaces;
		std::vector<std::shared_ptr<FunctionDecl>> functions;
		static std::shared_ptr<ClassDecl> Parse(bool interface = false);
		void Gen() override;
		void GenHeader() override;
	};
}
#endif