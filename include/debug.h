
#ifndef DEBUGGER
#define DEBUGGER

#include "lexer.h"
#include "parser.h"
#include "keywords.h"
inline const char* Token::Name(const int type)
{
		if (type == Id)return "ID";
		if (type == NewLine) return "NewLine";
		if (type == Num) return "Num";
		if (type == Str) return "Str";
		if (type == 0) return "ERROR";
#define TOKEN(a)if(type==K_##a)return #a;
		KEYWORDS(TOKEN)
#define TOKEN(a,b)if(type==a)return b;
			OPERATORS(TOKEN)
#undef TOKEN
		return (new std::string(1, static_cast<char>(type)))->c_str();
}


inline void Factor::print() { printf("[Factor]"); }
inline void NumberConst::print()
{

	switch (type)
	{
	case K_int:printf("[%d]", static_cast<int>(value));return;
	case K_float:printf("[%f]", value); return;
	case K_double:printf("[%f]", value); return;
	default:printf("[%s]", Token::Name(type)); return;
	}
	
}
inline void String::print() { std::wcout << "[\"" << value << "\"]"; }
inline void Boolean::print() { printf("[%s]",value?"true":"false"); }
inline void Field::print() { std::wcout << "[" <<name << "]"; }
inline void FuncCall::print()
{
	std::wcout << "[CALL " <<func<<" ( ";
	for (auto i=0;i<args.size();i++)
	{
		args[i]->print();
		printf(",");
	}
	printf(" )]\n");
}

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

inline void PrintError(const std::wstring msg,bool show_code)
{
	std::wcout << msg << std::endl;
	system("Pause");
	exit(-1);
}
#endif
