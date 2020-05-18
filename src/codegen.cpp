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


#include <codecvt>
#include <iostream>
#include <fstream>

#include <llvm/Support/raw_ostream.h>
#include "codegen.h"

std::shared_ptr<parser::Program>  CodeGen::program=nullptr;
llvm::LLVMContext CodeGen::the_context;
std::unique_ptr<llvm::Module> CodeGen::the_module = std::make_unique<llvm::Module>("Program", CodeGen::the_context);
llvm::IRBuilder<> CodeGen::builder(CodeGen::the_context);
// llvm::Function* CodeGen::the_function = nullptr;
std::map<std::string, llvm::Value*> CodeGen::local_fields_table;
std::map<std::string, llvm::Value*> CodeGen::global_fields_table;
std::map<std::string, parser::ClassDecl*> CodeGen::types_table;
llvm::Value* CodeGen::True = llvm::ConstantInt::get(llvm::IntegerType::get(CodeGen::the_context, 1), 1);
llvm::Value* CodeGen::False = llvm::ConstantInt::get(llvm::IntegerType::get(CodeGen::the_context, 1), 0);

bool CodeGen::is_sub_block = false;
llvm::BasicBlock* CodeGen::block_begin = nullptr;
llvm::BasicBlock* CodeGen::block_end = nullptr;

// llvm::Value* CodeGen::This = nullptr;

llvm::Value* CodeGen::LogErrorV(const char* str) {
    *Debugger::out << str;
    system("PAUSE");
    exit(-1);
}

llvm::GlobalVariable* CodeGen::CreateGlob(llvm::IRBuilder<>& builder, const std::string name, llvm::Type* ty) {
    the_module->getOrInsertGlobal(name, ty);
    auto g_var = the_module->getNamedGlobal(name);
    g_var->setLinkage(llvm::GlobalValue::CommonLinkage);
    g_var->setAlignment(4);
    return g_var;
}

llvm::Function* CodeGen::CreateMainFunc() {
    const auto func_type = llvm::FunctionType::get(builder.getInt32Ty(), false);
    const auto func = llvm::Function::Create(func_type, llvm::GlobalValue::ExternalLinkage, "main",the_module.get());
    return func;
}

llvm::BasicBlock* CodeGen::CreateBasicBlock(llvm::Function* func, const std::string name) {
    return llvm::BasicBlock::Create(CodeGen::the_context, name, func);
}

llvm::AllocaInst* CodeGen::CreateEntryBlockAlloca(llvm::Type* type,const std::string& var_name, llvm::Function* the_function) {
    if(the_function==nullptr)
        the_function= builder.GetInsertBlock()->getParent();
    llvm::IRBuilder<> tmp_b(&the_function->getEntryBlock(), the_function->getEntryBlock().begin());
	auto alloca = tmp_b.CreateAlloca(type, 0, var_name);
	alloca->setAlignment(llvm::MaybeAlign(8));
    return alloca;
}

std::string CodeGen::MangleStr(const std::wstring str) {
    return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().
        to_bytes(str);
}

llvm::Type* CodeGen::GetTypeByName(std::wstring type_name) {
	// if (type_name[0] == L'\0')printf("size %d %d\n", type_name.size(), type_name[0]);
	if (type_name.size() != 1|| (type_name[0]>0&&type_name[0]<128)||Lexer::IsCjk(type_name[0])) {
        const auto mangled_name = MangleStr(type_name);
        const auto ty = the_module->getTypeByName(mangled_name);
		return types_table[mangled_name]->category== parser::ClassDecl::kClass?ty->getPointerTo():static_cast<llvm::Type*>(ty);
	}
	    
    const int type = type_name[0];
    switch (type) {
    case L'\0': return llvm::Type::getVoidTy(CodeGen::the_context);
    case K_int: return llvm::Type::getInt32Ty(CodeGen::the_context);
    case K_float: return llvm::Type::getFloatTy(CodeGen::the_context);
    case K_double: return llvm::Type::getDoubleTy(CodeGen::the_context);
    case K_bool: return llvm::Type::getInt1Ty(CodeGen::the_context);
    case K_string: return llvm::Type::getInt8PtrTy(CodeGen::the_context);
	default:
		CodeGen::LogErrorV("in CodeGen::GetType , unexpected typename");
        return nullptr;
    }
}

llvm::StoreInst* CodeGen::AlignStore(llvm::StoreInst* a) {
    // a->setAlignment(MaybeAlign(8));
    return a;
}

llvm::LoadInst* CodeGen::AlignLoad(llvm::LoadInst* a) {
    // a->setAlignment(MaybeAlign(8));
    return a;
}

void CodeGen::BuildInFunc(const char* name, llvm::Type* ret, std::vector<llvm::Type*> types, bool isVarArg) {
    llvm::Function::Create(llvm::FunctionType::get(ret, types, isVarArg), llvm::Function::ExternalLinkage, name,
                           the_module.get());
}

void CodeGen::WriteReadableIr(llvm::Module* module, const char* file, bool print) {
	std::string ir;
	llvm::raw_string_ostream ir_stream(ir);
	ir_stream << *module;
	ir_stream.flush();
	std::ofstream file_stream;
	file_stream.open(file);
	file_stream << ir;
	file_stream.close();
	if (print)std::cout << ir;
}

void CodeGen::WriteBitCodeIr(llvm::Module* module, const char* file) {
	std::error_code ec;
	llvm::raw_fd_ostream os(file, ec, llvm::sys::fs::F_None);
	WriteBitcodeToFile(*module, os);
	os.flush();
}


