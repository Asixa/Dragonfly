#ifndef NEW_H
#define NEW_H
#include "AST/expressions/expr.h"
#include "function-call.h"

namespace parser {
	// Expression node for Lambda expression.
	class New final : public Expr {
	public:
		std::shared_ptr<FuncCall>func;
		void ToString() override;
		llvm::Value* Gen(const int cmd = 0) override;
		static std::shared_ptr<New> Parse();
	};
}
#endif