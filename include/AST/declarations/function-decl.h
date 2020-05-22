#ifndef FUNCTION_DECL_H
#define FUNCTION_DECL_H

#include "lexer.h"
#include "AST/declarations/declaration.h"
#include "generic-param.h"

namespace parser {
	// class for matching function declaration parameters; 
	class FuncParam {
	public:
		int size;
		bool is_var_arg = false;
		std::vector<std::wstring> names, types;
		std::vector<int>generic_id;
		static std::shared_ptr<FuncParam> Parse();
	};

	// class for matching function definition.
	class FunctionDecl final : public Declaration {
	public:
		bool differentiable = false,
			 kernal = false,
			 is_extern = false;
		int generic_return = -1;

		std::wstring name;
		std::string header_name;
		std::string full_name;
		std::wstring return_type;
		llvm::StructType* parent_type;

		// std::vector<int>generic_arguments;
		std::shared_ptr<GenericParam> generic;
		std::shared_ptr<FuncParam> args;
		std::shared_ptr<Statement> statements;

		std::string GetInstanceName(GenericParam* g_param);
		bool IsGenericArgument(std::wstring name);
		void SetInternal(llvm::StructType* type);
		void Gen() override;
		void GenHeader() override;
	
		static std::shared_ptr<FunctionDecl> Parse(bool ext = false);
	};
}
#endif