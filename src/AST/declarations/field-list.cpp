

#include "field-list.h"
#include "frontend/lexer.h"

namespace AST {
	using namespace decl;
	std::string FieldList::ToString() {
		std::string str;
        switch (type) {
        case GenericDecl:
			str = "<";
			for (int i = 0, size = content.size(); i < size; i++)
				str += content[i]->name + (i == size - 1 ? "" : ",");
			return str + ">";
        case GenericInstantiate:
			str = "<";
			for (int i = 0, size = content.size(); i < size; i++)
				str += content[i]->type->ToString() + (i == size - 1 ? "" : ",");
			return str + ">";
        case Arguments:
            str = "(";
			for (int i = 0, size = content.size(); i < size; i++)
				if (std::string(1, content[i]->name[0]) != BUILTIN_TAG&& content[i]->name!="this") {      //skip builtin arg
					auto part = content[i]->type->ToString();
				    str += part + (i == size - 1 || part.size()==0 ? "" : ",");
				}
			return str + ")";
        default: ;
        }
		return str;
	}

    FieldList::FieldList(std::shared_ptr<FieldList> copy) {
	    type = copy->type;
        for (auto field : copy->content) 
			content.push_back(std::make_shared<FieldDecl>(field));
	}

    FieldList::FieldList(std::vector<std::shared_ptr<AST::Type>>& type) {
		for (auto ty : type)
			content.push_back(std::make_shared<FieldDecl>("",ty));
	}
    //<T,V,X,Z,some,A>  stores in FieldDecl.name
	std::shared_ptr<FieldList> FieldList::ParseGenericDecl() {
		frontend::Lexer::Match('<');
		auto instance = std::make_shared<FieldList>();
		instance->type =GenericDecl;
		while (frontend::Lexer::token->type != '>') {
			auto type_variable = frontend::Lexer::string_val;
			frontend::Lexer::Match(Id);
			instance->content.push_back(std::make_shared<FieldDecl>(type_variable, nullptr));
			if (frontend::Lexer::Check(',')) frontend::Lexer::Match(',');
		}
		frontend::Lexer::Match('>');
		return instance;
	}

	//<int,float,float[],(A,B)> stores in FieldDecl.type
	std::shared_ptr<FieldList> FieldList::ParseGenericInstantiate(){
		frontend::Lexer::Match('<');
		auto instance = std::make_shared<FieldList>();
		instance->type = GenericInstantiate;
		while (frontend::Lexer::token->type != '>') {
			instance->content.push_back(std::make_shared<FieldDecl>("", Type::Match()));
			if (frontend::Lexer::Check(',')) frontend::Lexer::Match(',');
		}
		frontend::Lexer::Match('>');
		return instance;
	}
	// (a:int,b:float,...)
	// (a:int = 1,b:float=2.0) stores in both FieldDecl.name and FieldDecl.type
	std::shared_ptr<FieldList> FieldList::ParseArguments(const bool parse_var_arg, const bool parse_expr) {
		frontend::Lexer::Match('(');
		auto instance = std::make_shared<FieldList>();
		instance->type = Arguments;
		while (frontend::Lexer::token->type != ')') {
			if (parse_var_arg&&frontend::Lexer::Check('.'))		// Parse three dots '...' for variable argument.
			{
                frontend::Lexer::Next();
                frontend::Lexer::Match('.');
                frontend::Lexer::Match('.');
				instance->content.push_back(std::make_shared<FieldDecl>("...", nullptr));
				return instance;
			}
			auto field_name = frontend::Lexer::string_val;
            frontend::Lexer::Match(Id);
            frontend::Lexer::Match(':');
			instance->content.push_back(std::make_shared<FieldDecl>(field_name, Type::Match()));
			if (frontend::Lexer::Check(',')) frontend::Lexer::Match(',');
		}
		frontend::Lexer::Match(')');
		return instance;
	}

    int FieldList::FindByName(const std::string name) {
        for (int i=0,size = content.size();i<size;i++) {
			if (content[i]->name == name)
				return i;
        }
		return -1;
	}

    int FieldList::FindByType(const std::string type) {
		for (int i = 0, size = content.size(); i < size; i++) {
			if (content[i]->type->ToString() == type)
				return i;
		}
		return -1;
	}

    bool FieldList::IsVariableArgument() {
		return content.back()->name == "..." && type == Arguments;
	}
}
