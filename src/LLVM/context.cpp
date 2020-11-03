
#include <codecvt>
#include <iostream>
#include <fstream>
#include <utility>

#include <llvm/Support/raw_ostream.h>
#include "LLVM/context.h"
#include "frontend/debug.h"

#include "AST/program.h"
#include "AST/type.h"

std::vector<std::shared_ptr<DFContext>> DFContext::contexts;


AST::decl::ClassDecl* DFContext::GetClassTemplate(const std::string name) {
	if (class_template_table.find(name) == class_template_table.end())return nullptr;
	return class_template_table[name];
}

AST::decl::FunctionDecl* DFContext::GetFuncTemplate(std::string name) {
	if (function_template_table.find(name) == function_template_table.end())return nullptr;
	return function_template_table[name];
}

std::shared_ptr<AST::decl::FunctionDecl> DFContext::GetFunctionDecl(std::string name) {
	if (functions_table.find(name) != functions_table.end())
		return functions_table[name];
	if (extern_functions_table.find(name) != extern_functions_table.end())
		return extern_functions_table[name];
	return nullptr;
}

llvm::GlobalVariable* DFContext::CreateGlobal(const std::string name, llvm::Type* ty) {
	module->getOrInsertGlobal(name, ty);
	auto g_var = module->getNamedGlobal(name);
	g_var->setLinkage(llvm::GlobalValue::CommonLinkage);
	g_var->setAlignment(4);
	return g_var;
}

// llvm::ConstantInt* DFContext::CreateConstant(const int value) {
// 	return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), value);
// }

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

// llvm::Type* DFContext::GetType(std::shared_ptr<AST::Type>  type) {
//
// 	llvm::Type* llvm_type = nullptr;
// 	if (type->ty > 0) {
// 		switch (type->ty) {
// 		case K_void:
// 		case 1:             llvm_type = llvm::Type::getVoidTy(context); break;
// 		case K_byte:        llvm_type = llvm::Type::getInt8Ty(context); break;
// 		case K_short:       llvm_type = llvm::Type::getInt16Ty(context); break;
// 		case K_int:         llvm_type = llvm::Type::getInt32Ty(context); break;
// 		case K_long:        llvm_type = llvm::Type::getInt64Ty(context); break;
// 		case K_float:       llvm_type = llvm::Type::getFloatTy(context); break;
// 		case K_double:      llvm_type = llvm::Type::getDoubleTy(context); break;
// 		case K_bool:        llvm_type = llvm::Type::getInt1Ty(context); break;
// 		case K_string:      llvm_type = llvm::Type::getInt8PtrTy(context); break;
// 			// case K_ushort:       return llvm::Type::getInt16Ty(DFContext::context);break;
// 			// case K_uint:         return llvm::Type::getInt32Ty(DFContext::context);break;
// 			// case K_ulong:        return llvm::Type::getInt64Ty(DFContext::context);break;
// 		default:
// 			Debugger::ErrorV("in DFContext::GetType , unexpected typename", -1, -1);
// 			return nullptr;
// 		}
// 	}
// 	else {
// 		if (IsCustomType(type->str)) {
// 			const auto ty = module->getTypeByName(type->str);
// 			llvm_type = types_table[type->str]->category == AST::decl::ClassDecl::kClass ? ty->getPointerTo() : static_cast<llvm::Type*>(ty);
// 		}
// 		else {
//
// 			llvm_type = module->getTypeByName(type->str);
// 			if (llvm_type == nullptr) {
// 				Debugger::ErrorV((std::string("Unknown Type: ") + type->str + "\n").c_str(), -1, -1);
// 				return nullptr;
// 			}
// 		}
// 	}
//
// 	if (type->array == -1)return llvm_type;
// 	if (type->array == -2)return llvm_type->getPointerTo();
// 	return llvm::ArrayType::get(llvm_type, type->array);
// }

llvm::StoreInst* DFContext::AlignStore(llvm::StoreInst* a) {
	// a->setAlignment(MaybeAlign(8));
	return a;
}

llvm::LoadInst* DFContext::AlignLoad(llvm::LoadInst* a) {
	// a->setAlignment(MaybeAlign(8));
	return a;
}
void DFContext::BuildInFunc(std::shared_ptr<DFContext> ctx,std::string name, std::shared_ptr<AST::Type> ret,  std::vector<std::shared_ptr<AST::Type>> args, const bool isVarArg)  {

	auto decl = std::make_shared<AST::decl::FunctionDecl>();
	decl->return_type = ret;
	decl->args = std::make_shared<AST::decl::FieldList>(args);
	decl->args->type = AST::decl::FieldList::Arguments;
	if (isVarArg)decl->args->content.push_back(std::make_shared<AST::decl::FieldDecl>("...", nullptr)); // var
	ctx->extern_functions_table[name] = decl;

    std::vector<llvm::Type*> llvm_types;
    for (auto ty : args) llvm_types.push_back(ty->ToLLVM(ctx));
	llvm::Function::Create(llvm::FunctionType::get(ret->ToLLVM(ctx), llvm_types, isVarArg), llvm::Function::ExternalLinkage, name,ctx->module.get());
}
// auto DFContext::BuildInFunc(const char* name, llvm::Type* ret, const std::vector<llvm::Type*> types,const bool isVarArg) const -> void {
// 	std::vector<llvm::Type*> types;
// 	llvm::Function::Create(llvm::FunctionType::get(ret, types, isVarArg), llvm::Function::ExternalLinkage, name,
// 		module.get());
// }

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


bool DFContext::IsCustomType(std::string name) {
	return types_table.find(name) != types_table.end();
}

int DFContext::GetCustomTypeCategory(llvm::Type* ty) {
	return GetCustomTypeCategory(GetStructName(ty));
}

int DFContext::GetCustomTypeCategory(const std::string ty) {
	return IsCustomType(ty) ? types_table[ty]->category : -1;
}


bool DFContext::ExistClass(std::string name) {
	return  (types_table.find(name) != types_table.end());
}

DFContext::DFContext(std::shared_ptr<AST::Program> program) {
    if(program==nullptr)return;

    module  =  std::make_unique<llvm::Module>("Program", context);
    builder = std::make_unique<llvm::IRBuilder<>>(context);
    data_layout = std::make_unique < llvm::DataLayout>(module.get());

	auto this_ptr = std::shared_ptr<DFContext>(this);
	ast = std::make_shared<Symbol>(this_ptr);
	llvm = std::make_shared<LLVMSymbol>(this_ptr);

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

void DFContext::Analysis() {
	for (const auto& context : contexts) {
		if (context->program)context->program->Analysis(context);
	}
}

std::shared_ptr<DFContext> DFContext::Create(std::shared_ptr<AST::Program> program) {
	auto ptr = std::make_shared<DFContext>(program);
	contexts.push_back(ptr);
	return ptr;
}
