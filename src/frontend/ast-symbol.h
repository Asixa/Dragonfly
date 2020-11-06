#ifndef SYMBOL_H
#define SYMBOL_H
#include <memory>
#include <map>
#include <string>
#include <llvm/IR/Value.h>

#include "llvm-symbol.h"
//foward decal
namespace AST
{
	namespace decl {
		class ClassDecl;
		class FunctionDecl;
	}

	class Type;
	class  CustomType;
}
class DFContext;


namespace frontend {
	class ASTSymbol {
		std::shared_ptr<DFContext> ctx;
		std::vector<std::map<std::string, std::shared_ptr<AST::Type>>> fields;

		std::map<std::string,std::string> func_alias;
		std::map<std::string, std::shared_ptr<AST::decl::ClassDecl>> class_templates;
		std::map<std::string, std::shared_ptr<AST::decl::FunctionDecl>> function_templates;
		std::map<std::string, std::shared_ptr<AST::CustomType>> types;
		std::map<std::string, std::shared_ptr<AST::decl::FunctionDecl>> functions;
		std::map<std::string, std::shared_ptr<AST::decl::FunctionDecl>> extern_functions;

		
	public:
		ASTSymbol(const std::shared_ptr<DFContext> ctx) :ctx(ctx) {}
		void CreateScope();
		void EndScope();
		void AddField(std::string, std::shared_ptr<AST::Type>);
		void AddExternFunc(std::string name, std::shared_ptr<AST::decl::FunctionDecl> decl);
		void AddClassTemplate(std::string name, std::shared_ptr<AST::decl::ClassDecl> decl);
		void AddFuncTemplate(std::string name, std::shared_ptr<AST::decl::FunctionDecl> decl);
		void AddAlias(std::string from, std::string to);
		void AddClass(std::string name, std::shared_ptr<AST::CustomType> ty);
		void AddFunction(std::string name, std::shared_ptr<AST::decl::FunctionDecl> decl);


		std::shared_ptr<AST::Type>                  GetField(std::string);
		std::shared_ptr<AST::Type>                  GetMemberField(std::shared_ptr<AST::Type>, std::string name);
		std::shared_ptr<AST::decl::ClassDecl>       GetClassTemplate(std::string name);
		std::shared_ptr<AST::decl::FunctionDecl>    GetFuncTemplate(std::string name);
		std::shared_ptr<AST::decl::FunctionDecl>    GetFunctionDecl(std::string name);
		std::shared_ptr<AST::CustomType>            GetClass(std::string name);
		std::shared_ptr<AST::decl::ClassDecl>       GetClassDecl(std::string name);
		int                                         GetCustomTypeCategory(const std::string ty);



		bool IsCustomType(std::string name);
        std::string Alias(std::string name);

	};


}
#endif
