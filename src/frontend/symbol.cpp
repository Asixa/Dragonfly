#include "symbol.h"
#include "debug.h"
#include "AST/declarations/class-decl.h"
#include "AST/declarations/field-list.h"
void frontend::Symbol::CreateScope() {
	fields.emplace_back(std::map<std::string, std::shared_ptr<AST::Type>>());
}
void frontend::Symbol::EndScope() {
	fields.pop_back();
}

void frontend::Symbol::AddField(std::string k, std::shared_ptr<AST::Type> v) {
	fields.back()[k] = v;
}

std::shared_ptr<AST::Type> frontend::Symbol::GetField(std::string k) {
	for (int i = fields.size() - 1; i >= 0; i--) {
		if (fields[i].find(k) != fields[i].end())
			return fields[i][k];
	}
	return nullptr;
}

std::shared_ptr<AST::Type> frontend::Symbol::GetMemberField(std::shared_ptr<AST::Type>type, std::string name) {
    if(type->category==AST::Type::Custom) {
		auto decl =ctx->types_table[type->ToString()] ->decl;
		if (!decl)return nullptr;
        const auto idx=decl->fields->FindByName(name);
		if (idx != -1)
			return decl->fields->content[idx]->type;
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
		auto this_decl = ctx->types_table[ctx->GetStructName(this_ptr)]->decl;
		auto this_fields = this_decl->fields;
		if (this_fields->FindByName(name) != -1) {
			// while (ctx->GetPtrDepth(this_ptr) > 1)
			// 	this_ptr = ctx->builder->CreateLoad(this_ptr);
			v = ctx->llvm->GetMemberField(this_ptr, name);
            v = !cmd && this_decl->category== AST::decl::ClassDecl::kClass ? ctx->builder->CreateLoad(v, "this." + name) : v;
		}
	}

	// // find this field in global varibales
	// if (!v && global_fields_table.find(mangle_name) != global_fields_table.end())
	// 	v = global_fields_table[mangle_name];

	// TODO find v in public Enums
	// TODO find v in namespaces

	return  !v  ? Debugger::ErrorV((std::string("Unknown variable name: ") + mangle_name + "\n").c_str(), -1, -1) : v;
}



llvm::Value* frontend::LLVMSymbol::GetMemberField(llvm::Value* obj, std::string name) {
	const auto obj_type_name = ctx->GetStructName(obj->getType());

	// get the this's type and check if it contains the field
	auto decl = ctx->types_table[obj_type_name]->decl;
	auto idx = -1;
	for (int id = 0, n = decl->fields->content.size(); id < n; id++) {
		if (decl->fields->content[id]->name == name) {
			idx = id;
			break;
		}
	}

	// get the base's type and check if it contains the field
	if (idx == -1) {
		if (decl->base_type!=nullptr) {
			auto base_decl = ctx->types_table[decl->base_type->ToString()]->decl;
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
		return Debugger::ErrorV("Cannot find field... \n", -1, -1);
	}
    while (ctx->GetPtrDepth(obj)>1) {
		obj = ctx->builder->CreateLoad(obj);
    }

    if(ctx->GetPtrDepth(obj) ==0) {
		const auto alloca = ctx->CreateEntryBlockAlloca(obj->getType(), name);
		ctx->AlignStore(ctx->builder->CreateStore(obj, alloca));
		obj = alloca;
    }
    // all obj here should have one ptr depths
	return  ctx->builder->CreateStructGEP(obj, idx);
}

