#ifndef GENERIC_PARAM_H
#define GENERIC_PARAM_H
#include "frontend/lexer.h"
namespace AST {
	namespace decl {
		/**
		 * \brief class for matching generic parameters' declaration ; \n
		 *  <NAME,NAME...>\n
		 */
		class GenericParam {
		public:
			int size;
			std::vector<std::string> typenames;
			std::string ToString();
			static std::shared_ptr<GenericParam> Parse();

		};
	}
}

#endif
