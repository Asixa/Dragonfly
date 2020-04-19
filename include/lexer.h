// Copyright 2019 The Dragonfly Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef LEXER_H
#define LEXER_H
#include <iostream>
#include <sstream>
#include <fstream>
#include <codecvt>
#include "keywords.h"
#include "debug.h"

#include <vector>



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
		if (wif.fail()) debugger::PrintErrorInfo(L"No such file or directory", false);
		wif.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
		std::wstringstream wss;
		wss << wif.rdbuf();
		size = std::size(wss.str()) + 1;
		root = src = _wcsdup((wss.str() + L"\n").c_str());
        debugger::lines.push_back(src);
		debugger::end = root + size;
	}
	 static wchar_t* Move()
	 {
		auto p = src;
		debugger::ch++;
		if (*src == '\t') { debugger::ch += 7;  debugger::tab = 1; }
		else if (*src > 128) debugger::ch++;
		
		src++;
	 	return p;
	}
	static void MoveLine()
	{
		if (debugger::skip_line) {
			while (*src != 0 && *src != '\n')src++;
			// line++;
			// ch = tab= chp = 0;
			// printf("[%c]", *src);
			// src++;
			// lines.push_back(src);
		}
		debugger::skip_line = true;
	}

    static bool IsCjk(const wchar_t t) {
		return t >= L'\u2E80' && t <=L'\u2FD5'
	    || t >= L'\u3190' && t <=L'\u319f'
	    || t >= L'\u3400' && t <=L'\u4DBF'
	    || t >= L'\u4E00' && t <=L'\u9FCC'
	    || t >= L'\uF900' && t <=L'\uFAAD';
	}

	static bool IsChar() {
		return peek >= 'a' && peek <= 'z' || peek >= 'A' && peek <= 'Z';
	}

	static void Next()
	{
		debugger::chp = debugger::ch;
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
				debugger::line++;  debugger::chp = debugger::ch = 0;
				debugger::lines.push_back(src);
				token = new Token(NewLine);
				return;
			}
			while (peek == ' ' || peek == '\t' || peek == '\n')
			{
				debugger::chp = debugger::ch;
				peek = *src;
				if (peek == '\n')
				{
					debugger::line++;
					debugger::ch = debugger::tab = debugger::chp = 0;
					Move();
					debugger::lines.push_back(src);
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
			else if (IsChar() || (peek == '_') || IsCjk(peek))
			{
				last_pos = src - 1;
				hash = peek;
				string_val = L"";
				string_val += peek;
				while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || (*src == '_') || IsCjk(*src))
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
								debugger::Alert(L"There are more than 1 dot in the number");
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
								debugger::Alert(L"There are more than 1 dot in the number");
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

				debugger::Alert((std::wstringstream() << "invaild token: \"" << peek << "\" ").str());
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
			if (token->type == NewLine) { debugger::AlertNewline(); }
			debugger::Alert((std::wstringstream() << L"expected \"" << Token::Name(tk) << L"\" but got \"" << Token::Name(token->type) << L"\" instead").str());
			return;
		}
		Next();
	}

	inline bool Check(const int t) {
		return token->type == t;
	}

	inline bool Check(const std::vector<int> t) {
		return std::find(t.begin(), t.end(), token->type) != t.end();
	}

	inline bool CheckType() {
		return lexer::token->type >= K_int && lexer::token->type <= K_double;
	}

	inline const char* Token::Name(const int type)
	{
		if (type == Id)return "Identifier";
		if (type == NewLine) return "NewLine";
		if (type == Num) return "Num";
		if (type == Str) return "Str";
		if (type == 0) return "ERROR";
		// This part using micro to automaticly write converter for Operator and Keywords, see "keywords.h"
#define TOKEN(a)if(type==K_##a)return #a;
		KEYWORDS(TOKEN)
#define TOKEN(a,b)if(type==a)return b;
			OPERATORS(TOKEN)
#undef TOKEN						
			return (new std::string(1, static_cast<char>(type)))->c_str();	// some type is just a chat, like ';' then we just return itself in a string.
	}
};

#endif