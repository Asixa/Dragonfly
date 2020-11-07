
#ifndef LLVM_SYMBOL_H
#define LLVM_SYMBOL_H
#include <memory>
#include <map>
#include <string>
#include <llvm/IR/Value.h>
#include <llvm/IR/Instructions.h>
class DFContext;
namespace frontend {
	class Constant {
		
	public:
		std::shared_ptr<DFContext> ctx;
		llvm::Type* int32;
		llvm::Type* int8;
		llvm::Type* double_type;
		llvm::Type* float_type;
		llvm::Type * void_type;
        llvm::Type * size_t;
        llvm::Type * float_ptr;
        llvm::Type * char_ptr;
        llvm::Type * int32_ptr;
        llvm::Type * void_ptr;
        llvm::Type * unsigned_int;
        llvm::Type * void_ptr_ptr;
		llvm::Type* null;

		llvm::Value* True;
		llvm::Value* False;
		llvm::Value* zero;

		llvm::Value* Get(int v);
		void Init(std::shared_ptr<DFContext> ctx);
    };

	class LLVMSymbol {
		std::shared_ptr<DFContext> ctx;
		std::vector<std::map<std::string, llvm::Value*>> fields;

	public:
		LLVMSymbol(const std::shared_ptr<DFContext> ctx) :ctx(ctx) {}
		void CreateScope();
		void EndScope();
		void AddField(std::string, llvm::Value*);
		llvm::Value* GetField(std::string, bool is_ptr = false);
		/**
		 * \brief Accpet a ptr value, and get it field'value by name.
		 * if you only have a value but not ptr. store it to an alloca.
		 * \param obj the object member.
		 * \param name the name of its field
		 * \return The value of the field, in...
		 */
		llvm::Value* GetMemberField(llvm::Value* obj, std::string name);
		// std::string GetStructName(llvm::Type* type);
		int GetCustomTypeCategory(llvm::Type* ty);

		std::string GetStructName(llvm::Value* value);
		std::string GetStructName(llvm::Type* type);

		/**
		 * \brief  Take a type, and return a value that created on heap,
		 * \param type Type should not be pointer, use the_module->getTypeByName instead of CodeGen::GetType
		 * \return Value of the given type that created on heap
		 */
		llvm::Value* Malloc(llvm::Type* type, bool cast = true);
		void Free(llvm::Value* value);



		llvm::GlobalVariable* CreateGlobal(const std::string name, llvm::Type* ty);
		// llvm::ConstantInt* CreateConstant(int value);

		llvm::Function* CreateMainFunc();
		llvm::BasicBlock* CreateBasicBlock(llvm::Function* func, const std::string name);
		llvm::AllocaInst* CreateEntryBlockAlloca(llvm::Type* type, const std::string& var_name, llvm::Function* the_function = nullptr);



		// llvm::Type* GetType(std::shared_ptr<AST::Type> type);

		llvm::StoreInst* AlignStore(llvm::StoreInst* a);
		llvm::LoadInst* AlignLoad(llvm::LoadInst* a);

		int GetPtrDepth(llvm::Value* value);
		int GetPtrDepth(llvm::Type* type);



		llvm::Function* GetFunction(std::string name);

		
		/**
		 * \brief Get the ir code for this value.
		 * \param value The value want to debug.
		 * \return ir code in string
		 */
		std::string DebugValue(llvm::Value* value);
	};
}
#endif
