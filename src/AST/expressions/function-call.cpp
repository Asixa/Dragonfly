#include "AST/expressions/function-call.h"
#include "AST/program.h"


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
			func_call->generic = GenericParam::Parse();
		Lexer::Match('(');
		while (Lexer::token->type != ')') {
			func_call->args.push_back(Binary::Parse());
			if (Lexer::Check(',')) Lexer::Match(',');
		}
		Lexer::Match(')');
		return func_call;
	}

	llvm::Value* FuncCall::Gen(std::shared_ptr<DFContext> context,int cmd) {
		return GenField(context,nullptr);
	}

	llvm::Value* FuncCall::GenField(std::shared_ptr<DFContext> ctx,llvm::Value* parent) {
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
		if (left != nullptr)
			parent = left->GenField(ctx,parent);

		// if is calling a constructor, eg: A(), A is any class or struct.
		auto is_constructor = false;
		// if is calling a member function, eg: a.bar(), a is any object of custom class or struct.
		// if true, parent is the value of a.
		auto is_member_func = parent != nullptr;

		// if the function is a memeber function, for example for class A.
		// then the name will be [A::name], otherwise the name is just [name]
		auto callee_name = is_member_func
			? ctx->GetStructName(parent) + "::" + name
			: name;

        if(generic) {
			
			const auto template_class_decl = ctx->GetTemplateClass(callee_name);
            if (template_class_decl != nullptr) {
			    template_class_decl->Instantiate(ctx,generic);
			}
            else {
				const auto template_decl = ctx->GetTemplateFunc(callee_name);
				if (template_decl == nullptr)
					return Debugger::ErrorV((std::string("Template not found ") + callee_name).c_str(), line, ch);
  				template_decl->Instantiate(ctx,generic);
            }
            
			
			callee_name += generic->ToString();
        }

		// Here we generate all the parameters for the Call, store the values into args_v
		// hidden pointer for member func is not added here. will added later.
		std::vector<llvm::Value*> args_v;
		for (unsigned i = 0, e = args.size(); i != e; ++i) {
			// val is the llvm::Value* of current argument experssion.
			auto val = args[i]->Gen(ctx);

			// all Struct type will 'pass as value',
			// except hidden pointer for member func, which is not added yet.
			if (ctx->GetCustomTypeCategory(val->getType()) == ClassDecl::kStruct)
				while (ctx->GetPtrDepth(val) != 0)
					val = ctx->builder->CreateLoad(val);

			// here we catch if the val is null. 
			if (!val)return Debugger::ErrorV("Incorrect # arguments passed with error",line,ch);
			// add the value the the args list.
			args_v.push_back(val);
		}

		// here we check if the func is a constructor
		if (ctx->IsCustomType(callee_name)) {
			switch (ctx->GetCustomTypeCategory(callee_name)) {
			case ClassDecl::kClass: {
				auto ty = ctx->module->getTypeByName(callee_name);
				parent = ctx->Malloc(ty);
				break;
			}
			case ClassDecl::kStruct:
				parent = ctx->CreateEntryBlockAlloca(ctx->module->getTypeByName(callee_name), callee_name);
				break;
			case ClassDecl::kInterface:
				return  Debugger::ErrorV("Cannot call a constructor of a interface type",line,ch);
			default: //here the category should be -1
				return Debugger::ErrorV("Cannot call a constructor of a basic type", line, ch);
			}
			// now the function is a construtor, which is a special type of member function.
			// we add back the  front name.
			is_member_func = true;
			callee_name = callee_name + "::" + callee_name;
			is_constructor = true;
		}
	
		// now we add the hidden pointer to the args list if it is a member function. //TODO catch errors like pass a value
		if (is_member_func) {
			const auto ptr_depth = ctx->GetPtrDepth(parent);
			auto this_arg = parent;
			if (ptr_depth == 2) // ptr_depth = 2 means it is X**, but we want X*
				this_arg = ctx->builder->CreateLoad(parent);
			args_v.insert(args_v.begin(), this_arg);
		}
		std::string param_name;
		param_name += "(";
		for (int i = is_member_func ? 1 : 0, argv_size = args_v.size(); i < argv_size; i++)
			param_name += ctx->GetStructName(args_v[i]->getType()) + (i == argv_size - 1 ? "" : ",");
		param_name += ")";
		// after fix the function name, we try to get it again.
		
		// now, callee_name is the full function name, eg: "A::bar" or "foo"  etc.
		// and we try get the fuction.
	
		if (!ctx->IsCustomType(callee_name)) {
			auto ty = ctx->module->getTypeByName(name);
			printf("%s\n", name.c_str());
			if (ty) {
				printf("try get callee %s\n", (name + "::" + callee_name + param_name).c_str());
				if (ctx->GetFunction(name + "::" + callee_name + param_name) != nullptr) {
					is_member_func = true;
					callee_name = name + "::" + callee_name+param_name;
					is_constructor = true;
					
					printf("---------------");
                    //HACK
					parent = ctx->Malloc(ty->getPointerTo());
					const auto ptr_depth = ctx->GetPtrDepth(parent);
					auto this_arg = parent;
					if (ptr_depth == 2) // ptr_depth = 2 means it is X**, but we want X*
						this_arg = ctx->builder->CreateLoad(parent);
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
			return Debugger::ErrorV((std::string("Unknown function referenced :") + callee_name).c_str(),line,ch);
		
		// here we found the callee!
		// check if the function argument count matchs.
		// some function could have varible arguments size when isVarArg is true.
		if (callee->arg_size() != args_v.size() && !callee->isVarArg())
			return Debugger::ErrorV((std::string("Incorrect # arguments passed: ") +
				std::to_string(args.size() + (is_member_func ? 1 : 0)) + " / " + std::to_string(callee->arg_size())).c_str(), line, ch);

		printf("arg size %d\n",ctx->GetPtrDepth(args_v[0]));
        while (ctx->GetPtrDepth(args_v[0])>1) {
			args_v[0] = ctx->builder->CreateLoad(args_v[0]);
        }

		// call the function and save it to v
		llvm::Value* v = ctx->builder->CreateCall(callee, args_v);

		// since constructor returns void, the value shoud be the initized 'parent'
		v = is_constructor ? parent : v;

		// it the field is not done yet. continue.
		if (child != nullptr)
		{
			child->cmd = cmd;
			v = child->GenField(ctx,v);
		}

		return v;
	}


}