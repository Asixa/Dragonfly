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
	// static llvm::Function* the_function;
	static std::map<std::string, llvm::Value*> local_fields_table;
	static std::map<std::string, llvm::Value*> global_fields_table;
	static std::map<std::string, parser::ClassDecl*> types_table;

	static llvm::Value* True;
	static llvm::Value* False;

    // True is in subblock like for or while
	static bool is_sub_block;
    static llvm::BasicBlock* block_begin;
	static llvm::BasicBlock* block_end;

	// static llvm::Value* This;

    static llvm::Value* LogErrorV(const char* str);

    static llvm::GlobalVariable* CreateGlob(llvm::IRBuilder<>& builder, const std::string name, llvm::Type* ty);

    static llvm::Function* CreateFunc(llvm::IRBuilder<>& builder, const std::string name);

    static llvm::BasicBlock* CreateBb(llvm::Function* func, const std::string name);

    static llvm::AllocaInst* CreateEntryBlockAlloca(llvm::Function* the_function, llvm::Type* type,
                                                    const std::string& var_name);
    static std::string MangleStr(const std::wstring str);

    static llvm::Type* GetType(std::wstring type_name);

    static llvm::StoreInst* AlignStore(llvm::StoreInst* a);

    static llvm::LoadInst* AlignLoad(llvm::LoadInst* a);

	static int GetValuePtrDepth(llvm::Value* value);

    static std::string GetValueStructType(llvm::Value* value);

	static std::string GetValueDebugType(llvm::Value* value);


	static llvm::Value* Malloc(llvm::Type* type);
	static llvm::Value* CodeGen::GetMemberField(llvm::Value* obj, const std::wstring name);
	static llvm::Value* CodeGen::GetField(const std::wstring name, bool warn = true);

    static void BuildInFunc(const char* name, llvm::Type* ret, std::vector<llvm::Type*> types, bool isVarArg = false);

	// Write human-readable ir to file , for debug and testing.
	static void WriteReadableIr(llvm::Module* module, const char* file, bool print = false);
	// Write compilable ir to file , for further compilation.
	static void WriteBitCodeIr(llvm::Module* module, const char* file);



};


#endif
