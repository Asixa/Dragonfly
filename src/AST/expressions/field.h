#ifndef FIELD_H
#define FIELD_H
#include "AST/expressions/expr.h"

namespace AST {
	namespace expr {
		// Expression node for variable or fields.
		class Field : public Expr {

		public:
			// enum CMD{ kConstantWanted = 0, kPtrWanted = 1 };
			// int cmd = kConstantWanted;
			bool is_ptr;
			std::shared_ptr<Field> child, left;
			void ToString() override;
			llvm::Value* Gen(std::shared_ptr<DFContext>, bool is_ptr) override;
			std::string name;
			// std::vector<std::wstring> names;
			// explicit Field(std::vector<std::wstring> d) : names(d) {}
			explicit Field(std::string d) : name(d) {}

			// eg. "a.b"  a is b's parent.
			std::shared_ptr<AST::Type> Analysis(std::shared_ptr<DFContext>) override;
			virtual std::shared_ptr<AST::Type> AnalysisField(std::shared_ptr<DFContext>, std::shared_ptr<AST::Type> parent);
			virtual llvm::Value* GenField(std::shared_ptr<DFContext>, llvm::Value* parent);
			static std::shared_ptr<Field>Parse();
			static std::shared_ptr<Field>ParsePostfix();

		};
	}
}
#endif