#include "AST/declarations/function-decl.h"
#include "AST/statements/statements.h"

#include <sstream>
#include "AST/program.h"
#include "llvm/IR/Verifier.h"
namespace AST {

	using namespace AST::decl;
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
			param->names.push_back(Lexer::string_val);
			Lexer::Match(Id);
			Lexer::Match(':');
			param->types.push_back(Type::Match());
			if (Lexer::Check(',')) Lexer::Match(',');
		}
		return param;
	}

	std::shared_ptr<FunctionDecl> FunctionDecl::Parse(const bool ext) {
		auto has_name = true;
		auto function = std::make_shared<FunctionDecl>();
		function->name = std::make_shared<Name>();
		function->name->type = Name::kFunction;
		function->is_extern = ext;
		if (Lexer::Check(K_dfunc))function->differentiable = true;
		else if (Lexer::Check(K_kernal))function->kernal = true;
		else if (Lexer::Check(K_init)) {
			has_name = false;
			function->name->Set("::init");
		}
		else if (Lexer::Check(K_delete))
		{
			has_name = false;
			function->name->Set("::delete");
		}
		Lexer::Next();

		if (has_name == false || Lexer::Check(Id)) {
			if (has_name) {
				function->name = Name::Parse(Name::kFunction);
			}
		}
		else {
			Debugger::Error(L" function name expected");
		}
		if (Lexer::Check('<')) {
			function->is_template = true;
			if (function->name->GetFunctionName() == "::init" || function->name->GetFunctionName() == "::delete") {
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


	FunctionDecl::FunctionDecl(std::shared_ptr < FunctionDecl>  copy) {
		*this = *copy;
		args = std::make_shared<FuncParam>(copy->args);
	}

    std::shared_ptr<FunctionDecl> FunctionDecl::CreateInit(std::shared_ptr<FuncParam> param) {
		auto instance = std::make_shared<FunctionDecl>();
		instance->name = std::make_shared<Name>();
		instance->name->Set("::init");
		instance->name->type = Name::kFunction;
		auto func_param = std::make_shared<FuncParam>();
		
		std::vector<std::shared_ptr<stmt::Statement>>statements;
        for(auto i=0;i<param->size;i++) {
			func_param->names.push_back("_"+param->names[i]);
			func_param->types.push_back(param->types[i]);
			statements.push_back(std::make_shared<stmt::Empty>(std::make_shared<expr::Binary>( param->names[i], func_param->names[i], '=')));
        }
		
		if (statements.size() > 0) {
			instance->statements = statements[0];
			for (auto i = 1; i < statements.size(); i++)
				instance->statements = std::make_shared<stmt::Statements>(instance->statements, statements[i]);
		}

		func_param->size = param->size;
		instance->args = func_param;
		instance->return_type->ty = 1;
		return instance;
	}


    void FunctionDecl::SetInternal(llvm::StructType* type) {
		parent_type = type;
	}
	void FunctionDecl::Instantiate(std::shared_ptr<DFContext> ctx,std::shared_ptr <GenericParam> param) {
		if (!generic)return;
		const auto func_instance_name = param->ToString();
		const auto function = ctx->module->getFunction(full_name + func_instance_name);
		if (function)return;
		const auto instance = std::make_shared<FunctionDecl>();
		*instance = *this;
		instance->args.swap(std::make_shared<FuncParam>(args));
		instance->PassGeneric(param);
		instance->is_template = false;
		instance->func_postfix = func_instance_name;
		instance->GenHeader(ctx);
		ctx->program->late_gen.push_back(instance);
	}
	void FunctionDecl::PassGeneric(std::shared_ptr<GenericParam> val, std::shared_ptr<GenericParam> key) {
		if (val == nullptr)return;
		if (key == nullptr)key = generic;
        if(!key)return;
		for (auto i = 0, size = args->size; i < size; ++i) {
				auto pos = std::find(key->names.begin(), key->names.end(), args->types[i]->str);
				if (pos != key->names.end())
					args->types[i] = std::make_shared<Type>(val->names[std::distance(key->names.begin(), pos)]);
		}
		const auto pos = std::find(key->names.begin(), key->names.end(), return_type->str);
		if (pos != key->names.end())
			return_type = std::make_shared<Type>(val->names[std::distance(key->names.begin(), pos)]);
	}

	void FunctionDecl::GenHeader(std::shared_ptr<DFContext> ctx) {
		std::vector<llvm::Type*> arg_types;
		auto const is_member_function = parent_type != nullptr;

        if(parent_type==nullptr) {
            const auto class_name = name->GetClassName();
            if(!class_name.empty()) {
				extension = true;
				parent_type = ctx->module->getTypeByName(class_name);
				return;
            }
        }

		// added :: if is a member function, specify name for constructor and destructor
		const auto class_name = is_member_function ? parent_type->getStructName().str() : "";
		if (name->GetFunctionName() == "::init")          full_name = class_name  + "::" + class_name;
		else if (name->GetFunctionName() == "::delete")   full_name = class_name  + "::~" + class_name;
		else if (is_member_function)    full_name = class_name + "::" + name->GetFunctionName();
		else                            full_name = name->GetFullName();
		full_name += func_postfix;
        if(is_template) {
            for(auto i=0;i<args->size;i++) {
				auto pos = std::find(generic->names.begin(), generic->names.end(), args->types[i]->str);
				if (pos != generic->names.end()) 
					args->generic_id.push_back(std::distance(generic->names.begin(), pos));
				else  args->generic_id.push_back(-1);
            }
			ctx->template_function_table[full_name] = this;
            return;
        }
	
		// function arguments types.

		// if the function is a member function. we add the hidden pointer.
		if (is_member_function) {
			arg_types.push_back(parent_type->getPointerTo());
			args->names.insert(args->names.begin(), "this");
		}
		// add to the arguments' type.
		for (auto i = 0; i < args->size; i++)
			arg_types.push_back(ctx->GetType(args->types[i]));

		
		// added arg types to name for overloading.
		full_name += "(";
		for (int i = parent_type == nullptr ? 0 : 1, types_size = arg_types.size(); i < types_size; i++)
			full_name += ctx->GetStructName(arg_types[i]) + (i == arg_types.size() - 1 ? "" : ",");
		full_name += ")";
		// check if the fucntion already exist.
		auto the_function = ctx->GetFunction(full_name);
		if (!the_function) {
			
			// create the function type.
			const auto func_type = llvm::FunctionType::get(ctx->GetType(return_type), arg_types, args->is_var_arg);
			// create the function
			the_function = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, full_name, ctx->module.get());
			// add name for arguments.
			unsigned idx = 0;
			for (auto& arg : the_function->args())
				arg.setName(args->names[idx++]);
		}
		else  Debugger::ErrorV((std::string("function ") + full_name + std::string(" already defined\n")).c_str(), Debugger::line, Debugger::ch);
		
	}




    void FunctionDecl::Gen(std::shared_ptr<DFContext> ctx) {
		if (is_extern)return;
        if(is_template)return;
        if(extension) {
			extension = false;
			GenHeader(ctx);
        }
		
		auto function = ctx->GetFunction(full_name);
		if (!function) {
			Debugger::ErrorV((std::string("function head not found: ") + full_name).c_str(), Debugger::line, Debugger::ch);
			return;
		}
		//  create the basic block for the function.
		const auto basic_block = ctx->CreateBasicBlock(function, name->GetFunctionName() + "_entry");
		ctx->builder->SetInsertPoint(basic_block);

		for (auto& arg : function->args()) {
			
			// store the argument to argument table.
			// if argment is a struct passed by value. we store its alloca to local_fields.
			const auto arg_type_name = ctx->GetStructName(arg.getType());
			if (ctx->GetCustomTypeCategory(arg_type_name) == ClassDecl::kStruct) {
				if (ctx->GetPtrDepth(&arg) == 0) {
					const auto alloca = ctx->CreateEntryBlockAlloca(arg.getType(), arg.getName());
					ctx->AlignStore(ctx->builder->CreateStore(&arg, alloca));
					ctx->local_fields_table[arg.getName()] = alloca;
					continue;
				}
			}
			ctx->local_fields_table[arg.getName()] = &arg;

		}

		// if 'this' have a base. then we create an alloca for the base.
		if (parent_type != nullptr) {
			const auto self_decl = ctx->types_table[parent_type->getStructName()];
			if (!self_decl->base_type_name->empty()) {
				const auto alloca = ctx->CreateEntryBlockAlloca(ctx->GetType(self_decl->base_type_name), "base", function);
				const auto base = ctx->FindMemberField(function->getArg(0), "base");
				ctx->AlignStore(ctx->builder->CreateStore(base, alloca));
				ctx->local_fields_table["base"] = alloca;
			}
		}
       
		ctx->current_function = this;
		// generate the statements in the function.
		if (statements != nullptr)statements->Gen(ctx);
		// return void by default.
		if (function->getReturnType()->getTypeID() == llvm::Type::VoidTyID)ctx->builder->CreateRetVoid();
		// clear the local scope.
		ctx->current_function = nullptr;
		ctx->local_fields_table.clear();
		verifyFunction(*function);

	}

}