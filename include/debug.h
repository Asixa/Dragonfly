
#ifndef DEBUGGER
#define DEBUGGER

#include "lexer.h"
#include "parser.h"
#include "syntex.h"
inline const char* Token::Name(const int type)
{
		if (type == Id)return "ID";
		if (type == NewLine) return "NewLine";
		if (type == Num) return "Num";
#define TOKEN(a)if(type==K_##a)return #a;
		KEYWORDS(TOKEN)
#define TOKEN(a,b)if(type==a)return b;
			OPERATORS(TOKEN)
#undef TOKEN
		return (new std::string(1, static_cast<char>(type)))->c_str();
}

inline void Factor::print() { printf("[Factor]"); }
inline void NumberConst::print() { printf("[number]"); }

inline void Unary::print() { printf("<%s>", Token::Name(op)); expr->print(); }
inline void Binary::print() { printf("("); LHS->print(); printf(" %s ", Token::Name(op)); RHS->print(); printf(")"); }
inline void Ternary::print()
{
	printf("[");
	a->print();
	printf("?");
	b->print();
	printf(":");
	c->print();
	printf("]");
}
#endif
