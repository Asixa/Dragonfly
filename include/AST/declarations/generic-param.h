#ifndef GENERIC_PARAM_H
#define GENERIC_PARAM_H
#include "lexer.h"
namespace parser {
	class GenericParam {
	public:
		int size;
		std::vector<std::string> names;
		std::string ToString();
	    static std::shared_ptr<GenericParam> Parse();
        
	};
}

#endif
