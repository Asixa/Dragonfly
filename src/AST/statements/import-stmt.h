#ifndef IMPORT_STMT_H
#define IMPORT_STMT_H
#include "AST/statements/statement.h"

namespace AST {
	namespace stmt {
		class Import final :public Statement {
		public:
			static std::shared_ptr<Import> Parse();
			void Analysis(std::shared_ptr<DFContext>) override;
			void Gen(std::shared_ptr<DFContext>) override;
		};
	}
}
#endif