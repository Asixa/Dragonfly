
#include <codecvt>
#include <iostream>
#include <fstream>

#include <llvm/Support/raw_ostream.h>
#include "LLVM/context.h"
#include "frontend/debug.h"

#include "AST/program.h"
#include "AST/type.h"

std::vector<std::shared_ptr<DFContext>> DFContext::contexts;


AST::decl::ClassDecl* DFContext::GetTemplateClass(const std::string name) {
	if (template_types_table.find(name) == template_types_table.end())return nullptr;
	return template_types_table[name];
}

AST::decl::FunctionDecl* DFContext::GetTemplateFunc(std::string name) {
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
	return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), value);
}

llvm::Function* DFContext::CreateMainFunc() {
	const auto func_type = llvm::FunctionType::get(builder->getInt32Ty(), false);
	const auto func = llvm::Function::Create(func_type, llvm::GlobalValue::ExternalLinkage, "main", module.get());
	return func;
}

llvm::BasicBlock* DFContext::CreateBasicBlock(llvm::Function* func, const std::string name) {
	return llvm::BasicBlock::Create(context, name, func);
}

llvm::AllocaInst* DFContext::CreateEntryBlockAlloca(llvm::Type* type, const std::string& var_name, llvm::Function* the_function) {
	if (the_function == nullptr)
		the_function = builder->GetInsertBlock()->getParent();
	llvm::IRBuilder<> tmp_b(&the_function->getEntryBlock(), the_function->getEntryBlock().begin());
	auto alloca = tmp_b.CreateAlloca(type, 0, var_name);
	alloca->setAlignment(llvm::MaybeAlign(8));
	return alloca;
}

llvm::Type* DFContext::GetType(std::shared_ptr<AST::Type>  type) {

	llvm::Type* llvm_type = nullptr;
	if (type->ty > 0) {
		switch (type->ty) {
		case K_void:
		case 1:             llvm_type = llvm::Type::getVoidTy(context); break;
		case K_byte:        llvm_type = llvm::Type::getInt8Ty(context); break;
		case K_short:       llvm_type = llvm::Type::getInt16Ty(context); break;
		case K_int:         llvm_type = llvm::Type::getInt32Ty(context); break;
		case K_long:        llvm_type = llvm::Type::getInt64Ty(context); break;
		case K_float:       llvm_type = llvm::Type::getFloatTy(context); break;
		case K_double:      llvm_type = llvm::Type::getDoubleTy(context); break;
		case K_bool:        llvm_type = llvm::Type::getInt1Ty(context); break;
		case K_string:      llvm_type = llvm::Type::getInt8PtrTy(context); break;
			// case K_ushort:       return llvm::Type::getInt16Ty(DFContext::context);break;
			// case K_uint:         return llvm::Type::getInt32Ty(DFContext::context);break;
			// case K_ulong:        return llvm::Type::getInt64Ty(DFContext::context);break;
		default:
			Debugger::ErrorV("in DFContext::GetType , unexpected typename", -1, -1);
			return nullptr;
		}
	}
	else {
		if (IsCustomType(type->str)) {
			const auto ty = module->getTypeByName(type->str);
			llvm_type = types_table[type->str]->category == AST::decl::ClassDecl::kClass ? ty->getPointerTo() : static_cast<llvm::Type*>(ty);
		}
		else {

			llvm_type = module->getTypeByName(type->str);
			if (llvm_type == nullptr) {
				Debugger::ErrorV((std::string("Unknown Type: ") + type->str + "\n").c_str(), -1, -1);
				return nullptr;
			}
		}
	}

	if (type->array == -1)return llvm_type;
	if (type->array == -2)return llvm_type->getPointerTo();
	return llvm::ArrayType::get(llvm_type, type->array);
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

void DFContext::Write() {
	WriteReadableIr(module.get(), (std::string(module->getModuleIdentifier().c_str())+".txt").c_str(), true);
	WriteBitCodeIr(module.get(), (std::string(module->getModuleIdentifier().c_str()) + ".ll").c_str());
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
	const auto ptr = builder->CreateCall(module->getFunction("malloc"),
		llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), data_layout->getTypeStoreSize(type)));
	return builder->CreateCast(llvm::Instruction::BitCast, ptr, type->getPointerTo());
}

void DFContext::Free(llvm::Value* value) {
	builder->CreateCall(module->getFunction("free"),
		builder->CreateCast(llvm::Instruction::BitCast, value, void_ptr));
}