int CodeGen::GetPtrDepth(llvm::Value* value) {
	return GetPtrDepth(value->getType());
}

int CodeGen::GetPtrDepth(llvm::Type* type) {
	auto depth = 0;
	while (type->getTypeID() == llvm::Type::PointerTyID) {
		depth++;
		type = type->getPointerElementType();
	}
	return depth;

}

std::string CodeGen::GetStructName(llvm::Value* value) {
	return GetStructName(value->getType());
}
std::string CodeGen::GetStructName(llvm::Type* type) {
	while (type->getTypeID() == llvm::Type::PointerTyID)
		type = type->getPointerElementType();
    switch (type->getTypeID()) {
	case llvm::Type::DoubleTyID:
		return "double";
	case llvm::Type::IntegerTyID:
		return "int";
	case llvm::Type::FloatTyID:
		return "float";
	default:
		return type->getStructName();
    }

	
}

std::string CodeGen::DebugValue(llvm::Value* value) {
	std::string ir;
	llvm::raw_string_ostream ir_stream(ir);
	value->print(ir_stream, true);
	return ir;
}

llvm::Value* CodeGen::Malloc(llvm::Type* type) {
	const auto ptr = CodeGen::builder.CreateCall(CodeGen::the_module->getFunction("malloc"),
		llvm::ConstantInt::get(llvm::Type::getInt32Ty(CodeGen::the_context), 32));
    const auto value= CodeGen::builder.CreateCast(llvm::Instruction::BitCast, ptr, type->getPointerTo());

  //   const auto decl = types_table[type->getStructName()];
  //   if(!decl->base_type_name.empty()) {
  //       const auto base = CodeGen::FindMemberField(value,L"base");
  //       const auto base_constructor = CodeGen::the_module->getFunction(CodeGen::MangleStr(decl->base_type_name));
  //       const std::vector<llvm::Value*> args_v;
		// auto obj = builder.CreateCall(base_constructor, args_v, "base");
		// // CodeGen::builder.CreateStore(,base );
  //   }
    return value;
}


llvm::Value* CodeGen::FindMemberField(llvm::Value* obj, const std::wstring name) {
	const auto obj_type_name = CodeGen::GetStructName(obj);


	// get the this's type and check if it contains the field
    auto decl = CodeGen::types_table[obj_type_name];
	auto idx = -1;
	for (int id = 0, n = decl->fields.size(); id < n; id++) {
		if (decl->fields[id] == name) { 
			idx = id;
			break;
		}
	}

    // get the base's type and check if it contains the field
	if (idx == -1) {
        if(!decl->base_type_name.empty()) {
			auto base_decl = CodeGen::types_table[CodeGen::MangleStr(decl->base_type_name)];
			for (int id = 0, n = decl->fields.size(); id < n; id++) {
				if (base_decl->fields[id] == name) {
					idx = id;
					break;
				}
			}
            if(idx!=-1) {
                const auto base = CodeGen::builder.CreateStructGEP(obj, 0);                      // base is A**
				return  CodeGen::builder.CreateStructGEP(base, idx);     // after a load, we get A* and return it.
            }
        }
        return CodeGen::LogErrorV("Cannot find field... \n");
	}

	return  CodeGen::builder.CreateStructGEP(obj, idx);
}

llvm::Value* CodeGen::FindField(const std::wstring name, int cmd, const bool warn) {
	llvm::Value* v = nullptr;
	const auto mangle_name = CodeGen::MangleStr(name);

	// find this field in local like function argument
	if (!v && CodeGen::local_fields_table.find(mangle_name) != CodeGen::local_fields_table.end())
		v = CodeGen::local_fields_table[mangle_name];
	//for (auto& it : local_fields_table)std::cout << it.first << ",";std::cout << std::endl;
	// find this field in this
	if (!v && CodeGen::local_fields_table.find("this") != CodeGen::local_fields_table.end()) {
		auto this_fields = types_table[CodeGen::GetStructName(local_fields_table["this"])]->fields;
		if (std::find(this_fields.begin(), this_fields.end(), name) != this_fields.end()) {
			v = CodeGen::FindMemberField(CodeGen::local_fields_table["this"], name);
            v= cmd == 0 ? CodeGen::builder.CreateLoad(v, "this." + mangle_name):v;
		}  
	}

	// find this field in base
	if (!v && CodeGen::local_fields_table.find("base") != CodeGen::local_fields_table.end()) {
	    auto base_field = types_table[CodeGen::GetStructName(local_fields_table["base"])]->fields;
		if (std::find(base_field.begin(), base_field.end(), name) != base_field.end()) {
			v = CodeGen::FindMemberField(CodeGen::builder.CreateLoad(CodeGen::local_fields_table["base"]), name);
			v = cmd == 0 ? CodeGen::builder.CreateLoad(v, "base." + mangle_name) : v;
		}
	}
	// find this field in global varibales
	if (!v && CodeGen::global_fields_table.find(mangle_name) != CodeGen::global_fields_table.end())
		v = CodeGen::global_fields_table[mangle_name];

	// TODO find v in public Enums
	// TODO find v in namespaces

	return  !v && warn ? CodeGen::LogErrorV((std::string("Unknown variable name: ") + mangle_name + "\n").c_str()) : v;
}

bool CodeGen::IsCustomType(std::string name) {
	return types_table.find(name) != CodeGen::types_table.end();
}

int CodeGen::GetCustomTypeCategory(llvm::Type* ty) {
	return GetCustomTypeCategory(GetStructName(ty));
}

int CodeGen::GetCustomTypeCategory(const std::string ty) {
	return IsCustomType(ty) ? types_table[ty]->category : -1;
}
