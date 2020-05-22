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
			}
			else if (Lexer::token->type == '.')		// Parse three dots '...' for variable argument.
			{
				Lexer::Next();
				Lexer::Match('.');
				Lexer::Match('.');
				param->size--;
				param->is_var_arg = true;
				return param;
			}
			else {
				param->types.push_back(Lexer::string_val);
				Lexer::Match(Id);
			}

			param->names.push_back(Lexer::string_val);
			Lexer::Match(Id);
			if (Lexer::Check(',')) Lexer::Match(',');
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

		if (has_name == false || Lexer::Check(Id)) {
			if (has_name) {
				function->name = Lexer::string_val;
				Lexer::Match(Id);
			}
		}
		else {
			Debugger::Error(L" function name expected");
		}
		if (Lexer::Check('<')) {
			if (function->name == L"::init" || function->name == L"::delete") {
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

		function->return_type = L'\0';
		if (Lexer::Check(':')) {
			Lexer::Next();
			if (Lexer::CheckType()) {
				function->return_type = static_cast<wchar_t>(Lexer::token->type);
				Lexer::Next();
			}
			else {
				function->return_type = Lexer::string_val;
				Lexer::Match(Id);
			}

		}
		if (ext) {
			*Debugger::out << "[Parsed] Extern function declaration\n";
			return function;
		}

		Lexer::SkipNewlines();
		*Debugger::out << "[Parsed] Function declaration\n";
		if (Lexer::Check(Arrow)) {
			Lexer::Next();
			function->statements = Statement::Parse();
		}
		else {
			Lexer::Match('{');
			function->statements = Statements::Parse();
			Lexer::SkipNewlines();
			Lexer::Match('}');
		}
		*Debugger::out << "[Parsed] Function end\n";
		return function;
	}

	std::string FunctionDecl::GetInstanceName(GenericParam* g_param) {
		auto instance_name = header_name + "(";
		for (auto i = 0; i < generic->size; i++) { 
			instance_name += "$";
			instance_name += (i < generic->size - 1 && args->size == 0) ? "" : ",";
		}
		// printf("   %ws::%s  %d-size%d\n",name.c_str(), instance_name.c_str(), parent_type == nullptr, args->size);
		// for (auto i = parent_type == nullptr ? 0 : 1; i < args->size; i++) {
		for (auto i = 0; i < args->size; i++) {
			const auto g_id = args->generic_id[i];
			instance_name += CodeGen::MangleStr(g_id >= 0 ? g_param->names[g_id] : args->types[i]) + (i == args->size - 1 ? "" : ",");
		}
		instance_name += ")";
		return instance_name;
	}

	bool FunctionDecl::IsGenericArgument(std::wstring name) {
		for (auto i = 0; i < args->size; i++)
			return(args->names[i] == name && args->generic_id[i] >= 0);
		return false;
	}

	void FunctionDecl::SetInternal(llvm::StructType* type) {
		parent_type = type;
	}

	void FunctionDecl::GenHeader() {
		// function arguments types.
		std::vector<llvm::Type*> arg_types;
		auto const is_member_function = parent_type != nullptr;
		// if the function is a member function. we add the hidden pointer.
		if (is_member_function) {
			arg_types.push_back(parent_type->getPointerTo());
			args->names.insert(args->names.begin(), L"this");
		}
		// add to the arguments' type.
		for (auto i = 0; i < args->size; i++)
			arg_types.push_back(CodeGen::GetTypeByName(args->types[i]));


		// added :: if is a member function, specify name for constructor and destructor
		const auto class_name = is_member_function ? parent_type->getStructName().str() : "";
		if (name == L"::init")          full_name = class_name + "::" + class_name;
		else if (name == L"::delete")   full_name = class_name + "::~" + class_name;
		else if (is_member_function)    full_name = class_name + "::" + CodeGen::MangleStr(name);
		else                            full_name = CodeGen::MangleStr(name);

		// added arg types to name for overloading.
		full_name += "(";
		for (int i = parent_type == nullptr ? 0 : 1, types_size = arg_types.size(); i < types_size; i++)
			full_name += CodeGen::GetStructName(arg_types[i]) + (i == arg_types.size() - 1 ? "" : ", ");
		full_name += ")";

		// check if the fucntion already exist.
		auto the_function = CodeGen::the_module->getFunction(full_name);
		if (!the_function) {
			// create the function type.
			const auto func_type = llvm::FunctionType::get(CodeGen::GetTypeByName(return_type), arg_types, args->is_var_arg);
			// create the function
			the_function = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, full_name, CodeGen::the_module.get());
			// add name for arguments.
			unsigned idx = 0;
			for (auto& arg : the_function->args())
				arg.setName(CodeGen::MangleStr(args->names[idx++]));
		}
		else {
			// *Debugger::out << "function " << std::wstring(full_name.begin(), full_name.end()) << " already defined\n";
			Debugger::ErrorV((std::string("function ") + full_name + std::string(" already defined\n")).c_str(), line, ch);
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

		for (auto& arg : function->args()) {
			// store the argument to argument table.
			// if argment is a struct passed by value. we store its alloca to local_fields.
			const auto arg_type_name = CodeGen::GetStructName(arg.getType());
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
		verifyFunction(*function);

	}

}