#ifndef ENUM_DECL_H
#define ENUM_DECL_H
#include "AST/declarations/declaration.h"

namespace parser {
	class EnumDecl final : public Declaration {
	public:
		bool anonymous = false;
        void GenHeader() override;
        void Gen() override;
		static std::shared_ptr<EnumDecl>Parse();
	};
}
#endif