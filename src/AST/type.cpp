#include "AST/Type.h"
#include "frontend/lexer.h"

std::shared_ptr<AST::Type> AST::Type::Match() {
    auto type=std::make_shared<AST::Type>();

	if (frontend::Lexer::IsBasicType()) 
		type = BasicType::Match();
	else if(frontend::Lexer::Check('(')) 
		type = Tuple::Match();
	else type = CustomType::Match();
	
	if (frontend::Lexer::Check('[')) 
		type= Tensor::Match(type);
	return type;

}

std::shared_ptr<AST::BasicType> AST::BasicType::Match() {
	auto type = std::make_shared<BasicType>();
	type->ty = frontend::Lexer::token->type;
	type->str = "";
	frontend::Lexer::Next();
	return type;
}

std::shared_ptr<AST::CustomType> AST::CustomType::Match() {
	auto type = std::make_shared<AST::CustomType>();
	type->ty = 0;
	type->str = frontend::Lexer::string_val;
	frontend::Lexer::Match(Id);
	while (frontend::Lexer::Check('.')) {
		frontend::Lexer::Next();
		type->str += "." + frontend::Lexer::string_val;
		frontend::Lexer::Match(Id);
	}
	if (frontend::Lexer::Check('<')) {
		frontend::Lexer::Next();
		type->str += "<";
		if (frontend::Lexer::Check(Id)) {
			frontend::Lexer::Next();
			type->str += frontend::Lexer::string_val;
			while (frontend::Lexer::Check(',')) {
				frontend::Lexer::Next();
				frontend::Lexer::Match(Id);
				type->str += "," + frontend::Lexer::string_val;
			}
		}
		frontend::Lexer::Match('>');
		type->str += ">";
	}
	return type;
}

std::shared_ptr<AST::Type> AST::Tuple::Match() {
	auto type = std::make_shared<AST::Tuple>();
	frontend::Lexer::Match('(');
	type->types.push_back(AST::Type::Match());
	if (frontend::Lexer::Check(')'))return type->types[0];
	while (frontend::Lexer::Check(',')) 
		type->types.push_back(AST::Type::Match());
	frontend::Lexer::Match(')');
	return std::static_pointer_cast<AST::Type>(type);
	}


std::shared_ptr<AST::Tensor> AST::Tensor::Match(std::shared_ptr<AST::Type> base) {
	auto type = std::make_shared<AST::Tensor>(base);

	frontend::Lexer::Match('[');
	if (frontend::Lexer::Check(Num)) {
		if (frontend::Lexer::token->value != K_int) {
			frontend::Debugger::Error(L"Value in side a array operator shoule be integer.");
			return nullptr;
		}
		type->array = frontend::Lexer::number_val;
		frontend::Lexer::Next();
	}
	else {
		type->array = -2;
	}
	frontend::Lexer::Match(']');
	return  type;
}
