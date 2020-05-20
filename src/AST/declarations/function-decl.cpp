#include "AST/declarations/function-decl.h"
#include "AST/statements/statements.h"
#include "codegen.h"

namespace parser {
	std::shared_ptr<FuncParam> FuncParam::Parse() {
		auto param = std::make_shared<FuncParam>();
		while (Lexer::token->type != ')') {
			param->size++;
			if (Lexer::CheckType()) {
				std::wstring t;
				t += static_cast<wchar_t>(Lexer::token->type);
				param->types.push_back(t);
				Lexer::Next();
				VERIFY
			}
			else if (Lexer::token->type == '.')		// Parse three dots '...' for variable argument.
			{
				Lexer::Next();
				VERIFY
					Lexer::Match('.');
				VERIFY
					Lexer::Match('.');
				VERIFY
					param->size--;
				param->is_var_arg = true;
				return param;
			}
			else {
				param->types.push_back(Lexer::string_val);
				Lexer::Match(Id);
				VERIFY
			}

			param->names.push_back(Lexer::string_val);
			Lexer::Match(Id);
			VERIFY
				if (Lexer::Check(',')) Lexer::Match(',');
			VERIFY
		}
		return param;
	}

	std::shared_ptr<FunctionDecl> FunctionDecl::Parse(const bool ext) {
		auto has_name = true;
		auto function = std::make_shared<FunctionDecl>();
		function->is_extern = ext;
		if (Lexer::Check(K_dfunc))function->differentiable = true;
		else if (Lexer::Check(K_kernal))function->kernal = true;
		else if (Lexer::Check(K_init)) {
			has_name = false;
			function->name = L"::init";
		}
		else if (Lexer::Check(K_delete))
		{
			has_name = false;
			function->name = L"::delete";
		}
		Lexer::Next();


		VERIFY
			if (has_name==false||Lexer::Check(Id)) {
				if (has_name) {
					function->name = Lexer::string_val;
					Lexer::Match(Id);
					VERIFY
				}
			}
            else {
				Debugger::Alert(L" function name expected");
            }
        if(Lexer::Check('<')) {
            if(function->name == L"::init"|| function->name == L"::delete") {

				Debugger::Alert(L"Constructor or Deconstructor cannot have generic type");
            }
			else {
				if (ext)Debugger::Alert(L" extern function cannot be generic");
				else function->generic = GenericParam::Parse();
			}
        }
		Lexer::Match('(');
		VERIFY
			function->args = FuncParam::Parse();
		Lexer::Match(')');
		VERIFY

			function->return_type = L'\0';
		if (Lexer::Check(':')) {
			Lexer::Next();
			VERIFY
				if (Lexer::CheckType()) {
					function->return_type = static_cast<wchar_t>(Lexer::token->type);
					Lexer::Next();
					VERIFY
				}
				else {
					function->return_type = Lexer::string_val;
					Lexer::Match(Id);
					VERIFY
				}

		}
		if (ext) {
			*Debugger::out << "[Parsed] Extern function declaration\n";
			return function;
		}

		Lexer::SkipNewlines();

		VERIFY
		* Debugger::out << "[Parsed] Function declaration\n";
        if(Lexer::Check(Arrow)) {
			Lexer::Next();
			function->statements = Statement::Parse();
        }
		else {

			Lexer::Match('{');
			VERIFY

			function->statements = Statements::Parse();
			Lexer::SkipNewlines();
			VERIFY

			Lexer::Match('}');
		}
		VERIFY
			* Debugger::out << "[Parsed] Function end\n";
		return function;
	}

    std::string FunctionDecl::GetInstanceName(GenericParam* g_param) {
        auto instance_name = header_name+"(";
		for (auto i = 0; i < generic->size; i++) {
			printf("what");
			instance_name += "#metadata";
		    instance_name+=(i < generic->size - 1 && args->size == 0) ? "" : ", ";
		}
        for(auto i = parent_type == nullptr ? 0 : 1;i<args->size;i++) {
            const auto g_id = args->generic_id[i];
           instance_name+=CodeGen::MangleStr(g_id >=0?g_param->names[g_id]:args->types[i])+(i == args->size - 1 ? "" : ", ");
        }
		instance_name += ")";
		return instance_name;
	}

    bool FunctionDecl::IsGenericArgument(std::wstring name) {
        for(auto i=0;i< args->size;i++)
			return(args->names[i]==name&&args->generic_id[i]>=0);
        return false;
	}

	void FunctionDecl::SetInternal(llvm::StructType* type) {
		parent_type = type;
	}

