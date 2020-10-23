#include "AST/declarations/class-decl.h"

#include <sstream>
#include "AST/declarations/enum-decl.h"
#include "AST/program.h"

namespace AST {
	using namespace decl;
	std::shared_ptr<ClassDecl> ClassDecl::Parse(int ty) {
		auto instance = std::make_shared<ClassDecl>();
		instance->category = ty;
		Lexer::Next();
		instance->name =Lexer::string_val;
		Lexer::Match(Id);
		if (Lexer::Check('<')) {
			instance->is_template = true;
			instance->generic = GenericParam::Parse();
		}
		if (Lexer::Check('(')) {
			Lexer::Next();
            const auto param = FuncParam::Parse();
			instance->one_line = true;
			instance->fields = param->names;
			instance->types = param->types;
			instance->functions.push_back(FunctionDecl::CreateInit(param));
			Lexer::Match(')');
		}

		if (Lexer::Check(':')) {
			Lexer::Next();
			instance->interfaces.push_back(std::make_shared<Type>(Lexer::string_val));
			Lexer::Next();
			while (Lexer::Check(',')) {
				Lexer::Next();
				instance->interfaces.push_back(std::make_shared<Type>(Lexer::string_val));
				Lexer::Match(Id);
			}
		}
        if(instance->one_line) {
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
				instance->fields.push_back(Lexer::string_val);
				Lexer::Match(Id);
				Lexer::Match(':');
				instance->types.push_back(Type::Match());
				Lexer::MatchSemicolon();
				Lexer::SkipNewlines();
			}
		}
		Lexer::Match('}');
		return instance;
	}

	void ClassDecl::Instantiate(std::shared_ptr<DFContext> ctx,std::shared_ptr<GenericParam> param) {
		const auto instance = new ClassDecl();
		instance->fields = fields;
		instance->is_template = false;
		instance->category = category;
		instance->name = name;
		instance->base_type_name = base_type_name;
		instance->interfaces = interfaces;
		instance->constructor = constructor;
		instance->generic = generic;
		instance->destructor = destructor;
		for (const auto& i : types)instance->types.push_back(i);
		for (int i = 0, size = instance->types.size(); i < size; ++i) {
			auto pos = std::find(generic->names.begin(), generic->names.end(), instance->types[i]->str);
			if (pos != generic->names.end())
				instance->types[i] = std::make_shared<Type>(param->names[i]);//TODO
		}
        const auto posfix = param->ToString();
		const auto full_name = name + posfix;
		auto the_struct = ctx->module->getTypeByName(full_name);
		if (the_struct) {
			Debugger::ErrorV((std::string("Type ") + full_name + " already defined").c_str(), line, ch);
			return;
		}

		the_struct = llvm::StructType::create(ctx->context, full_name);
		ctx->types_table[full_name] = instance;
		instance->full_name = full_name;
		instance->Gen(ctx);
        for(auto i=0;i<functions.size();i++) {
			instance->functions.push_back(std::make_shared<FunctionDecl>(functions[i]));
        }

		for (auto& function : instance->functions) {
			function->SetInternal(the_struct);
			function->PassGeneric(param,generic);
			function->GenHeader(ctx);
			ctx->program->late_gen.push_back(function);
		}
	}

	void ClassDecl::GenHeader(std::shared_ptr<DFContext> ctx) {
		full_name = name;
		auto the_struct = ctx->module->getTypeByName(full_name);
		if (!the_struct) {
            if(is_template) {
				ctx->template_types_table[full_name] = this;
                return;
            }
		    the_struct = llvm::StructType::create(ctx->context, name);
		}
		else {
			*Debugger::out << "Type " << name.c_str() << " already defined" << std::endl;
			return;
		}
		ctx->types_table[the_struct->getName().str()] = this;
		for (auto& function : functions) {
			function->SetInternal(the_struct);
			ctx->program->declarations.push_back(function);
		}

	}



    void ClassDecl::Gen(std::shared_ptr<DFContext> ctx) {
       
		if (is_template)return;
		auto the_struct = ctx->module->getTypeByName(full_name);
        if(!the_struct) {
			Debugger::ErrorV((std::string("type") + full_name + " is not defined").c_str(), line, ch);
            return;
        }
		std::vector<llvm::Type*> field_tys;

		if (!interfaces.empty()) {
			llvm::Type* base_type = nullptr;
			for (auto interface : interfaces) {
				auto mangled_interface_name =interface->str;
				if (ctx->IsCustomType(mangled_interface_name)) {
					const auto decl = ctx->types_table[mangled_interface_name];
					if (!decl->category == kInterface) {
						if (base_type == nullptr) {
							base_type = ctx->module->getTypeByName(interface->str);
							base_type_name = interface;
						}
						else Debugger::ErrorV("Inherit multiple classes is not allowed",line,ch);
					}
				}
				else  Debugger::ErrorV((Lexer::Str2W(mangled_interface_name) + L" is not defined"),line,ch);
			}
			if (base_type != nullptr) {

				fields.insert(fields.begin(), "base");
				field_tys.insert(field_tys.begin(), base_type);
			}
		}

		for (const auto& type : types)field_tys.push_back(ctx->GetType(type));
		the_struct->setBody(field_tys);
	}
}
