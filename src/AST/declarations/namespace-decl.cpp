#include "AST/declarations/namespace-decl.h"
#include "AST/declarations/class-decl.h"
#include "AST/declarations/enum-decl.h"
using namespace frontend;
using namespace AST::decl;
std::shared_ptr<AST::decl::Namespace> AST::decl::Namespace::Parse() {
	Lexer::Next();
	auto instance = std::make_shared<Namespace>();
	instance->name = Lexer::string_val;
	Lexer::Match(Id);
	Lexer::SkipNewlines();
	Lexer::Match('{');
	while (!Lexer::Check('}')) {
		try {instance->ParseSingle();}
		catch (int error) {
			while (Debugger::error_occurred) {
				Debugger::error_occurred = false;
				Lexer::MoveLine();
				Lexer::Next();

			}
		}
		Lexer::SkipNewlines();
	}
	Lexer::Match('}');
	return instance;
}

void AST::decl::Namespace::ParseSingle() {
	switch (Lexer::token->type) {
	case NewLine: Lexer::Next(); break;
	case K_func:
	case K_dfunc:
	case K_kernal:      declarations.push_back(FunctionDecl::Parse()); break;
	case K_extern:      declarations.push_back(FunctionDecl::Parse(true)); break;
	case K_class:       declarations.push_back(ClassDecl::Parse(ClassDecl::kClass)); break;
	case K_interface:   declarations.push_back(ClassDecl::Parse(ClassDecl::kInterface)); break;
	case K_struct:      declarations.push_back(ClassDecl::Parse(ClassDecl::kStruct)); break;
	case K_enum:        declarations.push_back(EnumDecl::Parse()); break;
	default:
		Debugger::Error(L"Unknonw Token");
        return;
	}
}

std::shared_ptr<Name> Name::Parse(int ty) {
	auto instance = std::make_shared<Name>();
	instance->type = ty;
	instance->names.push_back(Lexer::string_val);
	Lexer::Match(Id);

	while (Lexer::Check('.')) {
		Lexer::Next();
		instance->names.push_back(Lexer::string_val);
		Lexer::Match(Id);
	}
	printf("Parse %s\n", instance->GetFunctionName().c_str());
	return instance;
}

std::string Name::GetFunctionName() {
	if (type == Name::kFunction && !names.empty())return names[names.size() - 1];
	return "";
}
std::string Name::GetClassName() {
	if (type == Name::kFunction && names.size() > 1)return names[names.size() - 2];
	if (type == kClass&& !names.empty())return names[names.size() - 1];
	return "";
}

std::string Name::GetNamespace() {
	std::string str;
    if(type==kFunction||type==kClass) {
		for (int i = 0, size = names.size(); i < size-1; i++)
			str += names[i] + (i == size - 2 ? "" : "::");
		return str;
    }
	for (int  i = 0, size = names.size(); i < size; i++)
		str += names[i] + (i == size - 1 ? "" : "::");
	return str;
}
std::string Name::GetFullName() {
	std::string str;
    for(int  i=0,size=names.size();i<size;i++)
		str += names[i] + (i == size - 1 ? "" : "::");
	return str;
}

std::string Name::GetFullNameWithoutFunc() {
	if (type == kFunction) {
		if (names.size() <= 1)return "ERROR";
		std::string ret;
		for (auto i = 0; i < names.size() - 1; i++)
			ret += names[i] + "::";
		return  ret;
	}
    else {
		std::string ret;
		for (auto i = 0; i < names.size(); i++)
			ret += names[i] + "::";
		return  ret;
    }
	return "ERROR";
}

void Name::Set(std::string s) {
	names.clear();
	names.push_back(s);
}

void Name::Verify() {
    
}

void Namespace::AnalysisHeader(std::shared_ptr<DFContext>) {}
void Namespace::Analysis(std::shared_ptr<DFContext>) {}

void Namespace::Gen(std::shared_ptr<DFContext> context) {
    
}
void Namespace::GenHeader(std::shared_ptr<DFContext> context) {
    
}


