#include "AST/statements/import-stmt.h"
namespace AST {
	using namespace stmt;
		std::shared_ptr<Import> Import::Parse() {
			frontend::Lexer::Next();
			auto instance = std::make_shared<Import>();
			return instance;
		}

		void Import::Gen(std::shared_ptr<DFContext> context) { }
	
}