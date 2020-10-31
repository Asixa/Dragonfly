#ifndef FUNCTION_CALL_H
#define FUNCTION_CALL_H
#include "AST/expressions/field.h"
#include "frontend/lexer.h"
#include "AST/declarations/field-list.h"

namespace AST {
	namespace expr {
		// Expression node for function calls.
		class FuncCall final : public Field {
			std::string callee_name;
			std::string param_name;
			bool is_constructor=false;
			bool is_member_func=false;
			std::shared_ptr<AST::CustomType>class_type;
			std::shared_ptr<AST::decl::FunctionDecl>func;
		public:

			void ToString() override;
			llvm::Value* Gen(std::shared_ptr<DFContext>, bool is_ptr) override;
			std::shared_ptr<decl::FieldList> generic;
			std::vector<std::shared_ptr<Expr>> args;
			std::shared_ptr<AST::Type> Analysis(std::shared_ptr<DFContext>) override;
            std::shared_ptr<AST::Type> AnalysisField(std::shared_ptr<DFContext>, std::shared_ptr<AST::Type> parent) override;
			llvm::Value* GenField(std::shared_ptr<DFContext>, llvm::Value* _this) override;
			explicit FuncCall(std::string d) : Field(d) {}
			static std::shared_ptr<FuncCall> Parse(std::string f);
		};
	}
}
#endif