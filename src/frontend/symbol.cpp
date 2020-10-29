#include "symbol.h"
#include "debug.h"
#include "AST/declarations/class-decl.h"

void frontend::Symbol::CreateScope() {
	fields.push_back(std::map<std::string, AST::Type*>());
}
void frontend::Symbol::EndScope() {
	fields.pop_back();
}

void frontend::Symbol::AddField(std::string k, AST::Type* v) {
	fields.back()[k] = v;
}

AST::Type* frontend::Symbol::GetField(std::string k) {
	for (auto i = fields.size() - 1; i > 0; i--) {
		if (fields[i].find(k) != fields[i].end())
			return fields[i][k];
	}
	return nullptr;
}


void frontend::LLVMSymbol::CreateScope() {
	fields.push_back(std::map<std::string, llvm::Value*>());
}
void frontend::LLVMSymbol::EndScope() {
	fields.pop_back();
}

void frontend::LLVMSymbol::AddField(std::string k, llvm::Value* v) {
	fields.back()[k] = v;
}

llvm::Value* frontend::LLVMSymbol::GetField(std::string k) {
    for (auto i=fields.size()-1;i>0;i--) {
		if (fields[i].find(k) != fields[i].end())
			return fields[i][k];
    }
	return nullptr;
}

llvm::Value* frontend::LLVMSymbol::GetMemberField(llvm::Value* obj, std::string name) {
	const auto obj_type_name = GetStructName(obj->getType());

	// get the this's type and check if it contains the field
	auto decl = ctx->types_table[obj_type_name]->decl;
	auto idx = -1;
	for (int id = 0, n = decl->fields.size(); id < n; id++) {
		if (decl->fields[id]->name == name) {
			idx = id;
			break;
		}
	}

	// get the base's type and check if it contains the field
	if (idx == -1) {
		if (!decl->base_type->empty()) {
			auto base_decl = ctx->types_table[decl->base_type->str]->decl;
			for (int id = 0, n = decl->fields.size(); id < n; id++) {
				if (base_decl->fields[id]->name == name) {
					idx = id;
					break;
				}
			}
			if (idx != -1) {
				const auto base = ctx->builder->CreateStructGEP(obj, 0);                      // base is A**
				return  ctx->builder->CreateStructGEP(base, idx);     // after a load, we get A* and return it.
			}
		}
		return Debugger::ErrorV("Cannot find field... \n", -1, -1);
	}
	return  ctx->builder->CreateStructGEP(obj, idx);
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
