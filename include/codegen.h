// Copyright 2019 The Dragonfly Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef GEN_H
#define GEN_H
#include "llvm/ADT/APFloat.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

#include "parser.h"

class CodeGen {
public:
	static llvm::LLVMContext the_context;
	static std::unique_ptr<llvm::Module> the_module;
	static llvm::IRBuilder<> builder;
	static llvm::DataLayout data_layout;
	static std::shared_ptr<parser::Program> program;
	// static llvm::Function* the_function;
	static std::map<std::string, llvm::Value*> local_fields_table;
	static std::map<std::string, llvm::Value*> global_fields_table;

	static std::map<std::string, parser::ClassDecl*> template_types_table;
	static std::map<std::string, parser::FunctionDecl*> template_function_table;

	static std::map<std::string, parser::ClassDecl*> types_table;

	static llvm::Value* True;
	static llvm::Value* False;

	static llvm::Type* void_ptr;
	static llvm::Type* void_type;
	static llvm::Type* int32;
     
    // True is in subblock like for or while
	static bool is_sub_block;
    static llvm::BasicBlock* block_begin;
	static llvm::BasicBlock* block_end;

	static parser::FunctionDecl* current_function;


	static parser::ClassDecl* GetTemplateClass(std::string name);
	static parser::FunctionDecl* GetTemplateFunc(std::string name);

    static llvm::GlobalVariable* CreateGlob(const std::string name, llvm::Type* ty);
	static llvm::ConstantInt* CreateConstant(int value);

	static llvm::Function* CreateMainFunc();

    static llvm::BasicBlock* CreateBasicBlock(llvm::Function* func, const std::string name);

    static llvm::AllocaInst* CreateEntryBlockAlloca(llvm::Type* type,const std::string& var_name, llvm::Function* the_function = nullptr);



	static llvm::Type* GetType(parser::Type type);

    static llvm::StoreInst* AlignStore(llvm::StoreInst* a);
    static llvm::LoadInst* AlignLoad(llvm::LoadInst* a);

	static int GetPtrDepth(llvm::Value* value);
	static int GetPtrDepth(llvm::Type* type);

	static std::string GetStructName(llvm::Value* value);
	static std::string GetStructName(llvm::Type* type);


    /**
	 * \brief Get the ir code for this value.
	 * \param value The value want to debug.
	 * \return ir code in string
	 */
	static std::string DebugValue(llvm::Value* value);

    /**
	 * \brief  Take a type, and return a value that created on heap,
	 * \param type Type should not be pointer, use the_module->getTypeByName instead of CodeGen::GetType
	 * \return Value of the given type that created on heap
	 */
	static llvm::Value* Malloc(llvm::Type* type,bool cast=true);
	static void Free(llvm::Value* value);
    /**
	 * \brief Accpet a ptr value, and get it field'value by name.
	 * if you only have a value but not ptr. store it to an alloca.
	 * \param obj the object member.
	 * \param name the name of its field
	 * \return The value of the field, in...
	 */
	static llvm::Value* CodeGen::FindMemberField(llvm::Value* obj, const std::string name);

    /**
	 * \brief Find the field in current scope.
	 * \param name The name of the field want to find.
	 * \param cmd The wanted type.  'kConstantWanted' by default,
	 *  should be 'kPtrWanted'  when it is LHS of assignment operation.
	 * \param warn throw a error in not found.
	 * \return The value of the field. in wanted type
	 */
	static llvm::Value* CodeGen::FindField(const std::string name, int cmd = parser::Field::kConstantWanted, bool warn = true);


    // true if the name is a custom type.
	static bool IsCustomType(std::string name);

    /**
	 * \brief find the category of a type 
	 * \param ty type of the custom type
	 * \return  kClass, kStruct, kInterface or -1 if the type is not a custom type.
	 */
	static int GetCustomTypeCategory(llvm::Type* ty);

     /**
	 * \brief find the category of a type 
	 * \param ty name of the custom type
	 * \return  kClass, kStruct, kInterface or -1 if the type is not a custom type.
	 */
	static int GetCustomTypeCategory(std::string ty);

    static void BuildInFunc(const char* name, llvm::Type* ret, std::vector<llvm::Type*> types, bool isVarArg = false);
	// Write human-readable ir to file , for debug and testing.
	static void WriteReadableIr(llvm::Module* module, const char* file, bool print = false);
	// Write compilable ir to file , for further compilation.
	static void WriteBitCodeIr(llvm::Module* module, const char* file);

};


#endif
