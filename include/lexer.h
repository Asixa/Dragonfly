#ifndef LEXER
#define LEXER
#include <iostream>
#include <sstream>
#include <fstream>
#include <codecvt>
#include "keywords.h"
#include "debug.h"

#include <vector>


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
	static std::wstring string_val;
	static double number_val;

	static void CheckUtf8(const char* file)
	{
		auto i = 0;
		const auto fp = fopen(file, "rb");

		if (fp != nullptr)
		{
			while (i++ <= 3)
			{
				const auto c = fgetc(fp);
				if (c == 239) printf("UTF8  file");
				else printf("ANSI File");
			}
			fclose(fp);
		}
	}
	static void LoadFile(const char* file)
	{
		std::wifstream wif(file);
		if (wif.fail()) PrintErrorInfo(L"No such file or directory", false);
		wif.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
		std::wstringstream wss;
		wss << wif.rdbuf();
		size = std::size(wss.str()) + 1;
		root = src = _wcsdup((wss.str() + L"\n").c_str());
		lines.push_back(src);
		end = root + size;
	}
	 static wchar_t* Move()
	 {
		auto p = src;
	 	ch++;
		if (*src == '\t') { ch += 7; tab = 1; }
		else if (*src > 128)ch++;
		
		src++;
	 	return p;
	}
	static void MoveLine()
	{
		if (skipline) {
			while (*src != 0 && *src != '\n')src++;
			// line++;
			// ch = tab= chp = 0;
			// printf("[%c]", *src);
			// src++;
			// lines.push_back(src);
		}
		skipline = true;
	}
	static void Next()
	{
		chp = ch;
		wchar_t* last_pos;
		int hash;
		while ((peek = *src))
		{
			Move();
			if ((src - root) > size) {
				size = src - root;
				token = nullptr;
				return;
			}
			if (peek == '\n')
			{
				line++; chp = ch = 0;
				lines.push_back(src);
				token = new Token(NewLine);
				return;
			}
			while (peek == ' ' || peek == '\t' || peek == '\n')
			{
				chp = ch;
				peek = *src;
				if (peek == '\n')
				{
					line++;
					ch = tab = chp = 0;
					Move();
					lines.push_back(src);
					token = new Token(NewLine);
					return;
				}
				Move();
			}

			if (peek == '#') { MoveLine(); Next();return; }
			else if (peek == '/')						// skip comments
			{
				if (*src == '/') { MoveLine(); Next(); return; }
				if (*src == '=')
				{
					Move(); token = new Token(DivAgn);
				}
				else token = new Token('/');
				return;
			}
			else if (CR(peek, 'a', 'z') || CR(peek, 'A', 'Z') || (peek == '_') || CHINESE(peek))
			{
				last_pos = src - 1;
				hash = peek;
				string_val = L"";
				string_val += peek;
				while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || (*src == '_') || CHINESE(*src))
				{
					hash = hash * 147 + *src;
					string_val += *src;
					Move();
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
							if (decimal != 0) {
								ALERT(L"There are more than 1 dot in the number")
									token = nullptr;
								return;
							}
							Move();
							decimal = 1;
						}
						else if (decimal != 0) value = value + (*Move() - '0') * pow(0.1, decimal++);
						else value = value * 10 + (*Move() - '0');
					}
					if (*src == 'f' || *src == 'F')
					{
						Move();
						type = K_float;
					}
				}
				else // starts with number 0
				{
					if (*src == '.')
					{
						type = K_double;
						auto decimal = 1;
						Move();
						while ((*src >= '0' && *src <= '9') || *src == '.') {
							if (*src == '.') {
								ALERT(L"There are more than 1 dot in the number")
									token = nullptr;
								return;
							}
							else if (decimal != 0) value = value + (*Move() - '0') * pow(0.1, decimal++);
							else value = value * 10 + (*Move() - '0');
						}
						if (*src == 'f' || *src == 'F')
						{
							Move();
							type = K_float;
						}
					}
					if (*src == 'x' || *src == 'X') //hex
					{
						Move();
						peek = *src;
						while ((peek >= '0' && peek <= '9') || (peek >= 'a' && peek <= 'f') || (peek >= 'A' && peek <= 'F'))
						{
							value = value * 16 + (peek & 15) + (peek >= 'A' ? 9 : 0);
							Move();
							peek = *src;
						}
					}
					else // oct
					{
						while (*src >= '0' && *src <= '7')
							value = value * 8 + *Move() - '0';
					}
				}
				number_val = value;
				token = new Token(Num, type);
				return;
			}

			else if (peek == '"' || peek == '\'')
			{
				wchar_t value;
				string_val = L"";
				while (*src != 0 && *src != peek)
				{
					value = *Move();
					if (value == '\\')
					{
						value = *Move();
						// escape character
						if (value == 'n')value = '\n';
						else if (value == 't')value = '\t';
						else if (value == '\\')value = '\\';
						else if (value == 'a')value = '\a';
						else if (value == 'b')value = '\b';
						else if (value == 'f')value = '\f';
						else if (value == 'r')value = '\r';
						else if (value == 'v')value = '\v';
						// else if (value == 'U'){}
						// else if (value == 'u'){}
					}
					if (peek == '"')
					{
						string_val += value;
					}
				}
				Move();
				token = new Token(Str);
				return;
			}

#define SYMBOL(a) else if (peek == a){token = new Token(a);return;}
#define U_SYMBOL(c,t)\
		else if (peek == c){														\
				if (*src == '='){Move();token = new Token(t);}						\
				else{token = new Token(c);}											\
				return;}

#define D_SYMBOL(c,t,ta)\
		else if (peek == c){														\
				if (*src == c){Move();token = new Token(t);}						\
				else if (*src == '=') {Move(); token = new Token(ta);}				\
				else{token = new Token(c);}return;}

#define T_SYMBOL(c,ta,tt,tta)\
			else if (peek == c) {													\
					if (*src == '=') { Move(); token = new Token(ta); }				\
					else if (*src == c) { Move();									\
						if (*src == '=') {Move(); token = new Token(tta);}			\
						else token = new Token(tt); }								\
					else token = new Token(c);										\
					return; }
			
				SPECIAL_OP
				SINGEL_OP(SYMBOL)
				ASSGIN_OP(U_SYMBOL)
				ASSGIN_OR_REPEAT_OP(D_SYMBOL)
				ASSGIN_AND_REPEAT_OP(T_SYMBOL)

				ALERT("invaild token: \"" << peek << "\" ")
				token = nullptr;
				return;
		}
	}

	inline void Find(const wchar_t start, const wchar_t end)
	{
		auto i = 1;
		wchar_t t;
		while ((t = *Move()))
		{
			if (t == start)i++;
			else if (t == end)
				if (--i == 0) return;
		}
	}

	inline void Match(const int tk) {

		if (token->type != tk) {
			if (token->type == NewLine) { ALERT_NEWLINE }
				ALERT("expected \"" << Token::Name(tk) << "\" but got \"" << Token::Name(token->type) << "\" instead")
			return;
		}
		Next();
	}
};

#endif