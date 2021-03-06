#ifndef BOOLEAN_H
#define BOOLEAN_H
#include "AST/expressions/expr.h"

namespace AST {
	namespace expr {
		// Expression node for literal booleans.
		class Boolean final : public Expr {
		public:
			void ToString() override;
			std::shared_ptr<AST::Type> Analysis(std::shared_ptr<DFContext>) override;
			llvm::Value* Gen(std::shared_ptr<DFContext>, bool is_ptr) override;
			bool value;
			explicit Boolean(const bool d) : value(d) {}
		};
	}
}
#endif