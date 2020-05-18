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
		auto return_parent = false;
		auto is_member_func = parent != nullptr;
		
		llvm::Value* v = nullptr;

		if (left != nullptr)
			v = left->GenField(parent);

		auto callee_name = parent == nullptr
			? CodeGen::MangleStr(name)
			: CodeGen::GetValueStructName(parent) + "::" + CodeGen::MangleStr(name);

		// generate all the argument values;
		std::vector<llvm::Value*> args_v;
		// prepare the argv list

		printf("*************** %d\n", args.size());
		for (unsigned i = 0, e = args.size(); i != e; ++i) {
			auto v = args[i]->Gen();
			auto type_name = CodeGen::GetTypeStructName(v->getType());
			if ( CodeGen::types_table.find(type_name) != CodeGen::types_table.end()) {
				const auto decl = CodeGen::types_table[type_name];
				if (decl->type == ClassDecl::kStruct && CodeGen::GetValuePtrDepth(v) != 0) {
					v = CodeGen::builder.CreateLoad(v);

				}
			}
			args_v.push_back(v);
			if (!args_v.back())return CodeGen::LogErrorV("Incorrect # arguments passed with error");
		}



		// callee_name should now be the full function name, eg: A.bar
		auto callee = CodeGen::the_module->getFunction(callee_name);

		if (CodeGen::types_table.find(callee_name) != CodeGen::types_table.end()) {
			auto decl = CodeGen::types_table[callee_name];
            if(decl->type==ClassDecl::kClass)
			parent = CodeGen::Malloc(CodeGen::the_module->getTypeByName(callee_name));
            else {
				const auto the_function = CodeGen::builder.GetInsertBlock()->getParent();
				const auto alloca = CodeGen::CreateEntryBlockAlloca(the_function, CodeGen::the_module->getTypeByName(callee_name), callee_name);
				alloca->setAlignment(llvm::MaybeAlign(8));
				parent = alloca;
            }
			is_member_func = true;
			callee_name = callee_name + "::" + callee_name;
			return_parent = true;
		}

        if(!callee) {
			callee_name += "(";
            for(int i=0, argv_size = args_v.size();i< argv_size;i++)
				callee_name += CodeGen::GetTypeStructName(args_v[i]->getType())+(i==argv_size - 1?"":", ");
			callee_name += ")";
			callee = CodeGen::the_module->getFunction(callee_name);

        }

        

		// check if the function exist
		if (!callee)
			return CodeGen::LogErrorV((std::string("Unknown function referenced :") + callee_name).c_str());
		// check if the function argument count matchs, notes that member func pass an extra argument.
		// some function could have varible arguments size when isVarArg is true.
		if (callee->arg_size() != (args_v.size() + (is_member_func ? 1 : 0)) && !callee->isVarArg())
			return CodeGen::LogErrorV((std::string("Incorrect # arguments passed: " )+ std::to_string(args.size() + (is_member_func ? 1 : 0))+" / "+ std::to_string(callee->arg_size())).c_str());


		if (is_member_func) {
			const auto ptr_depth = CodeGen::GetValuePtrDepth(parent);
			auto this_arg = parent;
			if (ptr_depth == 2) // ptr_depth = 2 means it is X**, but we want X*
				this_arg = CodeGen::builder.CreateLoad(parent);
			
			args_v.insert(args_v.begin(),this_arg);
		}

		// call the function
		v = callee->getReturnType()->getTypeID() == llvm::Type::VoidTyID
			? CodeGen::builder.CreateCall(callee, args_v)
			: CodeGen::builder.CreateCall(callee, args_v, "calltmp");
		// for cases like A().a or A().B(), we need to continue the DFS.
		if (child != nullptr)
		{
			child->cmd = cmd;
			v = child->GenField(return_parent?parent:v);
			return_parent = false;
		}
	
		// and return the value of the whole nested field.

        
		return return_parent?parent:v;
	}


}
