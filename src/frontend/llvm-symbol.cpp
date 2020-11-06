

#include "llvm-symbol.h"
#include "AST/declarations/class-decl.h"
#include "AST/declarations/field-list.h"


void frontend::LLVMSymbol::CreateScope() {
	fields.push_back(std::map<std::string, llvm::Value*>());
}

llvm::GlobalVariable* frontend::LLVMSymbol::CreateGlobal(const std::string name, llvm::Type* ty) {
	ctx->module->getOrInsertGlobal(name, ty);
	auto g_var = ctx->module->getNamedGlobal(name);
	// g_var->setLinkage(llvm::GlobalValue::CommonLinkage);
	g_var->setAlignment(4);
	g_var->setLinkage(llvm::GlobalValue::CommonLinkage);
	if (ty->isAggregateType()) {

		Debugger::Debug("{} is isAggregateType", ctx->llvm->GetStructName(ty));
	    g_var->setInitializer(llvm::ConstantAggregateZero::get(ty));
	}
	else {
        switch (ty->getTypeID()) {
		    case llvm::Type::IntegerTyID: g_var->setInitializer(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx->context), 0)); break;
		    case llvm::Type::PointerTyID: g_var->setInitializer(
                    llvm::ConstantPointerNull::get(static_cast<llvm::PointerType*>(ty)));
				
                break;
        }
	   
	}
	return g_var;
}

void frontend::LLVMSymbol::EndScope() {
	fields.pop_back();
}

void frontend::LLVMSymbol::AddField(std::string k, llvm::Value* v) {
	fields.back()[k] = v;
}


llvm::Value* frontend::LLVMSymbol::GetField(const std::string name, bool cmd) {
	llvm::Value* v = nullptr;
	const auto mangle_name = name;

	// find this field in local like function argument
	for (int i = fields.size() - 1; i >= 0; i--) {
		if (fields[i].find(name) != fields[i].end()) {
			v = fields[i][name];
			break;
		}
	}
	if (!v && fields.back().find("this") != fields.back().end()) {
		auto this_ptr = fields.back()["this"];
		auto this_decl = ctx->ast->GetClassDecl(GetStructName(this_ptr));
		auto this_fields = this_decl->fields;
		if (this_fields->FindByName(name) != -1) {
			// while (ctx->GetPtrDepth(this_ptr) > 1)
			// 	this_ptr = ctx->builder->CreateLoad(this_ptr);
			v = GetMemberField(this_ptr, name);
			v = !cmd && this_decl->category == AST::decl::ClassDecl::kClass ? ctx->builder->CreateLoad(v, "this." + name) : v;
		}
	}

	// // find this field in global varibales
	// if (!v && global_fields_table.find(mangle_name) != global_fields_table.end())
	// 	v = global_fields_table[mangle_name];

	// TODO find v in public Enums
	// TODO find v in namespaces

	return  !v ? Debugger::ErrorV(-1, -1,"Unknown variable name:{}", mangle_name ) : v;
}



llvm::Value* frontend::LLVMSymbol::GetMemberField(llvm::Value* obj, std::string name) {
	const auto obj_type_name = GetStructName(obj->getType());

	// get the this's type and check if it contains the field
	auto decl = ctx->ast->GetClassDecl(obj_type_name);
	auto idx = -1;
	for (int id = 0, n = decl->fields->content.size(); id < n; id++) {
		if (decl->fields->content[id]->name == name) {
			idx = id;
			break;
		}
	}

	// get the base's type and check if it contains the field
	if (idx == -1) {
		if (decl->base_type != nullptr) {
			auto base_decl = ctx->ast->GetClassDecl(decl->base_type->ToString());
			for (int id = 0, n = decl->fields->content.size(); id < n; id++) {
				if (base_decl->fields->content[id]->name == name) {
					idx = id;
					break;
				}
			}
			if (idx != -1) {
				const auto base = ctx->builder->CreateStructGEP(obj, 0);                      // base is A**
				return  ctx->builder->CreateStructGEP(base, idx);     // after a load, we get A* and return it.
			}
		}
		return Debugger::ErrorV(-1, -1,"Cannot find field... ");
	}
	while (GetPtrDepth(obj) > 1) {
		obj = ctx->builder->CreateLoad(obj);
	}

	if (GetPtrDepth(obj) == 0) {
		const auto alloca = CreateEntryBlockAlloca(obj->getType(), name);
		AlignStore(ctx->builder->CreateStore(obj, alloca));
		obj = alloca;
	}
	// all obj here should have one ptr depths
	return  ctx->builder->CreateStructGEP(obj, idx);
}
std::string frontend::LLVMSymbol::GetStructName(llvm::Value* value) {
	return GetStructName(value->getType());
}

