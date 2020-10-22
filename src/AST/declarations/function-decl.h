#ifndef FUNCTION_DECL_H
#define FUNCTION_DECL_H

#include "frontend/lexer.h"
#include "AST/declarations/declaration.h"
#include "generic-param.h"
#include "namespace-decl.h"

namespace AST {
	// class for matching function declaration parameters; 
	class FuncParam {
	public:
		int size = 0;
		bool is_var_arg = false;
		std::vector<std::string> names;
		std::vector<AST::Type> types;
		std::vector<int>generic_id;
		static std::shared_ptr<FuncParam> Parse();
		FuncParam() {
			size = 0;
		}
        FuncParam(const std::shared_ptr<FuncParam> copy) {
			*this = *copy;
			// for (auto i : copy->types)types.push_back(i);
		}
    };

	// class for matching function definition.
	class FunctionDecl final : public Declaration {
	public:
		bool differentiable = false,
			 kernal = false,
			 is_extern = false,
	         extension=false;
		int generic_return = -1;

		bool is_template = false;
		std::shared_ptr<Name> name;
		std::string func_postfix; 
		std::string header_name;
		std::string full_name;
		Type return_type;
		llvm::StructType* parent_type=nullptr;

		// std::vector<int>generic_arguments;
		std::shared_ptr<GenericParam> generic;
		std::shared_ptr<FuncParam> args;
		std::shared_ptr<Statement> statements;

		FunctionDecl() {}
        FunctionDecl(std::shared_ptr < FunctionDecl> copy);


		static std::shared_ptr<FunctionDecl> CreateInit(std::shared_ptr<FuncParam> param);
	
		void SetInternal(llvm::StructType* type);
		void Gen(std::shared_ptr<DFContext>) override;
		void GenHeader(std::shared_ptr<DFContext>) override;

		void PassGeneric(std::shared_ptr <GenericParam> val, std::shared_ptr<GenericParam> key=nullptr);
		void Instantiate(std::shared_ptr<DFContext> context,std::shared_ptr <GenericParam> param);
	
		static std::shared_ptr<FunctionDecl> Parse(bool ext = false);
	};
}
#endif