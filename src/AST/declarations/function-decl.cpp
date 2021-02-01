#include "AST/declarations/function-decl.h"
#include "AST/statements/statements.h"

#include <sstream>
#include "AST/program.h"
#include "llvm/IR/Verifier.h"
#include "field-list.h"
namespace AST {

	using namespace AST::decl;

    // returns NAME<X,X>(X,X)
	std::string FunctionDecl::GetName() {
		return  nested_name->GetFunctionName() +
			(generic_instance_info == nullptr ? "" : generic_instance_info->ToString())+
			(is_generic_template?"":args->ToString());
	}

	FunctionDecl::FunctionDecl() { keyword = 0; isClass = false; }
	FunctionDecl::FunctionDecl(std::shared_ptr < FunctionDecl>  copy) {
		*this = *copy;
		args = std::make_shared<FieldList>(copy->args);
	}


#pragma region Parse

    std::shared_ptr<FunctionDecl> FunctionDecl::Parse(const bool ext) {
		auto has_name = true;
		auto function = std::make_shared<FunctionDecl>();
		function->nested_name = std::make_shared<NestedName>();
		function->nested_name->type = NestedName::kFunction;
        function->keyword = Lexer::token->type;
 
		if (Lexer::Check(K_init)|| Lexer::Check(K_delete)) {
			has_name = false;
			function->nested_name->Set(Lexer::Check(K_init)?BUILTIN_TAG"init":BUILTIN_TAG"delete");
		}
		if (Lexer::Check(K_operator)) {
			has_name = false;
			Lexer::Next();
            switch (Lexer::token->type) {
                
            }
			function->nested_name->Set(Lexer::Check(K_init) ? BUILTIN_TAG"init" : BUILTIN_TAG"delete");
		}
		Lexer::Next();

		if (has_name == false || Lexer::Check(Id)) {
			if (has_name) 
				function->nested_name = NestedName::Parse(NestedName::kFunction);
		}
		else Debugger::Error(" function name expected");
		
		if (Lexer::Check('<')) {
			function->is_generic_template=true;
			if (function->nested_name->GetFunctionName() == BUILTIN_TAG"init" || function->nested_name->GetFunctionName() == BUILTIN_TAG"delete") {
				Debugger::Error("Constructor or Deconstructor cannot have generic type");
			}
			else {
				if (ext)Debugger::Error(" extern function cannot be generic");
				else function->generic = FieldList::ParseGenericDecl();
			}
		}

		function->args = FieldList::ParseArguments(true,true);

		function->return_type = BasicType::Void;
		if (Lexer::Check(':')) {
			Lexer::Next();
			function->return_type = Type::Match();
		}
		if (ext) {
			return function;
		}

		Lexer::SkipNewlines();
		if (Lexer::Check(Arrow)) {
			Lexer::Next();
			function->statements = stmt::Statement::Parse();
		}
		else {
			Lexer::Match('{');
			function->statements = stmt::Statements::Parse();
			Lexer::SkipNewlines();
			Lexer::Match('}');
		}
		return function;
	}

	// Called by ClassDecl::Parse
	std::shared_ptr<FunctionDecl> FunctionDecl::CreateInit(const std::shared_ptr<FieldList>& init_field) {
		auto instance = std::make_shared<FunctionDecl>();
		instance->nested_name = std::make_shared<NestedName>();
		instance->nested_name->Set(JOINER_TAG"init");
		instance->nested_name->type = NestedName::kFunction;

		std::vector<std::shared_ptr<stmt::Statement>>statements;
		for (int i = 0,size= init_field->content.size(); i <size ; i++)
			statements.push_back(std::make_shared<stmt::Empty>(
				std::make_shared<expr::Binary>(init_field->content[i]->name, init_field->content[i]->name, '=')));

		// List to binary tree
		if (!statements.empty()) {
			instance->statements = statements[0];
			for (int i = 1, size = statements.size(); i < size; i++)
				instance->statements = std::make_shared<stmt::Statements>(instance->statements, statements[i]);
		}

		instance->args = init_field;
		instance->return_type = BasicType::Void;
		return instance;
	}
#pragma endregion Stage 1. Parsing


#pragma region Analysis_Template
	std::shared_ptr<FunctionDecl> FunctionDecl::InstantiateTemplate(std::shared_ptr<DFContext> ctx, std::shared_ptr<FieldList> param) {
		if (!generic)return nullptr;
		const auto func_instance_name = param->ToString();

        // TODO check dupulication
		// const auto function = ctx->module->getFunction(full_name + func_instance_name);
		// if (function)return;
		const auto instance = std::make_shared<FunctionDecl>();
		*instance = *this;
		instance->args.swap(std::make_shared<FieldList>(args)); //deep copy
		instance->PassGeneric(param);
		instance->is_generic_template = false;
		instance->AnalysisHeader(ctx);
		instance->generic_instance_info = param;
		ctx->program->declarations.push_back(instance);
		// Debugger::Debug("[Instantiate Generic Function]:{}", instance->GetFullname());
		ctx->ast->AddFunction(instance->GetFullname(), instance);
		return instance;
	}
	void FunctionDecl::PassGeneric(std::shared_ptr<FieldList> generic_instance, std::shared_ptr<FieldList> generic_decl) {
		if (generic_instance->content.empty())return;
		if (generic_decl == nullptr)generic_decl = generic;
        if(!generic_decl)return;
		for (int i = 0, size = args->content.size(); i < size; ++i) {
            const auto pos = generic_decl->FindByName(args->content[i]->type->ToString());
			if (pos != -1)
				args->content[i]->type = generic_instance->content[pos]->type;
		}
		const auto pos = generic_decl->FindByName(return_type->ToString());         //Replace Return Type
		if (pos != -1)
			return_type = generic_instance->content[pos]->type;
	}
#pragma endregion

