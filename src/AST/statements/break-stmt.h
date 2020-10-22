
#ifndef BREAK_STMT_H
#define BREAK_STMT_H

#include "AST/statements/statement.h"
#include "frontend/lexer.h"
namespace AST {
	namespace stmt {
		class Break final :public Statement {
		public:
			static std::shared_ptr<Break> Parse();
			void Gen(std::shared_ptr<DFContext>) override;
		};
	}

}
#endif