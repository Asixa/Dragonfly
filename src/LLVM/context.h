
#ifndef DFContext_H
#define DFContext_H

#include "llvm/IR/IRBuilder.h"
#include <stack>
#include "frontend/symbol.h"

// forward declaration
namespace AST {
	namespace decl {
		class ClassDecl;
		class FunctionDecl;
	}
    namespace expr {
		class Field;
	}
	class Type;
	class CustomType;
	class Program;
	
}


class DFContext {
public:

	static std::vector<std::shared_ptr<DFContext>> contexts;

	llvm::LLVMContext context;
	std::unique_ptr<llvm::Module> module;
	std::unique_ptr<llvm::IRBuilder<>> builder;
	std::unique_ptr<llvm::DataLayout> data_layout;

	std::shared_ptr<AST::Program> program;

	// std::stack<std::map<std::string,AST::Type>>symbols;

	std::shared_ptr<frontend::Symbol> ast;
	std::shared_ptr<frontend::LLVMSymbol> llvm;

	// std::map<std::string, llvm::Value*> local_fields_table;
	// std::map<std::string, llvm::Value*> global_fields_table;
	std::map<std::string, std::string> func_alias_table;

	std::map<std::string, AST::decl::ClassDecl*> template_types_table;
	std::map<std::string, AST::decl::FunctionDecl*> template_function_table;

	std::map<std::string, std::shared_ptr<AST::CustomType>> types_table;
	std::map<std::string, std::shared_ptr<AST::decl::FunctionDecl>> functions_table;


	llvm::Value* True;
	llvm::Value* False;
	// static llvm::Value* Null;

	llvm::Type* void_ptr;
	llvm::Type* void_type;
	llvm::Type* int32;

	// True is in subblock like for or while
	bool is_sub_block;
	llvm::BasicBlock* block_begin;
	llvm::BasicBlock* block_end;
	AST::decl::FunctionDecl* current_function;

	bool ExistClass(std::string){return false;}


    explicit DFContext(std::shared_ptr<AST::Program> program);
	static void Gen();
	static void Analysis();

	static std::shared_ptr<DFContext> Create(std::shared_ptr<AST::Program> program);

	AST::decl::ClassDecl* GetTemplateClass(std::string name) { return nullptr; }
	AST::decl::FunctionDecl* GetTemplateFunc(std::string name) { return nullptr; }

	llvm::GlobalVariable* CreateGlob(const std::string name, llvm::Type* ty) { return nullptr; }
	llvm::ConstantInt* CreateConstant(int value) { return nullptr; }

	llvm::Function* CreateMainFunc() { return nullptr; }

	llvm::BasicBlock* CreateBasicBlock(llvm::Function* func, const std::string name){return nullptr;}

	llvm::AllocaInst* CreateEntryBlockAlloca(llvm::Type* type, const std::string& var_name, llvm::Function* the_function = nullptr) { return nullptr; }



	// llvm::Type* GetType(std::shared_ptr<AST::Type> type);

	llvm::StoreInst* AlignStore(llvm::StoreInst* a) { return nullptr; };
	llvm::LoadInst* AlignLoad(llvm::LoadInst* a) { return nullptr; };

	int GetPtrDepth(llvm::Value* value);
	int GetPtrDepth(llvm::Type* type);

	std::string GetStructName(llvm::Value* value) { return ""; };
	std::string GetStructName(llvm::Type* type) { return ""; };

	llvm::Function* GetFunction(std::string name);

	/**
	 * \brief Get the ir code for this value.
	 * \param value The value want to debug.
	 * \return ir code in string
	 */
	std::string DebugValue(llvm::Value* value);

	/**
	 * \brief  Take a type, and return a value that created on heap,
	 * \param type Type should not be pointer, use the_module->getTypeByName instead of CodeGen::GetType
	 * \return Value of the given type that created on heap
	 */
	llvm::Value* Malloc(llvm::Type* type, bool cast = true);
	void Free(llvm::Value* value);



	/**
	 * \brief Find the field in current scope.
	 * \param name The name of the field want to find.
	 * \param cmd The wanted type.  'kConstantWanted' by default,
	 *  should be 'kPtrWanted'  when it is LHS of assignment operation.
	 * \param warn throw a error in not found.
	 * \return The value of the field. in wanted type
	 */
	llvm::Value* FindField(const std::string name, int cmd = 0, bool warn = true){return nullptr;}


	// // true if the name is a custom type.
	// bool IsCustomType(std::string name);

	/**
	 * \brief find the category of a type
	 * \param ty type of the custom type
	 * \return  kClass, kStruct, kInterface or -1 if the type is not a custom type.
	 */
	// int GetCustomTypeCategory(llvm::Type* ty);
	//
	// /**
	// * \brief find the category of a type
	// * \param ty name of the custom type
	// * \return  kClass, kStruct, kInterface or -1 if the type is not a custom type.
	// */
	// int GetCustomTypeCategory(std::string ty);

	void BuildInFunc(const char* name, llvm::Type* ret, std::vector<llvm::Type*> types, bool isVarArg = false);
	// Write human-readable ir to file , for debug and testing.
	void WriteReadableIr(llvm::Module* module, const char* file, bool print = false);
	// Write compilable ir to file , for further compilation.
	void WriteBitCodeIr(llvm::Module* module, const char* file);

	virtual void Write();
};

#endif
