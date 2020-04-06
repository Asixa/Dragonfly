#ifndef LEXER
#define LEXER

#include <iostream>
#include "symbol.h"
#include <sstream>
#include <fstream>
#include <codecvt>
#include "keywords.h"

void PrintError(std::wstring msg,bool show_code=true);

#define SYMBOL(a) else if (peek == a){token = new Token(a);return;}
#define U_SYMBOL(c,t)\
		else if (peek == c){														\
				if (*src == '='){src++;token = new Token(t);}						\
				else{token = new Token(c);}											\
				return;}

#define D_SYMBOL(c,t,ta)\
		else if (peek == c){														\
				if (*src == c){src++;token = new Token(t);}							\
				else if (*src == '=') {src++; token = new Token(ta);}				\
				else{token = new Token(c);}return;}

#define T_SYMBOL(c,ta,tt,tta)\
			else if (peek == c) {													\
					if (*src == '=') { src++; token = new Token(ta); }				\
					else if (*src == c) { src++;									\
						if (*src == '=') {src++; token = new Token(tta);}			\
						else token = new Token(tt); }								\
					else token = new Token(c);										\
					return; }

#define CR(c,a,b) c >= a && c <=b
enum
{
	OTHER_KEYWORDS
#define ENUM(NAME) K_##NAME,
	KEYWORDS(ENUM)
#define ENUM(NAME,_) NAME,
	OPERATORS(ENUM)
#undef ENUM
};

namespace lexer
{
	class Token
	{
	public:
		int type, value;
		explicit Token(const int t, const int v = 0) :type(t), value(v) {}
		static const char* Name(int type);
		const char* Name();
	};

	static wchar_t* src;
	static wchar_t* root;
	static wchar_t peek;
	static long size;
	static Token* token;
	static int line;
	
	static std::wstring string_val;
	static double number_val;

	
	static void LoadFile(const char* file)
	{
		std::wifstream wif(file);
		if (wif.fail()) PrintError(L"File doesn't exist\n",false);
		wif.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
		std::wstringstream wss;
		wss << wif.rdbuf();
		size = std::size(wss.str());
		root=src = _wcsdup(wss.str().c_str());
		// std::wcout<<L"---------\n" << src << L"\n---------\n";
	}

	static void Next()
	{
		wchar_t* last_pos;
		int hash;
		while ((peek = *src++))
		{
			// std::wcout << peek;
			// printf("[%wc,%d]",peek, peek);
			if ((src - root) > size) {
				size = src - root;
				token = nullptr;
				return;
			}

			while (peek == ' ' || peek == '\t' || peek == '\n')
			{
				peek = *src;
				if (peek == '\n')
				{
					++line;
					src++;
					token = new Token(NewLine);
					return;
				}
				src++;
			}

			if (peek == '#')							// skip preprocessor
				while (*src != 0 && *src != '\n')
					src++;
			else if (peek == '/')						// skip comments
			{
				if (*src == '/')
					while (*src != 0 && *src != '\n')
						++src;
				U_SYMBOL('/', '=', DivAgn)
			}
			else if (CR(peek,'a','z') || CR(peek, 'A', 'Z') || (peek == '_')|| CHINESE(peek))
			{
				last_pos = src - 1;
				hash = peek;
				string_val = L"";
				string_val += peek;
				while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || (*src == '_')|| CHINESE(*src))
				{
					hash = hash * 147 + *src;
					string_val += *src;
					src++;
				}
				// reserved keywords
				if constexpr (false);
				#define MATCH(a)else if(!wmemcmp(L#a,last_pos,src-last_pos)&&wcslen(L#a)==src-last_pos){token = new Token(K_##a);return;}
				KEYWORDS(MATCH)
				#undef MATCH
				token = new Token(Id);
				return;
			}

			else if (peek >= '0' && peek <= '9')
			{
				// parse number, four kinds: dec(123) hex(0x123) oct(017) Sci(10E3)
				number_val = 0;
				auto type = K_int;
				double value = peek - '0';
				if (value > 0)
				{
					auto decimal = 0;
					// dec, starts with [1-9]
					while ((*src >= '0' && *src <= '9') || *src == '.') {
						if (*src == '.')
						{
							type = K_double;
							if (decimal != 0) PrintError(L"There are more than 1 dot at%c\n");
							src++;
							decimal = 1;
						}
						else if (decimal != 0) value = value + (*src++ - '0') * pow(0.1, decimal++);
						else value = value * 10 + (*src++ - '0');
					}
					if(*src=='f'||*src=='F')
					{
						src++;
						type = K_float;
					}
				}
				else // starts with number 0
				{
					if(*src=='.')
					{
						type = K_double;
						auto decimal = 1;
						src++;
						while ((*src >= '0' && *src <= '9') || *src == '.') {
							if (*src == '.')PrintError(L"There are more than 1 dot at%c\n");
							else if (decimal != 0) value = value + (*src++ - '0') * pow(0.1, decimal++);
							else value = value * 10 + (*src++ - '0');
						}
						if (*src == 'f' || *src == 'F')
						{
							src++;
							type = K_float;
						}
					}
					if (*src == 'x' || *src == 'X') //hex
					{
						peek = *++src;
						while ((peek >= '0' && peek <= '9') || (peek >= 'a' && peek <= 'f') || (peek >= 'A' && peek <= 'F'))
						{
							value = value * 16 + (peek & 15) + (peek >= 'A' ? 9 : 0);
							peek = *++src;
						}
					}
					else // oct
					{
						while (*src >= '0' && *src <= '7')
							value = value * 8 + *src++ - '0';
					}
				}
				number_val = value;
				token = new Token(Num,type);
				return;
			}

			else if (peek == '"' || peek == '\'')
			{
				wchar_t value;
				string_val = L"";
				while (*src != 0 && *src != peek)
				{
					value = *src++;
					if (value == '\\')
					{
						value = *src++;
						// escape character
						if (value == 'n')value = '\n'; 	
						else if (value == 't')value = '\t'; 	
						else if (value == '\\')value = '\\';
						else if (value == 'a')value = '\a';
						else if (value == 'b')value = '\b';
						else if (value == 'f')value = '\f';
						else if (value == 'r')value = '\r';
						else if (value == 'v')value = '\v';
						// else if (value == 'U')
						// {
						// 	
						// }
						// else if (value == 'u')
						// {
						// 	
						// }
					}
					if (peek == '"')
					{
						string_val += value;
					}
				}
				src++;
				token = new Token(Str);
				return;
			}
				SPECIAL_OP
				SINGEL_OP(SYMBOL)
				ASSGIN_OP(U_SYMBOL)
				ASSGIN_OR_REPEAT_OP(D_SYMBOL)
				ASSGIN_AND_REPEAT_OP(T_SYMBOL)
		}
	}

	inline void Find(const wchar_t start, const wchar_t end)
	{
		auto i = 1;
		wchar_t t;
		while ((t=*src++))
		{
			if (t == start)i++;
			else if (t == end)
				if (--i == 0) return;
		}
		
	}
	
	Token* Match(const int tk) {
		if (token->type != tk) {
			std::wcout<<"expected \""<< Token::Name(tk) <<"\" but got \""<< Token::Name(token->type) <<"\" instead"<<std::endl;
			PrintError(L"");
		}
		const auto t = token;
		Next();
		return t;
	}
};

#endif