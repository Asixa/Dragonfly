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
llvm::DataLayout CodeGen::data_layout = llvm::DataLayout(the_module.get());

// llvm::Function* CodeGen::the_function = nullptr;
std::map<std::string, llvm::Value*> CodeGen::local_fields_table;
std::map<std::string, llvm::Value*> CodeGen::global_fields_table;
std::map<std::string, parser::ClassDecl*> CodeGen::types_table;
std::map<std::string, std::string> CodeGen::func_alias_table;

std::map<std::string, parser::ClassDecl*> CodeGen::template_types_table;
std::map<std::string, parser::FunctionDecl*> CodeGen::template_function_table;

llvm::Value* CodeGen::True = llvm::ConstantInt::get(llvm::IntegerType::get(CodeGen::the_context, 1), 1);
llvm::Value* CodeGen::False = llvm::ConstantInt::get(llvm::IntegerType::get(CodeGen::the_context, 1), 0);
llvm::Type* CodeGen::void_ptr = llvm::Type::getInt8PtrTy(CodeGen::the_context);
llvm::Type* CodeGen::void_type = llvm::Type::getVoidTy(CodeGen::the_context);
llvm::Type* CodeGen::int32 = llvm::Type::getInt32Ty(CodeGen::the_context);

bool CodeGen::is_sub_block = false;
llvm::BasicBlock* CodeGen::block_begin = nullptr;
llvm::BasicBlock* CodeGen::block_end = nullptr;

parser::FunctionDecl* CodeGen::current_function = nullptr;


parser::ClassDecl* CodeGen::GetTemplateClass(const std::string name) {
    if(template_types_table.find(name)==template_types_table.end())return nullptr;
	return template_types_table[name];
}

parser::FunctionDecl* CodeGen::GetTemplateFunc(std::string name) {
	if (template_function_table.find(name) == template_function_table.end())return nullptr;
	return template_function_table[name];
}

llvm::GlobalVariable* CodeGen::CreateGlob(const std::string name, llvm::Type* ty) {
    the_module->getOrInsertGlobal(name, ty);
    auto g_var = the_module->getNamedGlobal(name);
    g_var->setLinkage(llvm::GlobalValue::CommonLinkage);
    g_var->setAlignment(4);
    return g_var;
}

llvm::ConstantInt* CodeGen::CreateConstant(const int value) {
	return llvm::ConstantInt::get(llvm::Type::getInt32Ty(CodeGen::the_context), value);
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




llvm::Type* CodeGen::GetType(parser::Type  type) {

	llvm::Type* llvm_type = nullptr;
	if (type.ty > 0) {
		switch (type.ty) {
		case K_void:
		case 1:             llvm_type = llvm::Type::getVoidTy(CodeGen::the_context); break;
		case K_byte:        llvm_type = llvm::Type::getInt8Ty(CodeGen::the_context); break;
		case K_short:       llvm_type = llvm::Type::getInt16Ty(CodeGen::the_context); break;
		case K_int:         llvm_type = llvm::Type::getInt32Ty(CodeGen::the_context); break;
		case K_long:        llvm_type = llvm::Type::getInt64Ty(CodeGen::the_context); break;
		case K_float:       llvm_type = llvm::Type::getFloatTy(CodeGen::the_context); break;
		case K_double:      llvm_type = llvm::Type::getDoubleTy(CodeGen::the_context); break;
		case K_bool:        llvm_type = llvm::Type::getInt1Ty(CodeGen::the_context); break;
		case K_string:      llvm_type = llvm::Type::getInt8PtrTy(CodeGen::the_context); break;
		// case K_ushort:       return llvm::Type::getInt16Ty(CodeGen::the_context);break;
        // case K_uint:         return llvm::Type::getInt32Ty(CodeGen::the_context);break;
        // case K_ulong:        return llvm::Type::getInt64Ty(CodeGen::the_context);break;
		default:
			Debugger::ErrorV("in CodeGen::GetType , unexpected typename", -1, -1);
			return nullptr;
		}
	}
	else {
		if (IsCustomType(type.str)) {
			const auto ty = the_module->getTypeByName(type.str);
			llvm_type = types_table[type.str]->category == parser::ClassDecl::kClass ? ty->getPointerTo() : static_cast<llvm::Type*>(ty);
		}
		else {
			Debugger::ErrorV((std::string("Unknown Type: ") + type.str + "\n").c_str(), -1, -1);
			return nullptr;
		}
	}

	if (type.array == -1)return llvm_type;
	if (type.array == -2)return llvm_type->getPointerTo();
	return llvm::ArrayType::get(llvm_type, type.array);
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
	if (type == void_ptr)return "void*";
	while (type->getTypeID() == llvm::Type::PointerTyID)
		type = type->getPointerElementType();
    switch (type->getTypeID()) {
	case llvm::Type::DoubleTyID:
		return "double";
	case llvm::Type::IntegerTyID:
		return "int";
	case llvm::Type::FloatTyID:
		return "float";
	case llvm::Type::VoidTyID:
		return "void";
	default:
		return type->getStructName();
    }

	
}

llvm::Function* CodeGen::GetFunction(std::string name) {
	if (func_alias_table.find(name) != func_alias_table.end())
		name = func_alias_table[name];
	return the_module->getFunction(name);
}

std::string CodeGen::DebugValue(llvm::Value* value) {
	std::string ir;
	llvm::raw_string_ostream ir_stream(ir);
	value->print(ir_stream, true);
	return ir;
}

llvm::Value* CodeGen::Malloc(llvm::Type* type,bool cast) {
	const auto ptr = CodeGen::builder.CreateCall(CodeGen::the_module->getFunction("malloc"),
		llvm::ConstantInt::get(llvm::Type::getInt32Ty(CodeGen::the_context), data_layout.getTypeStoreSize(type)));
	return CodeGen::builder.CreateCast(llvm::Instruction::BitCast, ptr, type->getPointerTo());
}

void CodeGen::Free(llvm::Value* value) {
	CodeGen::builder.CreateCall(CodeGen::the_module->getFunction("free"), 
		builder.CreateCast(llvm::Instruction::BitCast, value, void_ptr));
}

llvm::Value* CodeGen::FindMemberField(llvm::Value* obj, const std::string name) {
   
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
			auto base_decl = CodeGen::types_table[decl->base_type_name.str];
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
        return Debugger::ErrorV("Cannot find field... \n",-1,-1);
	}
	return  CodeGen::builder.CreateStructGEP(obj, idx);
}

llvm::Value* CodeGen::FindField(const std::string name, int cmd, const bool warn) {
	llvm::Value* v = nullptr;
	const auto mangle_name = name;

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

	return  !v && warn ? Debugger::ErrorV((std::string("Unknown variable name: ") + mangle_name + "\n").c_str(),-1,-1) : v;
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
