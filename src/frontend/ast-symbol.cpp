
#include "debug.h"
#include "AST/declarations/class-decl.h"
#include "AST/declarations/field-list.h"
#include "ast-symbol.h"
#include "AST/type.h"
void frontend::ASTSymbol::CreateScope() {
	fields.emplace_back(std::map<std::string, std::shared_ptr<AST::Type>>());
}
void frontend::ASTSymbol::EndScope() {
	fields.pop_back();
}

#pragma region Get
std::shared_ptr<AST::Type> frontend::ASTSymbol::GetField(std::string name) {
	std::shared_ptr<AST::Type>  v = nullptr;
	for (int i = fields.size() - 1; i >= 0; i--) {
		if (fields[i].find(name) != fields[i].end()) {
			v = fields[i][name];
			break;
		}
	}
	if (!v && fields.back().find("this") != fields.back().end()) {
        const auto this_ptr = fields.back()["this"];
        const auto this_decl = std::static_pointer_cast<AST::CustomType>(this_ptr)->decl;
		auto this_fields = this_decl->fields;
        const auto idx = this_fields->FindByName(name);
		if (idx != -1) v= this_fields->content[idx]->type;
	}
    //TODO find in Base
	return v;
}

std::shared_ptr<AST::Type> frontend::ASTSymbol::GetMemberField(const std::shared_ptr<AST::Type>type, const std::string name) {
    if(type->category==AST::Type::Custom) {
		auto decl =GetClassDecl(type->ToString());
		if (!decl)return nullptr;
        const auto idx=decl->fields->FindByName(name);
		if (idx != -1)
			return decl->fields->content[idx]->type;
    }
    return nullptr;
}

std::shared_ptr<AST::decl::ClassDecl> frontend::ASTSymbol::GetClassTemplate(const std::string name) {
	if (class_templates.find(name) == class_templates.end())return nullptr;
	return class_templates[name];
}

std::shared_ptr<AST::decl::FunctionDecl>frontend::ASTSymbol::GetFuncTemplate(std::string name) {
	if (function_templates.find(name) == function_templates.end())return nullptr;
	return function_templates[name];
}

std::shared_ptr<AST::decl::FunctionDecl> frontend::ASTSymbol::GetFunctionDecl(std::string name) {
	if (functions.find(name) != functions.end())
		return functions[name];
	if (extern_functions.find(name) != extern_functions.end())
		return extern_functions[name];
	return nullptr;
}

std::shared_ptr<AST::CustomType> frontend::ASTSymbol::GetClass(std::string name) {
	if (types.find(name) != types.end())return types[name];
	return nullptr;
}

std::shared_ptr<AST::decl::ClassDecl> frontend::ASTSymbol::GetClassDecl(std::string name) {
	if (types.find(name) != types.end())return types[name]->decl;
	return nullptr;
}

int frontend::ASTSymbol::GetCustomTypeCategory(const std::string ty) {
	return IsCustomType(ty) ? types[ty]->category : -1;
}
#pragma endregion

#pragma region Add
void frontend::ASTSymbol::AddField(const std::string k, const std::shared_ptr<AST::Type> v) {
	Debugger::Debug("Add field {}:{}", k, v->ToString());
	fields.back()[k] = v;
}

void frontend::ASTSymbol::AddExternFunc(std::string name, std::shared_ptr<AST::decl::FunctionDecl> decl) {
    extern_functions[name] = decl;
}

void frontend::ASTSymbol::AddClassTemplate(std::string name, std::shared_ptr<AST::decl::ClassDecl> decl) {
    class_templates[name] = decl;
}

void frontend::ASTSymbol::AddFuncTemplate(std::string name, std::shared_ptr<AST::decl::FunctionDecl> decl) {
    function_templates[name] = decl;
}

void frontend::ASTSymbol::AddAlias(std::string from, std::string to) {
    func_alias[from] = to;
}

void frontend::ASTSymbol::AddClass(std::string name, std::shared_ptr<AST::CustomType> ty) {
    types[name] = ty;
}

void frontend::ASTSymbol::AddFunction(std::string name, std::shared_ptr<AST::decl::FunctionDecl> decl) {
    functions[name] = decl;
}
#pragma endregion

bool frontend::ASTSymbol::IsCustomType(const std::string name) {
	return types.find(name) != types.end();
}

std::string frontend::ASTSymbol::Alias(std::string name) {
    if (func_alias.find(name) != func_alias.end())
        return func_alias[name];
    return name;
}