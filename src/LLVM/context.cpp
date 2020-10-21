
#include <codecvt>
#include <iostream>
#include <fstream>

#include <llvm/Support/raw_ostream.h>
#include "LLVM/context.h"
#include "frontend/debug.h"

#include "frontend/parser.h"

parser::ClassDecl* DFContext::GetTemplateClass(const std::string name) {
	if (template_types_table.find(name) == template_types_table.end())return nullptr;
	return template_types_table[name];
}

parser::FunctionDecl* DFContext::GetTemplateFunc(std::string name) {
	if (template_function_table.find(name) == template_function_table.end())return nullptr;
	return template_function_table[name];
}

llvm::GlobalVariable* DFContext::CreateGlob(const std::string name, llvm::Type* ty) {
	module->getOrInsertGlobal(name, ty);
	auto g_var = module->getNamedGlobal(name);
	g_var->setLinkage(llvm::GlobalValue::CommonLinkage);
	g_var->setAlignment(4);
	return g_var;
}

llvm::ConstantInt* DFContext::CreateConstant(const int value) {
	return llvm::ConstantInt::get(llvm::Type::getInt32Ty(DFContext::context), value);
}

llvm::Function* DFContext::CreateMainFunc() {
	const auto func_type = llvm::FunctionType::get(builder->getInt32Ty(), false);
	const auto func = llvm::Function::Create(func_type, llvm::GlobalValue::ExternalLinkage, "main", module.get());
	return func;
}

llvm::BasicBlock* DFContext::CreateBasicBlock(llvm::Function* func, const std::string name) {
	return llvm::BasicBlock::Create(DFContext::context, name, func);
}

llvm::AllocaInst* DFContext::CreateEntryBlockAlloca(llvm::Type* type, const std::string& var_name, llvm::Function* the_function) {
	if (the_function == nullptr)
		the_function = builder->GetInsertBlock()->getParent();
	llvm::IRBuilder<> tmp_b(&the_function->getEntryBlock(), the_function->getEntryBlock().begin());
	auto alloca = tmp_b.CreateAlloca(type, 0, var_name);
	alloca->setAlignment(llvm::MaybeAlign(8));
	return alloca;
}

llvm::Type* DFContext::GetType(parser::Type  type) {

	llvm::Type* llvm_type = nullptr;
	if (type.ty > 0) {
		switch (type.ty) {
		case K_void:
		case 1:             llvm_type = llvm::Type::getVoidTy(DFContext::context); break;
		case K_byte:        llvm_type = llvm::Type::getInt8Ty(DFContext::context); break;
		case K_short:       llvm_type = llvm::Type::getInt16Ty(DFContext::context); break;
		case K_int:         llvm_type = llvm::Type::getInt32Ty(DFContext::context); break;
		case K_long:        llvm_type = llvm::Type::getInt64Ty(DFContext::context); break;
		case K_float:       llvm_type = llvm::Type::getFloatTy(DFContext::context); break;
		case K_double:      llvm_type = llvm::Type::getDoubleTy(DFContext::context); break;
		case K_bool:        llvm_type = llvm::Type::getInt1Ty(DFContext::context); break;
		case K_string:      llvm_type = llvm::Type::getInt8PtrTy(DFContext::context); break;
			// case K_ushort:       return llvm::Type::getInt16Ty(DFContext::context);break;
			// case K_uint:         return llvm::Type::getInt32Ty(DFContext::context);break;
			// case K_ulong:        return llvm::Type::getInt64Ty(DFContext::context);break;
		default:
			Debugger::ErrorV("in DFContext::GetType , unexpected typename", -1, -1);
			return nullptr;
		}
	}
	else {
		if (IsCustomType(type.str)) {
			const auto ty = module->getTypeByName(type.str);
			llvm_type = types_table[type.str]->category == parser::ClassDecl::kClass ? ty->getPointerTo() : static_cast<llvm::Type*>(ty);
		}
		else {

			llvm_type = DFContext::module->getTypeByName(type.str);
			if (llvm_type == nullptr) {
				Debugger::ErrorV((std::string("Unknown Type: ") + type.str + "\n").c_str(), -1, -1);
				return nullptr;
			}
		}
	}

	if (type.array == -1)return llvm_type;
	if (type.array == -2)return llvm_type->getPointerTo();
	return llvm::ArrayType::get(llvm_type, type.array);
}

llvm::StoreInst* DFContext::AlignStore(llvm::StoreInst* a) {
	// a->setAlignment(MaybeAlign(8));
	return a;
}

llvm::LoadInst* DFContext::AlignLoad(llvm::LoadInst* a) {
	// a->setAlignment(MaybeAlign(8));
	return a;
}

void DFContext::BuildInFunc(const char* name, llvm::Type* ret, std::vector<llvm::Type*> types, bool isVarArg) {
	llvm::Function::Create(llvm::FunctionType::get(ret, types, isVarArg), llvm::Function::ExternalLinkage, name,
		module.get());
}

