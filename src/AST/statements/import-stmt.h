#ifndef IMPORT_STMT_H
#define IMPORT_STMT_H
#include "AST/statements/statement.h"

namespace AST {
	class Import final :public Statement {
	public:
		static std::shared_ptr<Import> Parse();
		void Gen(std::shared_ptr<DFContext>) override;
	};
}
#endif