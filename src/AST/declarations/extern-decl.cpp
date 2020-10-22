#include "AST/declarations/extern-decl.h"

using namespace frontend;
namespace AST{

	using namespace AST::decl;
	std::shared_ptr<AST::Extern> AST::Extern::Parse() {
		auto instance = std::make_shared<Extern>();
		Lexer::Next();
		if (Lexer::Check(K_class)) {
			Lexer::Next();
			instance->type = K_class;
			instance->name = Lexer::string_val;
			Lexer::Match(Id);
			if (Lexer::Check(K_as)) {
				Lexer::Next();
				instance->alias = Name::Parse(Name::kClass);
			}
			Lexer::MatchSemicolon();
		}
		else if (Lexer::Check(K_func)) {
			Lexer::Next();
			instance->name = Lexer::string_val;
			instance->type = K_func;
			Lexer::Match(Id);

			Lexer::Match('(');
			instance->args = FuncParam::Parse();
			Lexer::Match(')');
			if (Lexer::Check(':')) {
				Lexer::Next();
				instance->return_type = Lexer::MatchType();
			}
			else instance->return_type.ty = K_void;
			if (Lexer::Check(K_as)) {
				Lexer::Next();
				instance->alias = Name::Parse(Name::kFunction);
				if (Lexer::Check(':')) {
					Lexer::Match(':');
					Lexer::Match(':');
					instance->init = true;
					instance->alias->type = Name::kClass;
					Lexer::Match(K_init);
				}

			}
			Lexer::MatchSemicolon();
		}
		else Debugger::Error(L"Expected class or func");
		return instance;
	}
	void Extern::GenHeader(std::shared_ptr<DFContext> ctx) {
		if (type == K_func) {
			std::string func_name = name;
			std::vector<llvm::Type*> arg_types;
			for (auto i = 0; i < args->size; i++)
				arg_types.push_back(ctx->GetType(args->types[i]));
			std::string param_name = "";
			
			llvm::Type* parent_type = nullptr;
			if (alias!=nullptr&&!alias->names.empty()) {
		
				if (!alias->GetClassName().empty()) {
					parent_type = ctx->module->getTypeByName(alias->GetClassName());
					if (parent_type && parent_type != arg_types[0]) {
						Debugger::ErrorV("the first argument nust be the member Class if it is a member function", line, ch);
						return;
					}
					arg_types[0] = arg_types[0]->getPointerTo();
				}
				func_name = alias->GetFullName();
			}
			

			if (init) {
				func_name = alias->GetFullNameWithoutFunc();
				func_name += parent_type->getStructName();
				printf("parent is null? %s -  %d  %d\n", func_name.c_str() , parent_type == nullptr, alias->type);
			
			}
		
			param_name += "(";
			for (int i = parent_type == nullptr ? 0 : 1, types_size = arg_types.size(); i < types_size; i++)
				param_name += ctx->GetStructName(arg_types[i]) + (i == arg_types.size() - 1 ? "" : ",");
			param_name += ")";
			if (alias != nullptr&&(alias->type != 0||init)) {
				printf(" set alias %s to %s\n", (func_name + param_name).c_str(), name.c_str());
				ctx->func_alias_table[func_name + param_name] = name;
			}


			auto the_function = ctx->module->getFunction(name);
			if (!the_function) {
				const auto func_type = llvm::FunctionType::get(ctx->GetType(return_type), arg_types, args->is_var_arg);
				the_function = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, name, ctx->module.get());
				unsigned idx = 0;
				for (auto& arg : the_function->args())
					arg.setName(args->names[idx++]);
			}
			else  Debugger::ErrorV((std::string("function ") + name + std::string(" already defined\n")).c_str(), line, ch);
		}
		else  if (type == K_class) {
			auto the_struct = ctx->module->getTypeByName(name);
			if (!the_struct) {
				the_struct = llvm::StructType::create(ctx->context, name);
			}
			else {
				*Debugger::out << "Type " << name.c_str() << " already defined" << std::endl;
				return;
			}
		}
	}

	void Extern::Gen(std::shared_ptr<DFContext> context) {

	}

}