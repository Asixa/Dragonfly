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
        enum {kInterface,kClass,kStruct};
		int category=kClass;
	
		bool is_template = false;
		std::string full_name;
		std::wstring name;
		
		std::wstring base_type_name;
		std::vector<std::wstring> fields;
		std::vector<std::wstring> types;
		std::vector<std::wstring> interfaces;
		std::vector<std::shared_ptr<FunctionDecl>> functions;
		std::vector<std::shared_ptr<FunctionDecl>> constructor;
		std::shared_ptr<GenericParam> generic;
		std::shared_ptr<FunctionDecl> destructor;
		static std::shared_ptr<ClassDecl> Parse(int type = kClass);
		void Gen() override;
		void GenHeader() override;
		void Instantiate(std::shared_ptr<GenericParam> param);
	};
}
#endif