#include "AST/expressions/new-expr.h"

using namespace frontend;
using namespace AST;
std::shared_ptr<expr::New> expr::New::Parse() {
	Lexer::Next();
	auto instance = std::make_shared<New>();
    const auto name = Lexer::string_val;
	Lexer::Match(Id);
	if (Lexer::Check('(')) {
		instance->func = FuncCall::Parse(name);
		return instance;
	}
	else Debugger::Error(L"expected '('");
    
}

void expr::New::ToString() {
    
}

llvm::Value* expr::New::Gen(const std::shared_ptr<DFContext> ctx,const int cmd) {

    AST::decl::ClassDecl* decl = nullptr;
	const auto mangled_str = func->name;
	for (auto i : ctx->types_table)
		if (i.first == mangled_str)
			decl = i.second;
	if (decl == nullptr)
		return Debugger::ErrorV((std::string("unknown type A") + mangled_str).c_str(),line,ch);
	// const auto the_function = CodeGen::builder.GetInsertBlock()->getParent();
	auto value = ctx->Malloc(ctx->module->getTypeByName(mangled_str));
    // const auto alloca = CodeGen::CreateEntryBlockAlloca(the_function, CodeGen::GetType(func->name), mangled_str);
	// alloca->setAlignment(llvm::MaybeAlign(8));
	func->GenField(ctx,value);
	return value;

}
