#ifndef EXTENSION_H
#define EXTENSION_H
#include "AST/declarations/declaration.h"
#include "frontend/lexer.h"
#include "AST/declarations/function-decl.h"
 
namespace AST {
	namespace decl {
		/**
		 * \brief <span style="color:red">**BUGGY**  keyword conflicts with extern</span>  \n
		 * Class for matching extension functions.\n
		 * ``extension XXX{...}``\n
		 */
		class Extension final :public Declaration {
		public:
			std::string name;
			std::vector<std::shared_ptr<FunctionDecl>> functions;

			static std::shared_ptr<Extension> Parse();
			void AnalysisHeader(std::shared_ptr<DFContext>) override;
			void Analysis(std::shared_ptr<DFContext>) override;
			void Gen(std::shared_ptr<DFContext>) override;
			void GenHeader(std::shared_ptr<DFContext>) override;
		};
	}
}
#endif