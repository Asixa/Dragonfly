
#ifndef BREAK_STMT_H
#define BREAK_STMT_H

#include "AST/statements/statement.h"
#include "lexer.h"
namespace parser {
	class Break final :public Statement {
	public:
		static std::shared_ptr<Break> Parse();
		void Gen() override;
	};

}
#endif