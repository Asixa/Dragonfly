
#ifndef DFContext_H
#define DFContext_H

#include "llvm/IR/IRBuilder.h"
#include <stack>
#include "frontend/ast-symbol.h"

// forward declaration
namespace AST {
	class Program;
}


class DFContext {
	// Write compilable ir to file , for further compilation.
	void WriteBitCodeIr(llvm::Module* module, const char* file);
	// Write human-readable ir to file , for debug and testing.
	static void WriteReadableIr(llvm::Module* module, const char* file, bool print = false);

public:

	static std::vector<std::shared_ptr<DFContext>> contexts;

	llvm::LLVMContext context;
	std::unique_ptr<llvm::Module> module;
	std::unique_ptr<llvm::IRBuilder<>> builder;
	std::unique_ptr<llvm::DataLayout> data_layout;

	std::shared_ptr<AST::Program> program;
	std::shared_ptr<frontend::ASTSymbol> ast;
	std::shared_ptr<frontend::LLVMSymbol> llvm;


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

	// bool ExistClass(std::string);


	
    explicit DFContext(std::shared_ptr<AST::Program> program);
	static void Gen();
	static void Analysis();

	
	static void BuildInFunc(std::shared_ptr<DFContext> ctx, std::string name, std::shared_ptr<AST::Type> ret, 
		const std::vector<std::shared_ptr<AST::Type>> args, const bool isVarArg = false);


	llvm::Function* BuildInFunc(const char* name, llvm::Type* ret, std::vector<llvm::Type*> types, bool isVarArg = false);
	llvm::StructType* BuildInType(const char* name);
	virtual void Write();
	virtual void BuiltIn(std::shared_ptr < DFContext> ptr)=0;
};

#endif
