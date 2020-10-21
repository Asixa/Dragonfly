#include "AST/statements/import-stmt.h"
namespace parser {
	std::shared_ptr<Import> Import::Parse() {
		Lexer::Next();
		auto instance = std::make_shared<Import>();
		return instance;
	}

	void Import::Gen(std::shared_ptr<DFContext> context) { }
}