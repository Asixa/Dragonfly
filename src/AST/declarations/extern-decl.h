#ifndef EXTERN_EXPR_H
#define EXTERN_EXPR_H
#include "frontend/lexer.h"
#include "function-decl.h"

namespace AST {
	namespace decl {
	     /**
		 * \brief Class for matching 'C' style extern functions.\n
		 * ``extern XXX(..){...}``\n
		 * ``extern XXX(..)=> STATEMENT``\n
		 */
		class Extern final : public Declaration {
		public:
			int type = 0;
			std::string name;
			std::shared_ptr<Name> alias;
			std::shared_ptr<FuncParam> args;
			std::shared_ptr<AST::Type> return_type;
			bool init;
			Extern() { return_type = std::make_shared<Type>(); }
			static std::shared_ptr<Extern>Parse();

			void AnalysisHeader(std::shared_ptr<DFContext>) override;
			void Analysis(std::shared_ptr<DFContext>) override;
			void GenHeader(std::shared_ptr<DFContext>) override;
			void Gen(std::shared_ptr<DFContext>) override;
			std::string GetName() override;
		};
	}
}
#endif
