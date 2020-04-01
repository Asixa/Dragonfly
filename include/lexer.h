#ifndef LEXER
#define LEXER

#include <iostream>
#include "symbol.h"


#define SYMBOL(a,b) else if (peek == a){token = new Token(b);return;}
#define D_SYMBOL(a,b,at,bt)\
else if (peek == a){if (*src == b){src++;token = new Token(bt);}\
else{token = new Token(at);}return;}
#define T_SYMBOL(a,b,c,at,bt,ct)\
else if (peek == a){if (*src == b){src++;token = new Token(bt);}\
else if (*src == c){src++;token = new Token(ct);}\
else{token = new Token(at);}return;}
#define KEYWORD(a,b)else if(!memcmp(a,last_pos,src-last_pos)&&strlen(a)==src-last_pos){token = new Token(b);return;}
#define DToken(a,b)else if(type==a)return b;
enum
{
	Num = 128,Str, Fun, Sys, Glo, Loc, Id,

	NewLine,
	Char, Else, Enum, If, Int, Return, Sizeof, While, Print,
	Func, Dfunc, Kernal, Let,

	// don't change the order below, it is important
	Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr,
	Add, Sub, Mul, Div, Mod, Inc, Dec, Brak,
};

namespace lexer
{
	class Token
	{
	public:
		int type, value;
		explicit Token(const int t, const int v = 0) :type(t), value(v) {}
		static const char* name(int type)
		{
			if (type == ' ')return "Space";
			DToken('~', " ~ ") DToken(';', " ; ") DToken('{', " { ") DToken('}', " } ")
				DToken('(', " ( ") DToken(')', " ) ") DToken('[', " [ ") DToken(']', " ] ")
				DToken(',', " , ") DToken(':', " : ") DToken('"', " STRING ")
				DToken(Num, " Number ") DToken(Fun, "  ") DToken(Sys, "  ") DToken(Glo, "  ")
				DToken(Loc, "  ")		DToken(Id, " ID ") DToken(Assign, " = ") DToken(Cond, "  ")
				DToken(Lor, "  ")		DToken(Lan, "  ") DToken(Or, " || ") DToken(Xor, "  ")
				DToken(And, " && ")		DToken(Eq, " == ") DToken(Ne, " != ") DToken(Lt, " < ")
				DToken(Gt, " > ")		DToken(Le, " <= ") DToken(Ge, " >= ") DToken(Shl, "  ")
				DToken(Shr, "  ")		DToken(Add, " + ") DToken(Sub, " - ") DToken(Mul, " * ")
				DToken(Div, " / ")		DToken(Mod, "  ") DToken(Inc, "  ") DToken(Dec, "  ")
				DToken(Brak, " break ")		DToken(NewLine, " newline ") DToken(Char, " char ")
				DToken(Else, " else ")		DToken(Enum, " enum ")		 DToken(If, " if ") DToken(Int, " int ")
				DToken(Return, " return ")	DToken(Sizeof, " sizeof ")   DToken(While, " while ")
				DToken(Print, " print ")	DToken(Func, " func ")		 DToken(Dfunc, " dfunc ") DToken(Kernal, " kernal ")
				DToken(Let, " let ")
		}

	};

	static char* src;
	static char* root;
	static char peek;
	static long size;
	static Token* token;
	static int line;
	static void LoadFile(const char* file)
	{
		FILE* f;
		fopen_s(&f, file, "r");
		fseek(f, 0, SEEK_END);
		size = ftell(f);
		fseek(f, 0, SEEK_SET);
		src = static_cast<char*>(malloc(size + 1));
		memset(src, 0, size + 1);
		fread(src, size, 1, f);
		root = src;
		fclose(f);
		// printf("---------\n%s\n---------\n", src);
	}

	static void Next()
	{
		char* last_pos;
		int hash;
	
		
		while ((peek = *src++))
		{
			// printf("%c", peek);
			
			if ((src - root) > size) {
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
				else
				{
					token = new Token(Div);
					return;
				}
			}
			else if ((peek >= 'a' && peek <= 'z') || (peek >= 'A' && peek <= 'Z') || (peek == '_'))
			{
				last_pos = src - 1;
				hash = peek;
				while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || (*src == '_'))
				{
					hash = hash * 147 + *src;
					src++;
				}

				// reserved keywords
				if (0);
				KEYWORD("if", If)
				KEYWORD("return", Return)
				KEYWORD("while", While)
				KEYWORD("let", Let)
				KEYWORD("func", Func)
				KEYWORD("dfunc", Dfunc)
				KEYWORD("kernal", Kernal)
				KEYWORD("print", Print)
					// look for existing identifier, linear search
					// current_id = symbols;
					// while (current_id[Token])
					// {
					// 	if (current_id[Hash] == hash && !memcmp((char*)current_id[Name], last_pos, src - last_pos))
					// 	{
					// 		//found one, return
					// 		token = current_id[Token];
					// 		return;
					// 	}
					// 	current_id = current_id + IdSize;
					// }
					// // store new ID
					// current_id[Name] = (int)last_pos;
					// current_id[Hash] = hash;
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
				// parse string literal, currently, the only supported escape
				// character is '\n', store the string literal into data.
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
				token = new Token(Str, value);
				return;
			}
			else if (peek == '!')  // parse '!='
			{
				if (*src == '=')
				{
					src++;
					token = new Token(Ne);
				}
				token = new Token('!');
				return;
			}
			// directly return the character as token;
			else if (peek == '~' || peek == ';' || peek == '{' || peek == '}' || peek == '('
				|| peek == ')' || peek == ']' || peek == ',' || peek == ':')
			{
				token = new Token(peek);
				return;
			}
			D_SYMBOL('=', '=', Assign, Eq)
			D_SYMBOL('+', '+', Add, Inc)
			D_SYMBOL('-', '-', Sub, Dec)
			T_SYMBOL('<', '=', '<', Lt, Le, Shl)
			T_SYMBOL('>', '=', '>', Gt, Ge, Shr)  // parse '>=', '>>' or '>'
			D_SYMBOL('|', '|',  Lor, Or )
			D_SYMBOL('&', '&', Lan, And) // parse '&' and '&&'
			SYMBOL('^', Xor)
			SYMBOL('%', Mod)
			SYMBOL('*', Mul)
			SYMBOL('[', Brak)
			SYMBOL('?', Cond)
		}
	}

	Token* Match(int tk) {
		if (token->type != tk) {
			printf("expected :%s,but got:%s\n", Token::name(tk), Token::name(token->type), 10, src);
			system("PAUSE");
			exit(-1);
		}
		const auto t = token;
		Next();
		return t;
	}
};

#endif