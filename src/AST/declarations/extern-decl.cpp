#include "AST/declarations/extern-decl.h"
#include "AST/declarations/class-decl.h"
#include "AST/declarations/field-list.h"
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
				instance->alias = NestedName::Parse(NestedName::kClass);
			}
			Lexer::MatchSemicolon();
		}
		else if (Lexer::Check(K_func)) {
			Lexer::Next();
			instance->name = Lexer::string_val;
			instance->type = K_func;
			Lexer::Match(Id);
            instance->args = FieldList::ParseArguments(true,true);
			if (Lexer::Check(':')) {
				Lexer::Next();
				instance->return_type = Type::Match();
			}
			else instance->return_type=BasicType::Void;
			if (Lexer::Check(K_as)) {
				Lexer::Next();
				instance->alias = NestedName::Parse(NestedName::kFunction);
				if (Lexer::Check(':')) {
					Lexer::Match(':');
					Lexer::Match(':');
					instance->init = true;
					instance->alias->type = NestedName::kClass;
					Lexer::Match(K_init);
				}

			}
			Lexer::MatchSemicolon();
		}
		else Debugger::Error("Expected class or func");
		return instance;
	}

    void Extern::AnalysisHeader(std::shared_ptr<DFContext>) {}
    void Extern::Analysis(std::shared_ptr<DFContext>) {}

    void Extern::GenHeader(std::shared_ptr<DFContext> ctx) {
		if (type == K_func) {
			std::string func_name = name;
			std::vector<llvm::Type*> arg_types;
			for (int i = 0,size= args->content.size(); i <size ; i++)
				arg_types.push_back(args->content[i]->type->ToLLVM(ctx));
			std::string param_name = "";
			
			llvm::Type* parent_type = nullptr;
			if (alias!=nullptr&&!alias->names.empty()) {
		
				if (!alias->GetClassName().empty()) {
					parent_type = ctx->module->getTypeByName(alias->GetClassName());
					if (parent_type && parent_type != arg_types[0]) {
						Debugger::ErrorV(line, ch,"the first argument nust be the member Class if it is a member function");
						return;
					}
					arg_types[0] = arg_types[0]->getPointerTo();
				}
				func_name = alias->GetFullName();
			}
			

			if (init) {
				func_name = alias->GetFullNameWithoutFunc();
				func_name += parent_type->getStructName();
			}
		
			param_name += "(";
			for (int i = parent_type == nullptr ? 0 : 1, types_size = arg_types.size(); i < types_size; i++)
				param_name += ctx->llvm->GetStructName(arg_types[i]) + (i == arg_types.size() - 1 ? "" : ",");
			param_name += ")";
			if (alias != nullptr&&(alias->type != 0||init)) ctx->ast->AddAlias(func_name + param_name, name);
			


			auto the_function = ctx->module->getFunction(name);
			if (!the_function) {
				const auto func_type = llvm::FunctionType::get(return_type->ToLLVM(ctx), arg_types, args->IsVariableArgument());
				the_function = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, name, ctx->module.get());
				unsigned idx = 0;
				for (auto& arg : the_function->args())
					arg.setName(args->content[idx++]->name);
			}
			else  Debugger::ErrorV(line, ch,"function {} already defined", name);
		}
		else  if (type == K_class) {
			auto the_struct = ctx->module->getTypeByName(name);
			if (!the_struct) {
				the_struct = llvm::StructType::create(ctx->context, name);
			}
			else {
				Debugger::ErrorV(line, ch, "Type {} already defined", name);
				return;
			}
		}
	}

	void Extern::Gen(std::shared_ptr<DFContext> context) {

	}

	std::string Extern::GetName() { return name; }

}
