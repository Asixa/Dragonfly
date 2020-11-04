#include "AST/expressions/function-call.h"
#include "AST/program.h"
#include "AST/declarations/field-list.h"

namespace AST {
	using namespace decl;
	using namespace expr;
	void FuncCall::ToString() {
		if (left != nullptr)left->ToString();
		*Debugger::out << name.c_str() << "(";
		for (auto& arg : args) {
			arg->ToString();
			*Debugger::out << ",";
		}
		*Debugger::out << ")";
		if (child != nullptr) {
			*Debugger::out << ".";
			child->ToString();
		}
	}

	std::shared_ptr<FuncCall> FuncCall::Parse(std::string f) {
		auto func_call = std::make_shared<FuncCall>(f);
		if (Lexer::Check('<'))
			func_call->generic = FieldList::ParseGenericInstantiate();
		Lexer::Match('(');
		while (Lexer::token->type != ')') {
			func_call->args.push_back(Binary::Parse());
			if (Lexer::Check(',')) Lexer::Match(',');
		}
		Lexer::Match(')');
		return func_call;
	}

	llvm::Value* FuncCall::Gen(const std::shared_ptr<DFContext> context,bool is_var) {
		return GenField(context,nullptr);
	}

    std::shared_ptr<AST::Type> FuncCall::Analysis(const std::shared_ptr<DFContext>ctx) {
		return AnalysisField(ctx, nullptr);
	}

    std::shared_ptr<AST::Type> FuncCall::AnalysisField(std::shared_ptr<DFContext>ctx, std::shared_ptr<AST::Type>parent) {
		
		// the data structure of Field for exmaple: a.B().C().d is
		//  [a]
		//     \child
		//      [C()]            cannot be linked list:  a->B()->C()->d 
		//   left/  \child       because left recursion happens when parse nested funtion.
		//  [B()]     [d]
		// so here we DFS to generate previous nodes.

		// true if nested function call like A().B(), then we left-DFS
		// TODO:  test needed , I think it should be "parent = left->GenField(parent);" added fix arg_list
		
		if (left != nullptr) parent = left->AnalysisField(ctx, parent);


		// if is calling a member function, eg: a.bar(), a is any object of custom class or struct.
		// if true, parent is the value of a.
        is_member_func = parent != nullptr;
		callee_name = is_member_func ? parent->ToString() + JOINER_TAG + name : name;
		
		if (generic) {
			const auto template_class_decl = ctx->GetClassTemplate(callee_name);
			const auto template_func_decl = ctx->GetFuncTemplate(callee_name);
			if (template_class_decl != nullptr) {
				class_type =template_class_decl->InstantiateTemplate(ctx, generic)->GetType();
				callee_name += generic->ToString()+JOINER_TAG+BUILTIN_TAG+"init";
				is_constructor = true;
				is_member_func = true;

			}
			else if (template_func_decl != nullptr) {
				template_func_decl->InstantiateTemplate(ctx, generic);
				callee_name += generic->ToString();
			}
			else {
				Debugger::ErrorV((std::string("Template not found ") + callee_name).c_str(), line, ch);
				return nullptr;
			}
		}

		if (ctx->IsCustomType(callee_name)) {
            if(ctx->GetCustomTypeCategory(callee_name) < ClassDecl::kInterface) {
				Debugger::ErrorV("Cannot call a constructor of a interface or basic type", line, ch);
				return nullptr;
            }
			class_type = ctx->types_table[callee_name];
			// now the function is a construtor, which is a special type of member function.
			// we add back the  front name.
			is_member_func = true;
			callee_name = callee_name + JOINER_TAG + "$init";
			is_constructor = true;
		}


		param_name += "(";
		for (int i = 0, argv_size = args.size(); i < argv_size; i++)
			param_name += args[i]->Analysis(ctx)->ToString() + (i == argv_size - 1 ? "" : ",");
		param_name += ")";


		func = ctx->GetFunctionDecl(callee_name);
		if (!func) func = ctx->GetFunctionDecl(callee_name + param_name);
		
        if(!func) {
			 Debugger::ErrorV((std::string("Unknown function referenced:") + callee_name + param_name).c_str(), line, ch);
			 return nullptr;
        }
		// here we found the callee!
        // check if the function argument count matchs.
        // some function could have varible arguments size when isVarArg is true.

		if (!func->args->IsVariableArgument()&&func->args->content.size() != args.size() + (is_member_func ? 1 : 0) ) {
			Debugger::ErrorV((std::string("Incorrect # arguments passed: ") +
				std::to_string(func->args->content.size()) + " needed, but got " + std::to_string(args.size() + (is_member_func ? 1 : 0)) +" instead").c_str(), line, ch);
			return nullptr;
		}
		return is_constructor?class_type:func->return_type;
	}