void DFContext::WriteReadableIr(llvm::Module* module, const char* file, bool print) {
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

void DFContext::WriteBitCodeIr(llvm::Module* module, const char* file) {
	std::error_code ec;
	llvm::raw_fd_ostream os(file, ec, llvm::sys::fs::F_None);
	WriteBitcodeToFile(*module, os);
	os.flush();
}

int DFContext::GetPtrDepth(llvm::Value* value) {
	return GetPtrDepth(value->getType());
}

int DFContext::GetPtrDepth(llvm::Type* type) {
	auto depth = 0;
	while (type->getTypeID() == llvm::Type::PointerTyID) {
		depth++;
		type = type->getPointerElementType();
	}
	return depth;

}

std::string DFContext::GetStructName(llvm::Value* value) {
	return GetStructName(value->getType());
}
std::string DFContext::GetStructName(llvm::Type* type) {
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

llvm::Function* DFContext::GetFunction(std::string name) {
	printf("find %s in [", name.c_str());
	for (auto i : func_alias_table)printf("%s,", i.first.c_str());
	printf("]\n");
	if (func_alias_table.find(name) != func_alias_table.end()) {

		printf("find!  %s now named to %s\n", name.c_str(), func_alias_table[name].c_str());
		name = func_alias_table[name];
		printf("find real func: %d\n", module->getFunction(name) != nullptr);
	}

	return module->getFunction(name);
}

std::string DFContext::DebugValue(llvm::Value* value) {
	std::string ir;
	llvm::raw_string_ostream ir_stream(ir);
	value->print(ir_stream, true);
	return ir;
}

llvm::Value* DFContext::Malloc(llvm::Type* type, bool cast) {
	const auto ptr = DFContext::builder->CreateCall(DFContext::module->getFunction("malloc"),
		llvm::ConstantInt::get(llvm::Type::getInt32Ty(DFContext::context), data_layout->getTypeStoreSize(type)));
	return DFContext::builder->CreateCast(llvm::Instruction::BitCast, ptr, type->getPointerTo());
}

void DFContext::Free(llvm::Value* value) {
	DFContext::builder->CreateCall(DFContext::module->getFunction("free"),
		builder->CreateCast(llvm::Instruction::BitCast, value, void_ptr));
}

llvm::Value* DFContext::FindMemberField(llvm::Value* obj, const std::string name) {

	const auto obj_type_name = DFContext::GetStructName(obj);

	// get the this's type and check if it contains the field
	auto decl = DFContext::types_table[obj_type_name];
	auto idx = -1;
	for (int id = 0, n = decl->fields.size(); id < n; id++) {
		if (decl->fields[id] == name) {
			idx = id;
			break;
		}
	}

	// get the base's type and check if it contains the field
	if (idx == -1) {
		if (!decl->base_type_name.empty()) {
			auto base_decl = DFContext::types_table[decl->base_type_name.str];
			for (int id = 0, n = decl->fields.size(); id < n; id++) {
				if (base_decl->fields[id] == name) {
					idx = id;
					break;
				}
			}
			if (idx != -1) {
				const auto base = DFContext::builder->CreateStructGEP(obj, 0);                      // base is A**
				return  DFContext::builder->CreateStructGEP(base, idx);     // after a load, we get A* and return it.
			}
		}
		return Debugger::ErrorV("Cannot find field... \n", -1, -1);
	}
	return  DFContext::builder->CreateStructGEP(obj, idx);
}

llvm::Value* DFContext::FindField(const std::string name, int cmd, const bool warn) {
	llvm::Value* v = nullptr;
	const auto mangle_name = name;

	// find this field in local like function argument
	if (!v && DFContext::local_fields_table.find(mangle_name) != DFContext::local_fields_table.end())
		v = DFContext::local_fields_table[mangle_name];
	//for (auto& it : local_fields_table)std::cout << it.first << ",";std::cout << std::endl;
	// find this field in this
	if (!v && DFContext::local_fields_table.find("this") != DFContext::local_fields_table.end()) {
		auto this_fields = types_table[DFContext::GetStructName(local_fields_table["this"])]->fields;
		if (std::find(this_fields.begin(), this_fields.end(), name) != this_fields.end()) {
			v = DFContext::FindMemberField(DFContext::local_fields_table["this"], name);
			v = cmd == 0 ? DFContext::builder->CreateLoad(v, "this." + mangle_name) : v;
		}
	}

	// find this field in base
	if (!v && DFContext::local_fields_table.find("base") != DFContext::local_fields_table.end()) {
		auto base_field = types_table[DFContext::GetStructName(local_fields_table["base"])]->fields;
		if (std::find(base_field.begin(), base_field.end(), name) != base_field.end()) {
			v = DFContext::FindMemberField(DFContext::builder->CreateLoad(DFContext::local_fields_table["base"]), name);
			v = cmd == 0 ? DFContext::builder->CreateLoad(v, "base." + mangle_name) : v;
		}
	}
	// find this field in global varibales
	if (!v && DFContext::global_fields_table.find(mangle_name) != DFContext::global_fields_table.end())
		v = DFContext::global_fields_table[mangle_name];

	// TODO find v in public Enums
	// TODO find v in namespaces

	return  !v && warn ? Debugger::ErrorV((std::string("Unknown variable name: ") + mangle_name + "\n").c_str(), -1, -1) : v;
}

bool DFContext::IsCustomType(std::string name) {
	return types_table.find(name) != DFContext::types_table.end();
}

int DFContext::GetCustomTypeCategory(llvm::Type* ty) {
	return GetCustomTypeCategory(GetStructName(ty));
}

int DFContext::GetCustomTypeCategory(const std::string ty) {
	return IsCustomType(ty) ? types_table[ty]->category : -1;
}

DFContext::DFContext() {
	builder = std::make_unique<llvm::IRBuilder<>>(context);
	True = llvm::ConstantInt::get(llvm::IntegerType::get(context, 1), 1);
	False = llvm::ConstantInt::get(llvm::IntegerType::get(context, 1), 0);
	// llvm::Value* CodeGen::Null = llvm::Constant::getNullValue();
	void_ptr = llvm::Type::getInt8PtrTy(context);
	void_type = llvm::Type::getVoidTy(context);
	int32 = llvm::Type::getInt32Ty(context);

	is_sub_block = false;
	block_begin = nullptr;
	block_end = nullptr;

	current_function = nullptr;

}