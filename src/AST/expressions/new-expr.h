#ifndef NEW_H
#define NEW_H
#include "AST/expressions/expr.h"
#include "function-call.h"

namespace AST {
	namespace expr {
		// Expression node for Lambda expression.
		class New final : public Expr {
		public:
			std::shared_ptr<FuncCall>func;
			void ToString() override;
			std::shared_ptr<AST::Type> Analysis(std::shared_ptr<DFContext>) override;
			llvm::Value* Gen(std::shared_ptr<DFContext>, const int cmd = 0) override;
			static std::shared_ptr<New> Parse();
		};
	}
}
#endif