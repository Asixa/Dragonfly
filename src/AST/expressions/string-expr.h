#ifndef MY_STRING_H
#define MY_STRING_H
#include "AST/expressions/expr.h"

namespace AST {
	namespace expr {
		// Expression node for literal strings.
		class String final : public Expr {
		public:
			void ToString() override;
			llvm::Value* Gen(std::shared_ptr<DFContext>, const int cmd = 0) override;
			std::wstring value;
			std::shared_ptr<AST::Type> Analysis(std::shared_ptr<DFContext>) override;
			String(std::wstring d) : value(std::move(d)) {}
		};
	}
}
#endif