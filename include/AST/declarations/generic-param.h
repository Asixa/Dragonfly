#ifndef GENERIC_PARAM_H
#define GENERIC_PARAM_H
#include "lexer.h"
namespace parser {
	class GenericParam {
	public:
		int size;
		std::vector<std::wstring> names;
		static std::shared_ptr<GenericParam> Parse();
	};
}

#endif
