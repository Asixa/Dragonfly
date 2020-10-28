#ifndef CONTINUE_STMT_H
#define CONTINUE_STMT_H
#include "AST/statements/statement.h"

namespace AST {
	namespace stmt {
		class Continue final :public Statement {
		public:
			static std::shared_ptr<Continue> Parse();
			void Analysis(std::shared_ptr<DFContext>) override;
			void Gen(std::shared_ptr<DFContext>) override;
		};
	}
}
#endif