	void FunctionDecl::AnalysisHeader(std::shared_ptr<DFContext>ctx) {
		auto const is_member_function = parent != nullptr;

        //TODO here to check if parent is classdecl
	    const auto full_name = is_member_function? GetFullname():nested_name->GetFullName();

		if (is_generic_template) {
			for (int i = 0,size= args->content.size(); i <size; i++) 
                args->content[i]->generic = generic->FindByName(args->content[i]->type->ToString());
			ctx->ast->AddFuncTemplate(full_name,std::shared_ptr<FunctionDecl>(this));
			return;
		}
		ctx->ast->AddFunction(full_name,std::shared_ptr<FunctionDecl>(this));
		// added this in the argument list
		if (is_member_function) {
			args->content.insert(args->content.begin(),
				std::make_shared<FieldDecl>("this", std::static_pointer_cast<ClassDecl>(parent)->GetType()));
		}
	}
	void FunctionDecl::Analysis(std::shared_ptr<DFContext>ctx) {
		auto const is_member_function = parent != nullptr;

		if (is_member_function) {       //TODO check if it is an extension function, if so, we find its parent class
			const auto class_name = nested_name->GetClassName();
			if (!class_name.empty()) {
				parent = ctx->ast->GetClassDecl(class_name);
				return;
			}
		}
		ctx->ast->CreateScope();
		for (auto& i : args->content) 
			ctx->ast->AddField(i->name, i->type);
		statements->Analysis(ctx);
		ctx->ast->EndScope();
	
	}

	void FunctionDecl::GenHeader(std::shared_ptr<DFContext> ctx) {
        if(is_generic_template)return;
		// function llvm arguments' type.
		std::vector<llvm::Type*> llvm_arg_types;
		for (auto& parameter : args->content) {
            if(parameter->name=="this" &&ctx->ast->GetClassDecl(parameter->type->ToString())->category== ClassDecl::kStruct)
				llvm_arg_types.push_back(parameter->type->ToLLVM(ctx)->getPointerTo());
			else llvm_arg_types.push_back(parameter->type->ToLLVM(ctx));
		}
			
	    // create the function type.
		const auto func_type = llvm::FunctionType::get(return_type->ToLLVM(ctx), llvm_arg_types, args->IsVariableArgument());
		// create the function
		auto the_function = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, GetFullname(), ctx->module.get());
		// add name for arguments.
		unsigned idx = 0;
		for (auto& arg : the_function->args())
			arg.setName(args->content[idx++]->name);
	}




    void FunctionDecl::Gen(std::shared_ptr<DFContext> ctx) {
		if(keyword==K_extern|| is_generic_template)return;

		auto function = ctx->llvm->GetFunction(GetFullname());
		//  create the basic block for the function.
		const auto basic_block = ctx->llvm->CreateBasicBlock(function, nested_name->GetFunctionName() + "_entry");
		ctx->builder->SetInsertPoint(basic_block);
		ctx->llvm->CreateScope();
		for (auto& arg : function->args()) {
			// store the argument to argument table.
			// if argment is a struct passed by value. we store its alloca to local_fields.
			const auto arg_type_name = ctx->llvm->GetStructName(arg.getType());
			if (ctx->ast->GetCustomTypeCategory(arg_type_name)== ClassDecl::kStruct) {
				if (ctx->llvm->GetPtrDepth(&arg) == 0) {
					const auto alloca = ctx->llvm->CreateEntryBlockAlloca(arg.getType(), arg.getName());
					ctx->llvm->AlignStore(ctx->builder->CreateStore(&arg, alloca));
					ctx->llvm->AddField(arg.getName(),alloca);
					continue;
				}
			}
			ctx->llvm->AddField(arg.getName(),&arg);
		}

		// if 'this' have a base. then we create an alloca for the base.
		if (parent != nullptr) {
			const auto self_decl = std::static_pointer_cast<ClassDecl>(parent);
			if (self_decl->base_type!=nullptr) {
				const auto alloca = ctx->llvm->CreateEntryBlockAlloca(self_decl->base_type->ToLLVM(ctx), BUILTIN_TAG"base", function);
				const auto base = ctx->llvm->GetMemberField(function->getArg(0), BUILTIN_TAG"base");
				ctx->llvm->AlignStore(ctx->builder->CreateStore(base, alloca));
				ctx->llvm->AddField(BUILTIN_TAG"base",alloca);
			}
		}
       
		ctx->current_function = this;
		// generate the statements in the function.
		if (statements != nullptr)statements->Gen(ctx);
		// return void by default.
		if (function->getReturnType()->getTypeID() == llvm::Type::VoidTyID)ctx->builder->CreateRetVoid();
		// clear the local scope.
		ctx->current_function = nullptr;
		// ctx->local_fields_table.clear();

		ctx->llvm->EndScope();
	    verifyFunction(*function);


	}

}