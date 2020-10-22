#ifndef ENUM_DECL_H
#define ENUM_DECL_H
#include "AST/declarations/declaration.h"

namespace AST {
	namespace decl {
		/**
		 * \brief <span style="color:red">**TODO**</span>\n
		 * Class for matching enum declarations.\n
		 * ``enum XXX{...}``\n
		 */
		class EnumDecl final : public Declaration {
		public:
			bool anonymous = false;
			void GenHeader(std::shared_ptr<DFContext> context) override;
			void Gen(std::shared_ptr<DFContext> context) override;
			static std::shared_ptr<EnumDecl>Parse();
		};
	}
}
#endif