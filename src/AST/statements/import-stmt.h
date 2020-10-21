#ifndef IMPORT_STMT_H
#define IMPORT_STMT_H
#include "AST/statements/statement.h"

namespace parser {
	class Import final :public Statement {
	public:
		static std::shared_ptr<Import> Parse();
		void Gen() override;
	};
}
#endif