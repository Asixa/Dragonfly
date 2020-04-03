#ifndef LEXER
#define LEXER

#include <iostream>
#include "symbol.h"
#include <sstream>
#include <fstream>
#include <codecvt>
#include "keywords.h"

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
#define CHINESE(a) CR(a,L'\u2E80',L'\u2FD5')|| CR(a,L'\u3190', L'\u319f')|| CR(a,L'\u3400', L'\u4DBF')||CR(a,L'\u4E00',L'\u9FCC')||CR(a,L'\uF900',L'\uFAAD')

namespace lexer
{
	class Token
	{
	public:
		int type, value;
		explicit Token(const int t, const int v = 0) :type(t), value(v) {}
		static const char* Name(int type);
	};

	static wchar_t* src;
	static wchar_t* root;
	static wchar_t peek;
	static long size;
	static Token* token;
	static int line;
	static void LoadFile(const char* file)
	{
		std::wifstream wif(file);
		if (wif.fail()) {
			std::wcout << L"File doesn't exist\n";
			system("Pause");
			exit(-1);
		}
		wif.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
		std::wstringstream wss;
		wss << wif.rdbuf();
		size = std::size(wss.str());
		root=src = _wcsdup(wss.str().c_str());
		std::wcout<<L"---------\n" << src << L"\n---------\n";
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
				while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || (*src == '_')|| CHINESE(*src))
				{
					hash = hash * 147 + *src;
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
				auto value = peek - '0';
				if (value > 0)
				{
					// dec, starts with [1-9]
					while (*src >= '0' && *src <= '9')
						value = value * 10 + *src++ - '0';
				}
				else // starts with number 0
				{
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
				token = new Token(Num, value);
				return;
			}

			else if (peek == '"' || peek == '\'')
			{
				int value;
				last_pos = Symbol::data;
				while (*src != 0 && *src != peek)
				{
					value = *src++;
					if (value == '\\')
					{
						value = *src++;
						if (value == 'n')
							value = '\n'; 	// escape character
					}
					if (peek == '"')
					{
						*Symbol::data++ = value;
					}
				}
				src++;
				value = reinterpret_cast<int>(last_pos);
				token = new Token(K_string, value);
				return;
			}
				SINGEL_OP(SYMBOL)
				ASSGIN_OP(U_SYMBOL)
				ASSGIN_OR_REPEAT_OP(D_SYMBOL)
				ASSGIN_AND_REPEAT_OP(T_SYMBOL)
		}
	}

	Token* Match(int tk) {
		if (token->type != tk) {
			printf("expected \"%ws\" but got \"%ws\" at \"%.5ws\"\n", Token::Name(tk), Token::Name(token->type), src);
			system("PAUSE");
			exit(-1);
		}
		const auto t = token;
		Next();
		return t;
	}
};

#endif