    llvm::Value* FuncCall::GenField(std::shared_ptr<DFContext> ctx,llvm::Value* _this) {
		// TODO function call like a()()
        
		// the data structure of Field for exmaple: a.B().C().d is
		//  [a]
		//     \child
		//      [C()]            cannot be linked list:  a->B()->C()->d 
		//   left/  \child       because left recursion happens when parse nested funtion.
		//  [B()]     [d]
		// so here we DFS to generate previous nodes.
  
		// true if nested function call like A().B(), then we left-DFS
		// TODO:  test needed , I think it should be "parent = left->GenField(parent);" added fix arg_list
		if (left != nullptr) _this = left->GenField(ctx,_this);
  

		// Here we generate all the parameters for the Call, store the values into args_v
		// hidden pointer for member func is not added here. will added later.
		std::vector<llvm::Value*> args_v;
		for (unsigned i = 0, e = args.size(); i != e; ++i) {
			// val is the llvm::Value* of current argument experssion.
			auto val = args[i]->Gen(ctx,false);
			if (ctx->GetCustomTypeCategory(val->getType()) == ClassDecl::kStruct)
				while (ctx->GetPtrDepth(val) != 0)
					val = ctx->builder->CreateLoad(val);
			// here we catch if the val is null. 
			if (!val)return Debugger::ErrorV("Incorrect # arguments passed with error",line,ch);
			// add the value the the args list.
			args_v.push_back(val);
		}

  
		// here we check if the func is a constructor
		if (is_constructor) {
			switch (class_type->decl->category) {
			case ClassDecl::kClass: 
				_this = ctx->Malloc(class_type->ToLLVM(ctx)->getPointerElementType());
				break;
			case ClassDecl::kStruct:
				_this = ctx->CreateEntryBlockAlloca(class_type->ToLLVM(ctx), callee_name);
				break;
			case ClassDecl::kInterface:
				return  Debugger::ErrorV("Cannot call a constructor of a interface type",line,ch);
			default: //here the category should be -1
				return Debugger::ErrorV("Cannot call a constructor of a basic type", line, ch);
			}
		}

		// now we add the hidden pointer to the args list if it is a member function. //TODO catch errors like pass a value
		if (is_member_func) {
			const auto ptr_depth = ctx->GetPtrDepth(_this);
			auto this_arg = _this;
			if (ptr_depth == 2) // ptr_depth = 2 means it is X**, but we want X*
				this_arg = ctx->builder->CreateLoad(_this);
			args_v.insert(args_v.begin(), this_arg);
		}

		
		// now, callee_name is the full function name, eg: "A::bar" or "foo"  etc.
		// and we try get the fuction.
	 
		if (!is_constructor) {
            const auto ty = ctx->module->getTypeByName(name);
			if (ty) {
				if (ctx->GetFunction(callee_name + param_name) != nullptr) {
					is_member_func = true;
					callee_name +=param_name;
					is_constructor = true;
                    //HACK
					_this = ctx->Malloc(ty->getPointerTo());
					const auto ptr_depth = ctx->GetPtrDepth(_this);
					auto this_arg = _this;
					if (ptr_depth == 2) // ptr_depth = 2 means it is X**, but we want X*
						this_arg = ctx->builder->CreateLoad(_this);
					args_v.insert(args_v.begin(), this_arg);
				}
			}
		}
		auto callee = ctx->GetFunction(callee_name);
		
		// if the function not exist, maybe because it is a overloaded function.
		// then the name should be "A::bar()" or "foo(int)" etc.
		// we ignore the hidden pointer
		if (!callee) {
			callee_name += param_name;
			callee = ctx->GetFunction(callee_name);
		}
  
		// if we still cannot find the function, we could now throw a error.
		if (!callee)
			return Debugger::ErrorV((std::string("Unknown function referenced in GEN:") + callee_name).c_str(),line,ch);
		
        while (ctx->GetPtrDepth(args_v[0])>1)                               // TODO this is a hack, should be removed
			args_v[0] = ctx->builder->CreateLoad(args_v[0]);

        for(auto i=0;i<callee->arg_size();i++) {
			// printf("%s *%d == %s *%d\n", ctx->GetStructName(callee->getArg(i)).c_str(), ctx->GetPtrDepth(callee->getArg(i)), ctx->GetStructName(args_v[i]).c_str(), ctx->GetPtrDepth(args_v[i]));
            const auto want_ptr = ctx->GetPtrDepth(callee->getArg(i));
            while (ctx->GetPtrDepth(args_v[i])>want_ptr) 
				args_v[i] = ctx->builder->CreateLoad(args_v[i]);
        }
		// call the function and save it to v
		llvm::Value* value = ctx->builder->CreateCall(callee, args_v);
		
		// since constructor returns void, the value shoud be the initized 'parent'
		if (is_constructor) value = _this;
		// it the field is not done yet. continue.
		
		if (child != nullptr) {
			child->is_ptr = is_ptr;
			value = child->GenField(ctx, value);
		}
		return value;
	}


}