	void FunctionDecl::GenHeader() {
        // function arguments types.
		std::vector<llvm::Type*> arg_types;
		std::vector<std::wstring> arg_names;
		auto const is_member_function = parent_type != nullptr;
		header_name = CodeGen::MangleStr(name);

		//////////////////////////////////////////////////////////////////////////////
        /// State 1£¬ Check if is member function, and calculate header name.
        //////////////////////////////////////////////////////////////////////////////

        // if the function is a member function. we add the hidden pointer.
		if (is_member_function) {
			arg_types.push_back(parent_type->getPointerTo());
			arg_names.push_back(L"this");

			// added :: to Name specify name for constructor and destructor
			const auto class_name = parent_type->getStructName().str();
			if (name == L"::init")          header_name = class_name + "::" + class_name;
			else if (name == L"::delete")   header_name = class_name + "::~" + class_name;
			else header_name = class_name + "::" + header_name;
		}
		CodeGen::generic_functions_table[header_name].push_back(this);
		//////////////////////////////////////////////////////////////////////////////
        /// State 2£¬ Check if is generic function
        //////////////////////////////////////////////////////////////////////////////

        // Add generic type's metadata to argument
        if(generic!=nullptr) {

			// Generic Type arguments

            for(auto i=0;i<generic->size;i++) {
				arg_types.push_back(CodeGen::metadata_type->getPointerTo());
				arg_names.push_back(+L"$"+generic->names[i]);
				CodeGen::func_generic_table["$" + CodeGen::MangleStr(generic->names[i])] = nullptr;
            }
			// Generic return argument
			std::shared_ptr<GenericParam> parentGeneric = nullptr;
            if(parent_type)
                parentGeneric = CodeGen::types_table[parent_type->getStructName()]->generic;
            
            if (std::find(generic->names.begin(), generic->names.end(), return_type) != generic->names.end()
                ||(parentGeneric !=nullptr&& std::find(parentGeneric->names.begin(), parentGeneric->names.end(), return_type) != parentGeneric->names.end()) )
			{
				generic_return = arg_types.size();
				arg_types.push_back(CodeGen::void_ptr);
				arg_names.push_back(L"$return");
			}
        }

		//////////////////////////////////////////////////////////////////////////////
        /// State 3£¬ Add normal arguments
        //////////////////////////////////////////////////////////////////////////////

        // add to the arguments' type.
		for (auto i = 0; i < args->size; i++) {
			auto g_result = CodeGen::TestIfGenericType(CodeGen::MangleStr(args->types[i]));
			args->generic_id.push_back(g_result);
            if(g_result>0)  arg_types.push_back(CodeGen::void_ptr);
		    else            arg_types.push_back(CodeGen::GetTypeByName(args->types[i]));
		}

		////////////////////////////////////////////////////////////
        /// State 4£¬ Name the function
        ////////////////////////////////////////////////////////////

        // added arg types to name for overloading.
		full_name = header_name;
        full_name+= "(";
		for (int i = parent_type == nullptr ? 0 : 1,types_size = arg_types.size(); i < types_size; i++)
			full_name += (arg_types[i]==CodeGen::void_ptr?"void*": CodeGen::GetStructName(arg_types[i])) + 
			            (i == arg_types.size() - 1 ? "" : ", ");
		full_name += ")";



        ////////////////////////////////////////////////////////////
        /// State 5£¬ Generate the function.
		////////////////////////////////////////////////////////////


		CodeGen::functions_table[full_name] = this;
		// check if the fucntion already exist.
		auto the_function = CodeGen::the_module->getFunction(full_name);
		if (!the_function) {
            // create the function type.
            const auto return_ty = generic_return>=0 ? CodeGen::void_type: CodeGen::GetTypeByName(return_type);
			const auto func_type = llvm::FunctionType::get(return_ty, arg_types, args->is_var_arg);
            // create the function
			the_function = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, full_name,CodeGen::the_module.get());
            // add name for arguments.
			std::move(args->names.begin(), args->names.end(), std::back_inserter(arg_names));
			unsigned idx = 0;
			for (auto& arg : the_function->args())
				arg.setName(CodeGen::MangleStr(arg_names[idx++]));
		}
		else {
			// *Debugger::out << "function " << std::wstring(full_name.begin(), full_name.end()) << " already defined\n";
			CodeGen::LogErrorV((std::string("function ") + full_name + std::string(" already defined\n")).c_str());
			// CodeGen::current_function = nullptr;
		}
	}

   

	void FunctionDecl::Gen() {
		if (is_extern)return;
		
		auto function = CodeGen::the_module->getFunction(full_name);
		if (!function) {
			*Debugger::out << "function head not found\n";
			return;
		}

        //  create the basic block for the function.
		const auto basic_block = CodeGen::CreateBasicBlock(function, CodeGen::MangleStr(name) + "_entry");
		CodeGen::builder.SetInsertPoint(basic_block);
		CodeGen::current_function = this;
		for (auto& arg : function->args()) {
			// store the argument to argument table.
			// if argment is a struct passed by value. we store its alloca to local_fields.
			const auto arg_type_name = CodeGen::GetStructName(arg.getType());

            if(arg_type_name=="#metadata") {
				CodeGen::func_generic_table[arg.getName()] = &arg;
                continue;
            }
            if (CodeGen::GetCustomTypeCategory(arg_type_name) == ClassDecl::kStruct) {
                if (CodeGen::GetPtrDepth(&arg) == 0) {
                    const auto alloca = CodeGen::CreateEntryBlockAlloca(arg.getType(), arg.getName());
                    CodeGen::AlignStore(CodeGen::builder.CreateStore(&arg, alloca));
                    CodeGen::local_fields_table[arg.getName()] = alloca;
                    continue;
                }
            }
            CodeGen::local_fields_table[arg.getName()] = &arg;

		}

		// if 'this' have a base. then we create an alloca for the base.
		if (parent_type != nullptr) {
			const auto self_decl = CodeGen::types_table[parent_type->getStructName()];
			if (!self_decl->base_type_name.empty()) {
				const auto alloca = CodeGen::CreateEntryBlockAlloca(CodeGen::GetTypeByName(self_decl->base_type_name), "base", function);
				const auto base = CodeGen::FindMemberField(function->getArg(0), L"base");
				CodeGen::AlignStore(CodeGen::builder.CreateStore(base, alloca));
				CodeGen::local_fields_table["base"] = alloca;
			}
		}
        // generate the statements in the function.
		if (statements != nullptr)statements->Gen();
        // return void by default.
		if (function->getReturnType()->getTypeID() == llvm::Type::VoidTyID)CodeGen::builder.CreateRetVoid();
        // clear the local scope.
		CodeGen::local_fields_table.clear();
		CodeGen::current_function = nullptr;
		verifyFunction(*function);

	}

}
