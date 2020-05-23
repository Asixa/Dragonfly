#include "AST/expressions/new-expr.h"
#include "codegen.h"

std::shared_ptr<parser::New> parser::New::Parse() {
	Lexer::Next();
	auto instance = std::make_shared<New>();
	auto name = Lexer::string_val;
	Lexer::Match(Id);
	if (Lexer::Check('(')) {
		instance->func = FuncCall::Parse(name);
		return instance;
	}
	else Debugger::Error(L"expected '('");
    
}

void parser::New::ToString() {
    
}

llvm::Value* parser::New::Gen(const int cmd) {

	ClassDecl* decl = nullptr;
	const auto mangled_str = func->name;
	for (auto i : CodeGen::types_table)
		if (i.first == mangled_str)
			decl = i.second;
	if (decl == nullptr)
		return Debugger::ErrorV((std::string("unknown type A") + mangled_str).c_str(),line,ch);
	// const auto the_function = CodeGen::builder.GetInsertBlock()->getParent();
	auto value = CodeGen::Malloc(CodeGen::the_module->getTypeByName(mangled_str));
    // const auto alloca = CodeGen::CreateEntryBlockAlloca(the_function, CodeGen::GetType(func->name), mangled_str);
	// alloca->setAlignment(llvm::MaybeAlign(8));
	func->GenField(value);
	return value;

}
