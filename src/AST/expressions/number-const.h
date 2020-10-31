#ifndef NUMBER_CONST_H
#define NUMBER_CONST_H
#include "frontend/lexer.h"
#include "AST/expressions/expr.h"

namespace AST {
	namespace expr {
	// Expression node for literal numbers.
	class NumberConst final : public Expr {
	public:
		int type;
		double value;
		explicit NumberConst(const double d, const int t) : value(d) { type = t; }
		void ToString() override;
		std::shared_ptr<AST::Type> Analysis(std::shared_ptr<DFContext>) override;
		llvm::Value* Gen(std::shared_ptr<DFContext>, bool is_ptr) override;
	};
	}
}
#endif