llvm::Value* DFContext::FindMemberField(llvm::Value* obj, const std::string name) {

	const auto obj_type_name = GetStructName(obj);

	// get the this's type and check if it contains the field
	auto decl = types_table[obj_type_name];
	auto idx = -1;
	for (int id = 0, n = decl->fields.size(); id < n; id++) {
		if (decl->fields[id] == name) {
			idx = id;
			break;
		}
	}

	// get the base's type and check if it contains the field
	if (idx == -1) {
		if (!decl->base_type_name->empty()) {
			auto base_decl = types_table[decl->base_type_name->str];
			for (int id = 0, n = decl->fields.size(); id < n; id++) {
				if (base_decl->fields[id] == name) {
					idx = id;
					break;
				}
			}
			if (idx != -1) {
				const auto base = builder->CreateStructGEP(obj, 0);                      // base is A**
				return  builder->CreateStructGEP(base, idx);     // after a load, we get A* and return it.
			}
		}
		return Debugger::ErrorV("Cannot find field... \n", -1, -1);
	}
	return  builder->CreateStructGEP(obj, idx);
}

llvm::Value* DFContext::FindField(const std::string name, int cmd, const bool warn) {
	llvm::Value* v = nullptr;
	const auto mangle_name = name;

	// find this field in local like function argument
	if (!v && local_fields_table.find(mangle_name) != local_fields_table.end())
		v = local_fields_table[mangle_name];
	//for (auto& it : local_fields_table)std::cout << it.first << ",";std::cout << std::endl;
	// find this field in this
	if (!v && local_fields_table.find("this") != local_fields_table.end()) {
		auto this_fields = types_table[GetStructName(local_fields_table["this"])]->fields;
		if (std::find(this_fields.begin(), this_fields.end(), name) != this_fields.end()) {
			v = FindMemberField(local_fields_table["this"], name);
			v = cmd == 0 ? builder->CreateLoad(v, "this." + mangle_name) : v;
		}
	}

	// find this field in base
	if (!v && local_fields_table.find("base") != local_fields_table.end()) {
		auto base_field = types_table[GetStructName(local_fields_table["base"])]->fields;
		if (std::find(base_field.begin(), base_field.end(), name) != base_field.end()) {
			v = FindMemberField(builder->CreateLoad(local_fields_table["base"]), name);
			v = cmd == 0 ? builder->CreateLoad(v, "base." + mangle_name) : v;
		}
	}
	// find this field in global varibales
	if (!v && global_fields_table.find(mangle_name) != global_fields_table.end())
		v = global_fields_table[mangle_name];

	// TODO find v in public Enums
	// TODO find v in namespaces

	return  !v && warn ? Debugger::ErrorV((std::string("Unknown variable name: ") + mangle_name + "\n").c_str(), -1, -1) : v;
}

bool DFContext::IsCustomType(std::string name) {
	return types_table.find(name) != types_table.end();
}

int DFContext::GetCustomTypeCategory(llvm::Type* ty) {
	return GetCustomTypeCategory(GetStructName(ty));
}

int DFContext::GetCustomTypeCategory(const std::string ty) {
	return IsCustomType(ty) ? types_table[ty]->category : -1;
}

DFContext::DFContext(std::shared_ptr<AST::Program> program) {
    if(program==nullptr)return;

    module  =  std::make_unique<llvm::Module>("Program", context);
    builder = std::make_unique<llvm::IRBuilder<>>(context);
    data_layout = std::make_unique < llvm::DataLayout>(module.get());

    //states
	is_sub_block = false;
	block_begin = nullptr;
	block_end = nullptr;
	current_function = nullptr;

    //constant
	True = llvm::ConstantInt::get(llvm::IntegerType::get(context, 1), 1);
	False = llvm::ConstantInt::get(llvm::IntegerType::get(context, 1), 0);
	// llvm::Value* CodeGen::Null = llvm::Constant::getNullValue();
	void_ptr = llvm::Type::getInt8PtrTy(context);
	void_type = llvm::Type::getVoidTy(context);
	int32 = llvm::Type::getInt32Ty(context);

	this->program = program;

}

void DFContext::Gen() {
    for (auto context : contexts) {
		if(context->program)context->program->Gen(context);
		if (!Debugger::error_existed) {
			context->Write();
			std::cout << "Context \""<< context->module->getModuleIdentifier().c_str() <<"\" Compiled successfully "<< "\n";
		}
		else std::cout << "Context \"" << context->module->getModuleIdentifier().c_str() << "\" Compilation Failed " << "\n";
    }
}

std::shared_ptr<DFContext> DFContext::Create(std::shared_ptr<AST::Program> program) {
	auto ptr = std::make_shared<DFContext>(program);
	contexts.push_back(ptr);
	return ptr;
}
