#ifndef GENERIC_PARAM_H
#define GENERIC_PARAM_H
#include "frontend/lexer.h"
namespace AST {
	class GenericParam {
	public:
		int size;
		std::vector<std::string> names;
		std::string ToString();
	    static std::shared_ptr<GenericParam> Parse();
        
	};
}

#endif
