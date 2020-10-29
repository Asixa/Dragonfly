#include "AST/declarations/class-decl.h"

#include <sstream>
#include "AST/declarations/enum-decl.h"
#include "AST/program.h"

namespace AST {
	using namespace decl;

    std::string FieldList::ToGenericString() {
		std::string str = "<";
		for (int i = 0, size = fields.size(); i < size; i++)
			str += fields[i].type->ToString()+ ((i == size - 1) ? "" : ",");
		return str + ">";
    }
    // copy from
    ClassDecl::ClassDecl(ClassDecl* from) {
		fields = from->fields;
		is_template = from->is_template;
		category = from->category;
		name = from->name;
		base_type = from->base_type;
		interfaces = from->interfaces;
		constructor = from->constructor;
		generic = from->generic;
		destructor = from->destructor;
    }


#pragma region Parse
	std::shared_ptr<ClassDecl> ClassDecl::Parse(int ty) {
		auto instance = std::make_shared<ClassDecl>();
        auto singleline = false;
		instance->category = ty;
		Lexer::Next();
		instance->name =Lexer::string_val;
		Lexer::Match(Id);
		if (Lexer::Check('<')) {        //Check if is a template
			instance->is_template = true;
			instance->generic = GenericParam::Parse();
		}

        // check class A(a:X,b:X) grammar
		if (Lexer::Check('(')) {
			Lexer::Next();
            const auto param = FuncParam::Parse();
			singleline = true;
			instance->fields = param->fields;
			instance->functions.push_back(FunctionDecl::CreateInit(param));
			Lexer::Match(')');
		}

		if (Lexer::Check(':')) {
			Lexer::Next();
			instance->interfaces.push_back(Type::Match());
			Lexer::Next();
			while (Lexer::Check(',')) {
				Lexer::Next();
				instance->interfaces.push_back(Type::Match());
				Lexer::Match(Id);
			}
		}
        if(singleline) {
			Lexer::MatchSemicolon();
			return instance;
        }
		Lexer::SkipNewlines();
		Lexer::Match('{');

		while (true) {
			Lexer::SkipNewlines();
			if (Lexer::token->type == '}')break;
			switch (Lexer::token->type) {
			case K_delete: {
				auto func = FunctionDecl::Parse();
				if (instance->destructor == nullptr)instance->destructor = func;
				else Debugger::Error(L"Mutiple destructors is not allowed");
				instance->functions.push_back(func);
				break;
			}
			case K_init:
			case K_func:
			case K_dfunc:
			case K_kernal:      instance->functions.push_back(FunctionDecl::Parse());break;
			case K_class:       instance->declarations.push_back(ClassDecl::Parse(ClassDecl::kClass)); break;
			case K_interface:   instance->declarations.push_back(ClassDecl::Parse(ClassDecl::kInterface)); break;
			case K_struct:      instance->declarations.push_back(ClassDecl::Parse(ClassDecl::kStruct)); break;
			case K_enum:        instance->declarations.push_back(EnumDecl::Parse()); break;
			default:
				if (instance->category == kInterface)break;
				auto field_name=Lexer::string_val;
				Lexer::Match(Id);
				Lexer::Match(':');
				instance->fields.push_back(std::make_shared<FieldDecl>(field_name,Type::Match()));
				Lexer::MatchSemicolon();
				Lexer::SkipNewlines();
			}
		}
		Lexer::Match('}');
		return instance;
	}
#pragma endregion    Stage 1: Parsing
 
#pragma region  Analysis
	void ClassDecl::AnalysisHeader(std::shared_ptr<DFContext>ctx) {
		if (is_template)return;
        const auto full_name = GetFullname();

		// Check duplicated class
		if (ctx->ExistClass(full_name)) {
			Debugger::ErrorV((std::string("type") + full_name + " is not defined").c_str(), line, ch);
			return;
		}
		ctx->types_table[full_name] = GetType();
        const auto this_ptr = std::shared_ptr<ClassDecl>(this);
		// Add subfunction to global decalarations
		for (auto& function : functions) {
			function->parent= this_ptr;
			if (generic_info)function->PassGeneric(generic_info);
			ctx->program->declarations.push_back(function);
		}
	}


    void ClassDecl::Analysis(std::shared_ptr<DFContext>ctx) {
		if (!interfaces.empty()) {
            for(auto i=0;i<interfaces.size();i++) {
				auto inherit = interfaces[i];
                if(inherit->category== Type::Custom) {
					auto cast = std::static_pointer_cast<CustomType>(inherit);
					if (i <= 0) {
						base_type = cast;
						fields.insert(fields.begin(), std::make_shared<FieldDecl>("$base", base_type));
					}
                        
                    else if (cast->decl->category != kInterface)
                         Debugger::ErrorV("Multiple inherting is not allowed,it must be interface", line, ch);
                }
                else  Debugger::ErrorV("this Type is not inheritable", line, ch);
            }
		}
	}


    // Called at Analysis State
	void ClassDecl::InstantiateTemplate(std::shared_ptr<DFContext> context,std::shared_ptr<FieldList> replace_by) {

		const auto instance = std::shared_ptr<ClassDecl>(this);
		instance->generic_info = replace_by;
		// Here replace all Generic Types to Real Types, instance.fields will be change, also the generic functions
		for (int i = 0, size = fields.size(); i < size; ++i) {
			auto pos = std::find(generic->typenames.begin(), generic->typenames.end(), fields[i]->type->ToString());
			auto type = fields[i]->type;
			if (pos != generic->typenames.end()) type = replace_by->fields[i].type;
			instance->fields.push_back(std::make_shared<FieldDecl>(fields[i]->name, type));
		}

		const auto full_name = instance->GetFullname() + replace_by->ToGenericString();      //eg:  "NAMESPACE::CLASSNAME<int,float>"
		if (!context->ExistClass(full_name)) {
			Debugger::ErrorV((std::string("Type ") + full_name + " already defined").c_str(), line, ch);
			return;
		}
		context->program->declarations.push_back(instance);
		instance->AnalysisHeader(context);
	}

    std::shared_ptr<CustomType> ClassDecl::GetType() {
		return std::make_shared<CustomType>(std::shared_ptr<ClassDecl>(this));
    }

#pragma endregion Stage 2: Semantic Analysis

#pragma region  CodeGen
	void ClassDecl::GenHeader(std::shared_ptr<DFContext> ctx) {
        auto the_struct = llvm::StructType::create(ctx->context, GetFullname());
	}

	void ClassDecl::Gen(const std::shared_ptr<DFContext> ctx) {
		auto the_struct = ctx->module->getTypeByName(GetFullname());    //Get the LLVM::Struct
		std::vector<llvm::Type*> llvm_class_field_types;
		for (const auto& field: fields) llvm_class_field_types.push_back(field->type->ToLLVM(ctx));
		the_struct->setBody(llvm_class_field_types);
	}

	std::string ClassDecl::GetName() { return name+(generic_info==nullptr?"":generic_info->ToGenericString()); }

#pragma endregion  State 3: Code Generating
}
