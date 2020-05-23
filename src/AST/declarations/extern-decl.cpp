#include "AST/declarations/extern-decl.h"
#include "codegen.h"


std::shared_ptr<parser::Extern> parser::Extern::Parse() {
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
		}
		Lexer::MatchSemicolon();
	}
	else Debugger::Error(L"Expected class or func");
	return instance;
}
void parser::Extern::GenHeader() {
    if(type==K_func) {
		std::string func_name=name;
		std::vector<llvm::Type*> arg_types;
		for (auto i = 0; i < args->size; i++)
			arg_types.push_back(CodeGen::GetType(args->types[i]));
		std::string param_name = "";

		llvm::Type* parent_type = nullptr;
		if (!alias->names.empty()) {
			if (!alias->GetClassName().empty()) {
				parent_type = CodeGen::the_module->getTypeByName(alias->GetClassName());
				if (parent_type && parent_type != arg_types[0]) {
					Debugger::ErrorV("the first argument nust be the member Class if it is a member function", line, ch);
					return;
				}
			}
            func_name= alias->GetFullName();
		}
		 
        if(init) {
			func_name = alias->GetFullNameWithoutFunc();
			func_name += "::";
			func_name +=parent_type->getStructName();
        }
		
		param_name += "(";
		for (int i = parent_type == nullptr ? 0 : 1, types_size = arg_types.size(); i < types_size; i++)
			param_name += CodeGen::GetStructName(arg_types[i]) + (i == arg_types.size() - 1 ? "" : ",");
		param_name += ")";
		if (alias->type != 0) {
			printf(" set alias %s to %s\n", (func_name + param_name).c_str(), name.c_str());
				CodeGen::func_alias_table[func_name + param_name] = name;
		}
			

		auto the_function = CodeGen::the_module->getFunction(name);
		if (!the_function) {
			const auto func_type = llvm::FunctionType::get(CodeGen::GetType(return_type), arg_types, args->is_var_arg);
			the_function = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, name, CodeGen::the_module.get());
			unsigned idx = 0;
			for (auto& arg : the_function->args())
				arg.setName(args->names[idx++]);
		}
		else  Debugger::ErrorV((std::string("function ") + name + std::string(" already defined\n")).c_str(), line, ch);
    }
    else  if(type==K_class) {
		auto the_struct = CodeGen::the_module->getTypeByName(name);
        if(!the_struct) {
			the_struct = llvm::StructType::create(CodeGen::the_context, name);
        }
		else {
			*Debugger::out << "Type " << name.c_str() << " already defined" << std::endl;
			return;
		}
    }
}

void parser::Extern::Gen() {
    
}

