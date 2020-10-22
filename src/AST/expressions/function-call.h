#ifndef FUNCTION_CALL_H
#define FUNCTION_CALL_H
#include "AST/expressions/field.h"
#include "frontend/lexer.h"
#include "AST/declarations/generic-param.h"

namespace AST {
	namespace expr {
		// Expression node for function calls.
		class FuncCall final : public Field {
		public:

			void ToString() override;
			llvm::Value* Gen(std::shared_ptr<DFContext>, const int cmd = 0) override;
			std::shared_ptr<decl::GenericParam> generic;
			std::vector<std::shared_ptr<Expr>> args;
			llvm::Value* GenField(std::shared_ptr<DFContext>, llvm::Value* parent) override;
			explicit FuncCall(std::string d) : Field(d) {}
			static std::shared_ptr<FuncCall> Parse(std::string f);
		};
	}
}
#endif