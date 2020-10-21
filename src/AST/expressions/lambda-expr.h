#ifndef LAMBDA_H
#define LAMBDA_H
#include "AST/expressions/expr.h"

namespace parser {
	// Expression node for Lambda expression.
	class Lambda final : public Expr {
	public:
		void ToString() override;
		llvm::Value* Gen(std::shared_ptr<DFContext>,const int cmd = 0) override;
		static std::shared_ptr<Lambda> Parse() { return nullptr; }
	};
}
#endif