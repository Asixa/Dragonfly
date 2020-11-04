
#include "debug.h"
#include "AST/declarations/class-decl.h"
#include "AST/declarations/field-list.h"
#include "ast-symbol.h"
#include "AST/type.h"
void frontend::Symbol::CreateScope() {
	fields.emplace_back(std::map<std::string, std::shared_ptr<AST::Type>>());
}
void frontend::Symbol::EndScope() {
	fields.pop_back();
}

void frontend::Symbol::AddField(const std::string k, const std::shared_ptr<AST::Type> v) {
	fields.back()[k] = v;
}

std::shared_ptr<AST::Type> frontend::Symbol::GetField(std::string k) {
	for (int i = fields.size() - 1; i >= 0; i--) {
		if (fields[i].find(k) != fields[i].end())
			return fields[i][k];
	}
	return nullptr;
}

std::shared_ptr<AST::Type> frontend::Symbol::GetMemberField(const std::shared_ptr<AST::Type>type, const std::string name) {
    if(type->category==AST::Type::Custom) {
		auto decl =GetClassDecl(type->ToString());
		if (!decl)return nullptr;
        const auto idx=decl->fields->FindByName(name);
		if (idx != -1)
			return decl->fields->content[idx]->type;
    }
    return nullptr;
}

std::shared_ptr<AST::decl::ClassDecl> frontend::Symbol::GetClassTemplate(const std::string name) {
	if (class_template_table.find(name) == class_template_table.end())return nullptr;
	return class_template_table[name];
}

std::shared_ptr<AST::decl::FunctionDecl>frontend::Symbol::GetFuncTemplate(std::string name) {
	if (function_template_table.find(name) == function_template_table.end())return nullptr;
	return function_template_table[name];
}

std::shared_ptr<AST::decl::FunctionDecl> frontend::Symbol::GetFunctionDecl(std::string name) {
	if (functions_table.find(name) != functions_table.end())
		return functions_table[name];
	if (extern_functions_table.find(name) != extern_functions_table.end())
		return extern_functions_table[name];
	return nullptr;
}

bool frontend::Symbol::IsCustomType(const std::string name) {
	return types_table.find(name) != types_table.end();
}

void frontend::Symbol::AddExternFunc(std::string name, std::shared_ptr<AST::decl::FunctionDecl> decl) {
    extern_functions_table[name] = decl;
}

void frontend::Symbol::AddClassTemplate(std::string name, std::shared_ptr<AST::decl::ClassDecl> decl) {
    class_template_table[name] = decl;
}

void frontend::Symbol::AddFuncTemplate(std::string name, std::shared_ptr<AST::decl::FunctionDecl> decl) {
    function_template_table[name] = decl;
}

void frontend::Symbol::AddAlias(std::string from, std::string to) {
    func_alias_table[from] = to;
}

void frontend::Symbol::AddClass(std::string name, std::shared_ptr<AST::CustomType> ty) {
    types_table[name] = ty;
}

void frontend::Symbol::AddFunction(std::string name, std::shared_ptr<AST::decl::FunctionDecl> decl) {
    functions_table[name] = decl;
}

std::shared_ptr<AST::CustomType> frontend::Symbol::GetClass(std::string name) {
    if (types_table.find(name) != types_table.end())return types_table[name];
    return nullptr;
}

std::shared_ptr<AST::decl::ClassDecl> frontend::Symbol::GetClassDecl(std::string name) {
    if (types_table.find(name) != types_table.end())return types_table[name]->decl;
    return nullptr;
}


int frontend::Symbol::GetCustomTypeCategory(const std::string ty) {
	return IsCustomType(ty) ? types_table[ty]->category : -1;
}

std::string frontend::Symbol::Alias(std::string name) {
    if (func_alias_table.find(name) != func_alias_table.end())
        return func_alias_table[name];
    return name;
}
