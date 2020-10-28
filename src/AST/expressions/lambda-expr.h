#ifndef LAMBDA_H
#define LAMBDA_H
#include "AST/expressions/expr.h"

namespace AST {
	namespace expr {
		// Expression node for Lambda expression.
		class Lambda final : public Expr {
		public:
			void ToString() override;
			std::shared_ptr<AST::Type> Analysis(std::shared_ptr<DFContext>) override;
			llvm::Value* Gen(std::shared_ptr<DFContext>, const int cmd = 0) override;
			static std::shared_ptr<Lambda> Parse() { return nullptr; }
		};
	}
}
#endif