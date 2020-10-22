#include "AST/declarations/generic-param.h"


std::string parser::GenericParam::ToString() {
	std::string name = "<";
    for (auto i=0;i<size;i++) 
		name += names[i] + (i == size - 1 ? "" : ",");
	return name+">";
}

std::shared_ptr<parser::GenericParam> parser::GenericParam::Parse() {
	auto instance = std::make_shared<GenericParam>();
	Lexer::Match('<');
    if(Lexer::Check(Id)) {
		Lexer::Next();
		instance->names.push_back(Lexer::string_val);
		instance->size=1;
        while (Lexer::Check(',')) {
			Lexer::Next();
			Lexer::Match(Id);
			instance->names.push_back(Lexer::string_val);
			instance->size ++;
        }
    }
	Lexer::Match('>');
	return instance;
}
