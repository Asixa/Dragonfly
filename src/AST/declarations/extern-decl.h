#ifndef EXTERN_EXPR_H
#define EXTERN_EXPR_H
#include "frontend/lexer.h"
#include "function-decl.h"

namespace AST {
    class Extern final : public Declaration {
    public:
		int type=0;
		std::string name;
		std::shared_ptr<Name> alias;
		std::shared_ptr<FuncParam> args;
		Type return_type;
		bool init;
		static std::shared_ptr<Extern>Parse();
        void GenHeader(std::shared_ptr<DFContext>) override;
		void Gen(std::shared_ptr<DFContext>) override;
    };
}
#endif
