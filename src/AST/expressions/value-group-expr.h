


#include "expr.h"

namespace AST {
	namespace expr {
		class ValueGroup final : public Expr {
		public:
            enum Type{Tuple,Array};

			std::shared_ptr<AST::Type> Analysis(std::shared_ptr<DFContext>) override;
			llvm::Value* Gen(std::shared_ptr<DFContext>, bool is_ptr) override;
			static std::shared_ptr<ValueGroup> Parse();
		};
	}
}
