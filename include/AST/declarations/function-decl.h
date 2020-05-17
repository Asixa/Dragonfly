#ifndef FUNCTION_DECL_H
#define FUNCTION_DECL_H

#include "lexer.h"
#include "AST/declarations/declaration.h"

namespace parser {
	// class for matching function declaration parameters; 
	class FuncParam {
	public:
		int size;
		bool isVarArg = false;
		std::vector<std::wstring> names, types;
		static std::shared_ptr<FuncParam> Parse();
	};

	// class for matching function definition.
	class FunctionDecl final : public Declaration {
	public:
		bool differentiable = false,
			kernal = false,
			is_extern = false;
		std::wstring name;
		std::string full_name;
		llvm::StructType* self_type;
		std::wstring return_type;
		std::shared_ptr<FuncParam> args;
		std::shared_ptr<Statement> statements;

		void SetInternal(llvm::StructType* type);

		void Gen() override;
		void GenHeader() override;
		static std::shared_ptr<FunctionDecl> Parse(bool ext = false);
	};
}
#endif