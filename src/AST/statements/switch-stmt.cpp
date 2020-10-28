#include "AST/statements/switch-stmt.h"
namespace AST {
	using namespace stmt;
	std::shared_ptr<Switch> Switch::Parse() {
        frontend::Lexer::Next();
        frontend::Lexer::Match('(');
        frontend::Lexer::Match(')');
        frontend::Lexer::Match('{');

		while (true) {
			break;
		}

        frontend::Lexer::Match('}');
		return nullptr;
	}

    void Switch::Analysis(std::shared_ptr<DFContext>) {}
    void Switch::Gen(std::shared_ptr<DFContext>) {}
}
