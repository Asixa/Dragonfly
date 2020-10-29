#ifndef SYMBOL_H
#define SYMBOL_H
#include <memory>
#include <map>
#include <string>
#include <llvm/IR/Value.h>


//foward decal
namespace AST
{
	namespace decl {
		class ClassDecl;
		class FunctionDecl;
	}

	class Type;
}
class DFContext;


namespace frontend {
	class Symbol {
		std::shared_ptr<DFContext> ctx;
		std::vector<std::map<std::string, AST::Type*>> fields;
		std::map<std::string, AST::decl::ClassDecl*> classes;
		std::map<std::string, AST::decl::FunctionDecl*> functions;

	public:
		Symbol(const std::shared_ptr<DFContext> ctx) :ctx(ctx) {}
		void CreateScope();
		void EndScope();
		void AddField(std::string, AST::Type*);
		AST::Type* GetField(std::string);
	};

	class LLVMSymbol {
		std::shared_ptr<DFContext> ctx;
		std::vector<std::map<std::string, llvm::Value*>> fields;
	public:
		LLVMSymbol(const std::shared_ptr<DFContext> ctx):ctx(ctx){}

		void CreateScope();
		void EndScope();
		void AddField(std::string, llvm::Value*);
		llvm::Value* GetField(std::string,int wanted);
		/**
         * \brief Accpet a ptr value, and get it field'value by name.
         * if you only have a value but not ptr. store it to an alloca.
         * \param obj the object member.
         * \param name the name of its field
         * \return The value of the field, in...
         */
		llvm::Value* GetMemberField(llvm::Value* obj,std::string name);
		// std::string GetStructName(llvm::Type* type);
	};
}
#endif
