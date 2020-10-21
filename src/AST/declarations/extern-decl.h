#ifndef EXTERN_EXPR_H
#define EXTERN_EXPR_H
#include "lexer.h"
#include "function-decl.h"

namespace parser {
    class Extern final : public Declaration {
    public:
		int type=0;
		std::string name;
		std::shared_ptr<Name> alias;
		std::shared_ptr<FuncParam> args;
		Type return_type;
		bool init;
		static std::shared_ptr<Extern>Parse();
        void GenHeader() override;
		void Gen() override;
    };
}
#endif
