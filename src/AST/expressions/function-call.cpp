#include "AST/expressions/function-call.h"
#include "parser.h"
#include "codegen.h"

namespace parser {
	void FuncCall::ToString() {
		if (left != nullptr)left->ToString();
		*Debugger::out << name << "(";
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

	std::shared_ptr<FuncCall> FuncCall::Parse(std::wstring f) {
		auto func_call = std::make_shared<FuncCall>(f);
		Lexer::Next(); VERIFY
			while (Lexer::token->type != ')') {
				func_call->args.push_back(Binary::Parse());
				VERIFY
					if (Lexer::Check(',')) Lexer::Match(',');
				VERIFY
			}
		Lexer::Match(')');
		VERIFY
		return func_call;
	}

	llvm::Value* FuncCall::Gen(int cmd) {
		return GenField(nullptr);
	}

	llvm::Value* FuncCall::GenField(llvm::Value* parent) {
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
			parent = left->GenField(parent);

        // if is calling a constructor, eg: A(), A is any class or struct.
		auto is_constructor = false;
		// if is calling a member function, eg: a.bar(), a is any object of custom class or struct.
        // if true, parent is the value of a.
		auto is_member_func = parent != nullptr;

		// if the function is a memeber function, for example for class A.
		// then the name will be [A::name], otherwise the name is just [name]
		auto callee_name = is_member_func
			? CodeGen::GetStructName(parent) + "::" + CodeGen::MangleStr(name)
		    : CodeGen::MangleStr(name);

		// Here we generate all the parameters for the Call, store the values into args_v
        // hidden pointer for member func is not added here. will added later.
		std::vector<llvm::Value*> args_v;
		for (unsigned i = 0, e = args.size(); i != e; ++i) {
            // val is the llvm::Value* of current argument experssion.
		    auto val = args[i]->Gen();

            // all Struct type will 'pass as value',
		    // except hidden pointer for member func, which is not added yet.
            if(CodeGen::GetCustomTypeCategory(val->getType()) == ClassDecl::kStruct)
                while (CodeGen::GetPtrDepth(val) != 0) 
					val = CodeGen::builder.CreateLoad(val);

			// here we catch if the val is null. 
            if(!val)return CodeGen::LogErrorV("Incorrect # arguments passed with error");
			// add the value the the args list.
			args_v.push_back(val);
		}

        // here we check if the func is a constructor
		if (CodeGen::IsCustomType(callee_name)) {
			switch (CodeGen::GetCustomTypeCategory(callee_name)) {
			case ClassDecl::kClass: 
				parent = CodeGen::Malloc(CodeGen::the_module->getTypeByName(callee_name));
				break;
			case ClassDecl::kStruct: 
				parent = CodeGen::CreateEntryBlockAlloca(CodeGen::the_module->getTypeByName(callee_name), callee_name);
				break; 
			case ClassDecl::kInterface:
				return CodeGen::LogErrorV("Cannot call a constructor of a interface type");
			default: //here the category should be -1
				return CodeGen::LogErrorV("Cannot call a constructor of a basic type");
			}
            // now the function is a construtor, which is a special type of member function.
		    // we add back the  front name.
			is_member_func = true;
			callee_name = callee_name + "::" + callee_name;
			is_constructor = true;
		}

        // now we add the hidden pointer to the args list if it is a member function. //TODO catch errors like pass a value
		if (is_member_func) {
			const auto ptr_depth = CodeGen::GetPtrDepth(parent);
			auto this_arg = parent;
			if (ptr_depth == 2) // ptr_depth = 2 means it is X**, but we want X*
				this_arg = CodeGen::builder.CreateLoad(parent);
			args_v.insert(args_v.begin(), this_arg);
		}

		// now, callee_name is the full function name, eg: "A::bar" or "foo"  etc.
        // and we try get the fuction.
		auto callee = CodeGen::the_module->getFunction(callee_name);

        // if the function not exist, maybe because it is a overloaded function.
        // then the name should be "A::bar()" or "foo(int)" etc.
        // we ignore the hidden pointer
		if (!callee) {
			callee_name += "(";
			for (int i = is_member_func?1:0, argv_size = args_v.size(); i < argv_size; i++)
				callee_name += CodeGen::GetStructName(args_v[i]->getType()) + (i == argv_size - 1 ? "" : ", ");
			callee_name += ")";
            // after fix the function name, we try to get it again.
			callee = CodeGen::the_module->getFunction(callee_name);
		}

		// if we still cannot find the function, we could now throw a error.
		if (!callee)
			return CodeGen::LogErrorV((std::string("Unknown function referenced :") + callee_name).c_str());

        // here we found the callee!
		// check if the function argument count matchs.
		// some function could have varible arguments size when isVarArg is true.
		if (callee->arg_size() != args_v.size()  && !callee->isVarArg())
			return CodeGen::LogErrorV((std::string("Incorrect # arguments passed: ") + 
				std::to_string(args.size() + (is_member_func ? 1 : 0)) + " / " + std::to_string(callee->arg_size())).c_str());

		// call the function and save it to v
		llvm::Value* v = CodeGen::builder.CreateCall(callee, args_v);

        // since constructor returns void, the value shoud be the initized 'parent'
		v = is_constructor ? parent : v;

        // it the field is not done yet. continue.
		if (child != nullptr)
		{
			child->cmd = cmd;
			v = child->GenField(v);
		}

		return v;
	}


}