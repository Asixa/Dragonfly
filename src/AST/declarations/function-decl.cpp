#include "AST/declarations/function-decl.h"
#include "AST/statements/statements.h"

#include <sstream>
#include "AST/program.h"
#include "llvm/IR/Verifier.h"
namespace AST {

	using namespace AST::decl;
	std::string FuncParam::ToString() {
		std::string str = "(";
        for(int i=0,size= fields.size();i<size;i++)
            if(std::string(1, fields[i]->name[0])!=BUILTIN_TAG)         //skip builtin arg
			    str += fields[i]->type->str+((i== size-1)?"":",");
		return str + ")";
	}

    // returns NAME<X,X>(X,X)
	std::string FunctionDecl::GetName() {
		return nested_name->GetFunctionName()+ 
			(generic_instance_info == nullptr ? "" : generic_instance_info->ToGenericString())+
			args->ToString();
	}

    FunctionDecl::FunctionDecl() { keyword = 0; return_type = std::make_shared<AST::Type>(); }

	FunctionDecl::FunctionDecl(std::shared_ptr < FunctionDecl>  copy) {
		*this = *copy;
		args = std::make_shared<FuncParam>(copy->args);
	}


#pragma region Parse
	std::shared_ptr<FuncParam> FuncParam::Parse() {
		auto param = std::make_shared<FuncParam>();
		while (Lexer::token->type != ')') {
			param->size++;
			if (Lexer::Check('.'))		// Parse three dots '...' for variable argument.
			{
				Lexer::Next();
				Lexer::Match('.');
				Lexer::Match('.');
				param->size--;
				param->is_var_arg = true;
				return param;
			}
			auto field_name = Lexer::string_val;
			Lexer::Match(Id);
			Lexer::Match(':');
			param->fields.push_back(std::make_shared<FieldDecl>(field_name, Type::Match()));
			if (Lexer::Check(',')) Lexer::Match(',');
		}
		return param;
	}

    std::shared_ptr<FunctionDecl> FunctionDecl::Parse(const bool ext) {
		auto has_name = true;
		auto function = std::make_shared<FunctionDecl>();
		function->nested_name = std::make_shared<NestedName>();
		function->nested_name->type = NestedName::kFunction;
  //       if(ext)
		// function->keyword = K_extern;
        function->keyword = Lexer::token->type;
 
		if (Lexer::Check(K_init)|| Lexer::Check(K_delete)) {
			has_name = false;
			function->nested_name->Set(Lexer::Check(K_init)?BUILTIN_TAG"init":BUILTIN_TAG"delete");
		}
		Lexer::Next();

		if (has_name == false || Lexer::Check(Id)) {
			if (has_name) 
				function->nested_name = NestedName::Parse(NestedName::kFunction);
		}
		else Debugger::Error(L" function name expected");
		
		if (Lexer::Check('<')) {
			function->is_generic_template=true;
			if (function->nested_name->GetFunctionName() == BUILTIN_TAG"init" || function->nested_name->GetFunctionName() == BUILTIN_TAG"delete") {
				Debugger::Error(L"Constructor or Deconstructor cannot have generic type");
			}
			else {
				if (ext)Debugger::Error(L" extern function cannot be generic");
				else function->generic = GenericParam::Parse();
			}
		}

		Lexer::Match('(');
		function->args = FuncParam::Parse();
		Lexer::Match(')');

		function->return_type->ty = 1;
		if (Lexer::Check(':')) {
			Lexer::Next();
			function->return_type = Type::Match();
		}
		if (ext) {
			*Debugger::out << "[Parsed] Extern function declaration\n";
			return function;
		}

		Lexer::SkipNewlines();
		*Debugger::out << "[Parsed] Function declaration\n";
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
		*Debugger::out << "[Parsed] Function end\n";
		return function;
	}

