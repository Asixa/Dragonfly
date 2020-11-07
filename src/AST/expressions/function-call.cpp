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

    std::shared_ptr<AST::Type> FuncCall::Analysis(const std::shared_ptr<DFContext>ctx) {
		return AnalysisField(ctx, nullptr);
	}
	// TODO:  test needed , I think it should be "parent = left->GenField(parent);" added fix arg_list
    std::shared_ptr<AST::Type> FuncCall::AnalysisField(std::shared_ptr<DFContext>ctx, std::shared_ptr<AST::Type>parent) {
		// the data structure of Field for exmaple: a.B().C().d is
		//  [a]
		//     \child
		//      [C()]            cannot be linked list:  a->B()->C()->d 
		//   left/  \child       because left recursion happens when parse nested funtion.
		//  [B()]     [d]
		// so here we DFS to generate previous nodes.
		
		if (left != nullptr) parent = left->AnalysisField(ctx, parent);
		// if is calling a member function, eg: a.bar(), a is any object of custom class or struct.
		// if true, parent is the value of a.
        is_member_func = parent != nullptr;
		callee_name = is_member_func ? parent->ToString() + JOINER_TAG + name : name;
		
		if (generic) {
			const auto template_class_decl = ctx->ast->GetClassTemplate(callee_name);
			const auto template_func_decl = ctx->ast->GetFuncTemplate(callee_name);
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
				Debugger::ErrorV(line, ch,"Template '{}' not defined", callee_name);
				return nullptr;
			}
		}
        //check if is a constructor
		if (ctx->ast->IsCustomType(callee_name)) {
            if(ctx->ast->GetCustomTypeCategory(callee_name) < ClassDecl::kInterface) {
				Debugger::ErrorV(line, ch,"Cannot call a constructor of a interface or basic type");
				return nullptr;
            }
			class_type = ctx->ast->GetClass(callee_name);
			// now the function is a construtor, which is a special type of member function.
			is_member_func = true;
			callee_name = callee_name + JOINER_TAG + "$init";
			is_constructor = true;
		}

        //get string of args, eg (1+1,true) should be (int,boolean)
		param_name += "(";
		for (int i = 0, argv_size = args.size(); i < argv_size; i++)
			param_name += args[i]->Analysis(ctx)->ToString() + (i == argv_size - 1 ? "" : ",");
		param_name += ")";


		func = ctx->ast->GetFunctionDecl(callee_name);
		if (!func) func = ctx->ast->GetFunctionDecl(callee_name + param_name);
		
        if(!func) {
			 Debugger::ErrorV(line, ch,"Unknown function referenced: {}", callee_name + param_name);
			 return nullptr;
        }
		// here we found the callee!
        // check if the function argument count matchs.
        // some function could have varible arguments size when IsVariableArgument() is true.
		if (!func->args->IsVariableArgument()&&func->args->content.size() != args.size() + (is_member_func ? 1 : 0) ) {
			Debugger::ErrorV(line, ch,"Incorrect # arguments passed: {} needed, but got {} instead", 
				    std::to_string(func->args->content.size()) ,std::to_string(args.size() + (is_member_func ? 1 : 0)));
			return nullptr;
		}
		return is_constructor?class_type:func->return_type;
	}


	llvm::Value* FuncCall::Gen(const std::shared_ptr<DFContext> context, bool is_var) {
		return GenField(context, nullptr);
	}

	llvm::Value* HiddenConvert(const std::shared_ptr<DFContext> ctx, llvm::Value* val, llvm::Type* target) {

		switch (val->getType()->getTypeID()) {

		case llvm::Type::IntegerTyID:

			if (target)
			{
				const auto val_ty = val->getType();
				if (target->getTypeID() == llvm::Type::IntegerTyID) {
					if (target->getIntegerBitWidth() > val_ty->getIntegerBitWidth())
						val = ctx->builder->CreateCast(llvm::Instruction::SExt, val, target);
					else if (target->getIntegerBitWidth() < val_ty->getIntegerBitWidth())
						val = ctx->builder->CreateCast(llvm::Instruction::Trunc, val, target);
				}
			}
			else if (val->getType()->getIntegerBitWidth() < 32)
				val = ctx->builder->CreateCast(llvm::Instruction::SExt, val, ctx->constant.int32);

			break;

		case llvm::Type::FloatTyID:
			if (target)
			{
				const auto val_ty = val->getType();
				if (target->getTypeID() == llvm::Type::DoubleTyID)
					val = ctx->builder->CreateCast(llvm::Instruction::SExt, val, target);
				else if (target->getTypeID() == llvm::Type::IntegerTyID) 
					val = ctx->builder->CreateCast(llvm::Instruction::FPToSI, val, target);
			}
			else  val = ctx->builder->CreateCast(llvm::Instruction::FPExt, val, ctx->constant.double_type);
			break;
		}
		return  val;

	}

    llvm::Value* FuncCall::GenField(std::shared_ptr<DFContext> ctx,llvm::Value* _this) {
		// TODO function call like a()()
		// true if nested function call like A().B(), then we left-DFS
		if (left != nullptr) _this = left->GenField(ctx,_this);

		auto callee = ctx->llvm->GetFunction(callee_name);
		if (!callee) {
			callee_name += param_name;
			callee = ctx->llvm->GetFunction(callee_name);
		}

		// Here we generate all the parameters for the Call, store the values into args_v
		// hidden pointer for member func is not added here. will added later.
		std::vector<llvm::Value*> args_v;
		for (unsigned i = 0, e = args.size(); i != e; ++i) {
			auto val = args[i]->Gen(ctx,false); 	// val is the llvm::Value* evaluation of current argument experssion.
            // hidden convert
			val = HiddenConvert(ctx,val, callee->isVarArg() ? nullptr : callee->getArg(i)->getType());
			if (ctx->llvm->GetCustomTypeCategory(val->getType()) == ClassDecl::kStruct)    // TODO check basic type 
				while (ctx->llvm->GetPtrDepth(val) != 0)
					val = ctx->builder->CreateLoad(val);
			if (!val)return Debugger::ErrorV(line, ch,"arguments passed with error");
			args_v.push_back(val);
		}

		//  if the func is a constructor, then it should pass a 'this' pointer to init.
        // class and struct use different ways to create. class stores on heap, struct stores on stack.
		if (is_constructor) {
			switch (class_type->decl->category) {
			case ClassDecl::kClass:  _this = ctx->llvm->Malloc(class_type->ToLLVM(ctx)->getPointerElementType()); break;
			case ClassDecl::kStruct: _this = ctx->llvm->CreateEntryBlockAlloca(class_type->ToLLVM(ctx), callee_name); break;
			case ClassDecl::kInterface: return Debugger::ErrorV(line, ch,"Cannot call a constructor of a interface type");
			default:  return Debugger::ErrorV(line, ch,"Cannot call a constructor of a basic type"); //here the category should be -1
			}
		}

		// now we add the hidden pointer to the args list if it is a member function. //TODO catch errors like pass a value
		if (is_member_func) {
			const auto ptr_depth = ctx->llvm->GetPtrDepth(_this);
			auto this_arg = _this;
			if (ptr_depth == 2) // ptr_depth = 2 means it is X**, but we want X*        // TODO this is to catch class this. X**. we also need to check struct.
				this_arg = ctx->builder->CreateLoad(_this);
			args_v.insert(args_v.begin(), this_arg);
		}

		// if we still cannot find the function, we could now throw a error.
		if (!callee)
			return Debugger::ErrorV(line, ch,"Unknown function referenced: {}",callee_name); // this should never happen since we have Semantic Analysis.
		
   //      while (ctx->llvm->GetPtrDepth(args_v[0])>1)                               // TODO this is a hack, should be removed
			// args_v[0] = ctx->builder->CreateLoad(args_v[0]);

        for(int i=0,size= callee->arg_size();i<size;i++) {                                      // TODO this is also a hack, but it works
            const auto want_ptr = ctx->llvm->GetPtrDepth(callee->getArg(i));
            while (ctx->llvm->GetPtrDepth(args_v[i])>want_ptr)
				args_v[i] = ctx->builder->CreateLoad(args_v[i]);
        }
		
		llvm::Value* value = ctx->builder->CreateCall(callee, args_v);  // call the function and save it to v
		if (is_constructor) value = _this;                              // since constructor returns void, the value shoud be the initized 'parent'
		if (child != nullptr) {                                         // it the field is not done yet. continue.
			child->is_ptr = is_ptr;
			value = child->GenField(ctx, value);
		}
		return value;
	}


}