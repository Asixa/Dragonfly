#ifndef FUNCTION_CALL_H
#define FUNCTION_CALL_H
#include "AST/expressions/field.h"
#include "lexer.h"

namespace parser {
	// Expression node for function calls.
	class FuncCall final : public Field {
	public:

		void ToString() override;
		llvm::Value* Gen(const int cmd = 0) override;
		// std::vector<std::wstring> names;
		std::vector<std::shared_ptr<Expr>> args;
		llvm::Value* GenField(llvm::Value* parent) override;
		explicit FuncCall(std::wstring d) : Field(d) {}
		static std::shared_ptr<FuncCall> Parse(std::wstring f);
	};

}
#endif