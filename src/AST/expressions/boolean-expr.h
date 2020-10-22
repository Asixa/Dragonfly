#ifndef BOOLEAN_H
#define BOOLEAN_H
#include "AST/expressions/expr.h"

namespace AST {
	// Expression node for literal booleans.
	class Boolean final : public Expr {
	public:
		void ToString() override;
		llvm::Value* Gen(std::shared_ptr<DFContext>,const int cmd = 0) override;
		bool value;
		explicit Boolean(const bool d) : value(d) {}
	};
}
#endif