	// Called by ClassDecl::Parse
	std::shared_ptr<FunctionDecl> FunctionDecl::CreateInit(const std::shared_ptr<FuncParam>& init_field) {
		auto instance = std::make_shared<FunctionDecl>();
		instance->nested_name = std::make_shared<NestedName>();
		instance->nested_name->Set(JOINER_TAG"init");
		instance->nested_name->type = NestedName::kFunction;

		std::vector<std::shared_ptr<stmt::Statement>>statements;
		for (auto i = 0; i < init_field->size; i++)
			statements.push_back(std::make_shared<stmt::Empty>(
				std::make_shared<expr::Binary>(init_field->fields[i]->name, init_field->fields[i]->name, '=')));

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
    void FunctionDecl::InstantiateTemplate(std::shared_ptr<DFContext> ctx, std::shared_ptr<FieldList> param) {
		if (!generic)return;
		const auto func_instance_name = param->ToGenericString();

        // TODO check dupulication
		// const auto function = ctx->module->getFunction(full_name + func_instance_name);
		// if (function)return;
		const auto instance = std::make_shared<FunctionDecl>();
		*instance = *this;
		instance->args.swap(std::make_shared<FuncParam>(args)); //deep copy
		instance->PassGeneric(param);
		instance->is_generic_template = false;
		instance->AnalysisHeader(ctx);
		ctx->program->late_gen.push_back(instance);
	}
	void FunctionDecl::PassGeneric(std::shared_ptr<FieldList> val, std::shared_ptr<GenericParam> key) {
		if (val->fields.empty())return;
		if (key == nullptr)key = generic;
        if(!key)return;
		for (auto i = 0, size = args->size; i < size; ++i) {
				auto pos = std::find(key->typenames.begin(), key->typenames.end(), args->fields[i]->type->str);
				if (pos != key->typenames.end())
					args->fields[i]->type = val->fields[std::distance(key->typenames.begin(), pos)].type;
		}

        //Replace Return Type
		const auto pos = std::find(key->typenames.begin(), key->typenames.end(), return_type->str);
		if (pos != key->typenames.end())
			return_type = val->fields[std::distance(key->typenames.begin(), pos)].type;
	}
#pragma endregion

	void FunctionDecl::AnalysisHeader(std::shared_ptr<DFContext>ctx) {
		auto const is_member_function = parent != nullptr;

        //TODO here to check if parent is classdecl
	    const auto full_name = is_member_function? GetFullname():nested_name->GetFullName();

		if (is_generic_template) {
			for (auto i = 0; i < args->size; i++) {
				auto pos = std::find(generic->typenames.begin(), generic->typenames.end(), args->fields[i]->type->str);
				if (pos != generic->typenames.end())
					args->generic_id.push_back(std::distance(generic->typenames.begin(), pos));
				else  args->generic_id.push_back(-1);
			}
			ctx->template_function_table[full_name] = this;
			return;
		}
		ctx->functions_table[full_name] = std::shared_ptr<FunctionDecl>(this);
	}
	void FunctionDecl::Analysis(std::shared_ptr<DFContext>ctx) {
		auto const is_member_function = parent != nullptr;

		if (is_member_function) {       //TODO check if it is an extension function, if so, we find its parent class
			const auto class_name = nested_name->GetClassName();
			if (!class_name.empty()) {
				// extension = true;
				parent = ctx->types_table[class_name]->decl;
				return;
			}
		}

		// added $this in the argument list
		if (is_member_function) {
			args->fields.insert(args->fields.begin(), 
				std::make_shared<FieldDecl>(BUILTIN_TAG"this",std::static_pointer_cast<ClassDecl>(parent)->GetType()));
		}
	}

	void FunctionDecl::GenHeader(std::shared_ptr<DFContext> ctx) {
		// function llvm arguments' type.
		std::vector<llvm::Type*> llvm_arg_types;
		for (auto i = 0; i < args->size; i++)
			llvm_arg_types.push_back(args->fields[i]->type->ToLLVM(ctx));

	    // create the function type.
		const auto func_type = llvm::FunctionType::get(return_type->ToLLVM(ctx), llvm_arg_types, args->is_var_arg);
		// create the function
		auto the_function = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, GetFullname(), ctx->module.get());
		// add name for arguments.
		unsigned idx = 0;
		for (auto& arg : the_function->args())
			arg.setName(args->fields[idx++]->name);

	}




    void FunctionDecl::Gen(std::shared_ptr<DFContext> ctx) {
		if(keyword==K_extern|| is_generic_template)return;
   //      if(extension) {
			// extension = false;
			// GenHeader(ctx);
   //      }
	
		auto function = ctx->GetFunction(GetFullname());
		//  create the basic block for the function.
		const auto basic_block = ctx->CreateBasicBlock(function, nested_name->GetFunctionName() + "_entry");
		ctx->builder->SetInsertPoint(basic_block);
		ctx->llvm->CreateScope();
		for (auto& arg : function->args()) {
			// store the argument to argument table.
			// if argment is a struct passed by value. we store its alloca to local_fields.
			const auto arg_type_name = ctx->GetStructName(arg.getType());
			if (ctx->types_table[arg_type_name]->decl->category == ClassDecl::kStruct) {
				if (ctx->GetPtrDepth(&arg) == 0) {
					const auto alloca = ctx->CreateEntryBlockAlloca(arg.getType(), arg.getName());
					ctx->AlignStore(ctx->builder->CreateStore(&arg, alloca));
					ctx->llvm->AddField(arg.getName(),alloca);
					continue;
				}
			}
			ctx->llvm->AddField(arg.getName(),&arg);
		}

		// if 'this' have a base. then we create an alloca for the base.
		if (parent != nullptr) {
			const auto self_decl = std::static_pointer_cast<ClassDecl>(parent);
			if (!self_decl->base_type->empty()) {
				const auto alloca = ctx->CreateEntryBlockAlloca(self_decl->base_type->ToLLVM(ctx), BUILTIN_TAG"base", function);
				const auto base = ctx->llvm->GetMemberField(function->getArg(0), BUILTIN_TAG"base");
				ctx->AlignStore(ctx->builder->CreateStore(base, alloca));
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