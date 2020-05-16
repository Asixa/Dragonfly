#ifndef ENUM_DECL_H
#define ENUM_DECL_H
#include "AST/declarations/declaration.h"

namespace parser {
	class EnumDecl final : public Declaration {
		bool anonymous = false;

	};
}
#endif