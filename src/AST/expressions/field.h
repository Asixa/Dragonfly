#ifndef FIELD_H
#define FIELD_H
#include "AST/expressions/expr.h"

namespace parser {
	// Expression node for variable or fields.
	class Field : public Expr {

	public:
		enum { kConstantWanted = 0, kPtrWanted = 1 };
		int cmd = kConstantWanted;
		std::shared_ptr<Field> child, left;
		void ToString() override;
		llvm::Value* Gen(const int cmd = 0) override;
		std::string name;
		// std::vector<std::wstring> names;
		// explicit Field(std::vector<std::wstring> d) : names(d) {}
		explicit Field(std::string d) : name(d) {}

        // eg. "a.b"  a is b's parent.
		virtual llvm::Value* GenField(llvm::Value* parent);
		static std::shared_ptr<Field>Parse();
		static std::shared_ptr<Field>ParsePostfix();

	};
}
#endif