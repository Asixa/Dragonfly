
#ifndef DEBUGGER
#define DEBUGGER
#include <string>
#include <iostream>
#include <vector>

static bool error_existed;
static bool error_occurred;

#define ALERT_LAST_LINE chp=ch=--src-lines[--line]; lines.pop_back(); skipline=false;
	
#define ALERT(a)\
	error_occurred = true;\
	error_existed = true;\
	log_color=Red;\
	PrintErrorInfo(L"error");\
	std::wcout << a << std::endl;\
	PrintErrorPostfix();

#define WARN(a)\
	log_color=Yellow;\
	PrintErrorInfo(L"warning");\
	std::wcout << a << std::endl;\
	PrintErrorPostfix();

static int line, ch,chp;
static bool skipline=true;
static std::vector<wchar_t*>lines;
static int log_color;
enum Color { Darkblue = 1, Darkgreen, Darkteal, Darkred, Darkpink, Darkyellow, Gray, Darkgray, Blue, Green, Teal, Red, Pink, Yellow, White };
inline void SetColor(const int c);
namespace lexer { static wchar_t* end; static void MoveLine(); }
static void PrintErrorInfo(const std::wstring type, const bool showPosition=true) {
		if(showPosition)
			std::cout << "[" << line+1 << "," << ch << "]: ";
		SetColor(log_color);
		std::wcout << type<<L": ";
		SetColor(White);
	}
static void PrintErrorPostfix()
{
	
	auto pt = lines[line];
	std:std::wstring str;
	while (*pt != L'\n'&&pt<lexer::end)str += *pt++;

	
	SetColor(Darkteal);
	std::wcout << str << std::endl;
	SetColor(log_color);
	auto space = chp - 1;
	auto error_token = ch - chp - 1;


	space = space < 0 ?0: space;
	error_token = error_token < 0 ? 0 : error_token;
	std::cout << std::string(space, ' ') << "↑" << std::string(error_token, '`') << std::endl;
	SetColor(White);
}

#endif


#ifdef PARSER
#ifndef OUTPUT
#define OUTPUT
#include "parser.h"

inline const char* Token::Name(const int type)
{
		if (type == Id)return "Identifier";
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
inline void Field::print() { std::wcout << "[" <<names[0] << "]"; }
inline void FuncCall::print()
{
	std::wcout << "[CALL " <<names[0]<<" ( ";
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


#endif
#endif
