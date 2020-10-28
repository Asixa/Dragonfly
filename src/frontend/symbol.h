#ifndef SYMBOL_H
#define SYMBOL_H
#include <memory>
#include <map>
#include <string>

//foward decal
namespace AST
{
	namespace decl {
		class ClassDecl;
		class FunctionDecl;
	}
}

namespace frontend {
	class Symbol {

		std::map<std::string, AST::decl::ClassDecl*> classes;
		std::map<std::string, AST::decl::FunctionDecl*> functions;

	public:
		Symbol();
		Symbol(std::shared_ptr<Symbol> from);
	};



	class LLVMSymbol {

	public:
		LLVMSymbol();
		LLVMSymbol(std::shared_ptr<LLVMSymbol> from);
	};
}
#endif