std::string frontend::LLVMSymbol::GetStructName(llvm::Type* type) {
	if (type == ctx->void_ptr)return "void*";
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


int frontend::LLVMSymbol::GetCustomTypeCategory(llvm::Type* ty) {
	return ctx->ast->GetCustomTypeCategory(GetStructName(ty));
}

int frontend::LLVMSymbol::GetPtrDepth(llvm::Value* value) {
	return GetPtrDepth(value->getType());
}

int frontend::LLVMSymbol::GetPtrDepth(llvm::Type* type) {
	auto depth = 0;
	while (type->getTypeID() == llvm::Type::PointerTyID) {
		depth++;
		type = type->getPointerElementType();
	}
	return depth;
}

llvm::StoreInst* frontend::LLVMSymbol::AlignStore(llvm::StoreInst* a) {
	// a->setAlignment(MaybeAlign(8));
	return a;
}

llvm::LoadInst* frontend::LLVMSymbol::AlignLoad(llvm::LoadInst* a) {
	// a->setAlignment(MaybeAlign(8));
	return a;
}
llvm::Function* frontend::LLVMSymbol::CreateMainFunc() {
	const auto func_type = llvm::FunctionType::get(ctx->builder->getInt32Ty(), false);
	const auto func = llvm::Function::Create(func_type, llvm::GlobalValue::ExternalLinkage, "main", ctx->module.get());
	return func;
}

llvm::BasicBlock* frontend::LLVMSymbol::CreateBasicBlock(llvm::Function* func, const std::string name) {
	return llvm::BasicBlock::Create(ctx->context, name, func);
}

llvm::AllocaInst* frontend::LLVMSymbol::CreateEntryBlockAlloca(llvm::Type* type, const std::string& var_name, llvm::Function* the_function) {
	if (the_function == nullptr) the_function = ctx->builder->GetInsertBlock()->getParent();
	llvm::IRBuilder<> tmp_b(&the_function->getEntryBlock(), the_function->getEntryBlock().begin());
	auto alloca = tmp_b.CreateAlloca(type, nullptr, var_name);
	alloca->setAlignment(llvm::MaybeAlign(8));
	return alloca;
}



llvm::Function* frontend::LLVMSymbol::GetFunction(std::string name) {
	return ctx->module->getFunction(ctx->ast->Alias(name));
}

std::string frontend::LLVMSymbol::DebugValue(llvm::Value* value) {
	std::string ir;
	llvm::raw_string_ostream ir_stream(ir);
	value->print(ir_stream, true);
	return ir;
}

llvm::Value* frontend::LLVMSymbol::Malloc(llvm::Type* type, bool cast) {
	const auto ptr = ctx->builder->CreateCall(ctx->module->getFunction("malloc"),
		llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx->context), ctx->data_layout->getTypeStoreSize(type)));
	return ctx->builder->CreateCast(llvm::Instruction::BitCast, ptr, type->getPointerTo());
}

void frontend::LLVMSymbol::Free(llvm::Value* value) {
	ctx->builder->CreateCall(ctx->module->getFunction("free"),
	ctx->builder->CreateCast(llvm::Instruction::BitCast, value, ctx->void_ptr));
}
