#ifndef CONTINUE_STMT_H
#define CONTINUE_STMT_H
#include "AST/statements/statement.h"

namespace parser {

	class Continue final :public Statement {
	public:
		static std::shared_ptr<Continue> Parse();
		void Gen() override;
	};

}
#endif