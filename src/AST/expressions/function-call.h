#ifndef FUNCTION_CALL_H
#define FUNCTION_CALL_H
#include "AST/expressions/field.h"
#include "frontend/lexer.h"
#include "AST/declarations/field-list.h"

namespace AST {
	namespace expr {
		// Expression node for function calls.
		class FuncCall final : public Field {
		public:

			void ToString() override;
			llvm::Value* Gen(std::shared_ptr<DFContext>, const int cmd = 0) override;
			std::shared_ptr<decl::FieldList> generic;
			std::vector<std::shared_ptr<Expr>> args;
			std::shared_ptr<AST::Type> Analysis(std::shared_ptr<DFContext>) override;
			llvm::Value* GenField(std::shared_ptr<DFContext>, llvm::Value* parent) override;
			explicit FuncCall(std::string d) : Field(d) {}
			static std::shared_ptr<FuncCall> Parse(std::string f);
		};
	}
}
#endif