#ifndef LEXER
#define LEXER

#include <iostream>
#include "symbol.h"


#define SYMBOL(a) else if (peek == a){token = new Token(a);return;}
#define U_SYMBOL(c,b,t)\
		else if (peek == c){if (*src == b){src++;token = new Token(t);}\
		else{token = new Token(c);}return;}

#define D_SYMBOL(c,t,ta)\
		else if (peek == c){if (*src == c){src++;token = new Token(t);}\
		else if (*src == '=') {src++; token = new Token(ta);}\
		else{token = new Token(c);}return;}
#define T_SYMBOL(c,ta,tt,tta)\
			else if (peek == c) {\
					if (*src == '=') { src++; token = new Token(ta); }\
					else if (*src == c) { src++; if (*src == '=') { src++; token = new Token(tta); } else token = new Token(tt); }\
					else token = new Token(c);\
					return; }

#define KEYWORD(a,b)else if(!memcmp(a,last_pos,src-last_pos)&&strlen(a)==src-last_pos){token = new Token(b);return;}
#define DToken(a,b)else if(type==a)return b;
enum
{
	Num = 128, Id, NewLine,Str,
	Int,Float,Long,Double,Number,Bool,
	True,False,
	If,Else,For,While,Switch,Repeat,
	Let,Var,Func,Dfunc,Kernal,
	Return,Continue,Break,Try,Catch,Throw,
	Import,Typedef,Extension,Operator,
	Struct,Class,Enum,Interface,
	Get,Set,Init,Deinit,
	Sizeof,Print,
	In,


	Or, And, Eq, Ne, Le, Ge, Shl, Shr,
	AddAgn, SubAgn, MulAgn, DivAgn, ModAgn,
	Inc, Dec,
	BAndAgn, BXORAgn, BORAgn, ShlAgn, ShrAgn,
};

namespace lexer
{
	class Token
	{
	public:
		int type, value;
		explicit Token(const int t, const int v = 0) :type(t), value(v) {}
		static const char* Name(int type);
	};

	inline const char* Token::Name(int type)
	{
		// if(type<128) printf("(%c)", type);
		if (type == ' ')return "Space";
		DToken(Id, "ID") DToken(NewLine, "newline") DToken(Str, "str")
			DToken(Int, "int") DToken(Float, "float") DToken(Long, "long") DToken(Double, "double") DToken(Number, "number") DToken(Bool, "bool")
			DToken(True, "true") DToken(False, "false")
			DToken(If, "if") DToken(Else, "else") DToken(For, "for") DToken(While, "while") DToken(Switch, "switch") DToken(Repeat, "repeat")
			DToken(Let, "let") DToken(Var, "var") DToken(Func, "func") DToken(Dfunc, "dfunc") DToken(Kernal, "kernal")
			DToken(Return, "return") DToken(Continue, "continue") DToken(Break, "break") DToken(Try, "try") DToken(Catch, "catch") DToken(Throw, "throw")
			DToken(Import, "import") DToken(Typedef, "typedef") DToken(Extension, "extension") DToken(Operator, "operator")
			DToken(Struct, "struct") DToken(Class, "class") DToken(Enum, "enum") DToken(Interface, "interface")
			DToken(Get, "get") DToken(Set, "set") DToken(Init, "init") DToken(Deinit, "deinit")
			DToken(Sizeof, "size") DToken(Print, "print")
			DToken(In, "in")
			DToken(Or, "||") DToken(And, "&&")
			DToken(Eq, "==") DToken(Ne, "!=") DToken(Le, "<=") DToken(Ge, ">=")
			DToken(Shl, "<<") DToken(Shr, ">>")
			DToken(Inc, "++") DToken(Dec, "--")
			DToken(AddAgn, "+=") DToken(SubAgn, "-=") DToken(MulAgn, "*=") DToken(DivAgn, "/=") DToken(ModAgn, "%=")
			DToken(BAndAgn, "&=") DToken(BXORAgn, "^=") DToken(BORAgn, "|=") DToken(ShlAgn, "<<=") DToken(ShrAgn, ">>=")

			DToken('~', "~") DToken(';', ";")
			DToken('{', "{") DToken('}', "}")
			DToken('(', "(") DToken(')', ")")
			DToken('[', "[") DToken(']', "]")
			DToken(',', ",") DToken('?', "?")
			DToken('.', ".") DToken(':', ":")
			DToken('^', "^") DToken('%', "%")
			DToken('*', "*") DToken('!', "!")
			DToken('>', ">") DToken('<', "<")
			DToken('&', "&") DToken('|', "|")
			DToken('=', "=")
			DToken('+', "+")DToken('-', "-")DToken('*', "*")DToken('/', "/")
			return "ERROR";
	}

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
				U_SYMBOL('/', '=', DivAgn)
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
				if constexpr (false);
				KEYWORD("int", Int) KEYWORD("long", Long) KEYWORD("float", Float) KEYWORD("double", Double) KEYWORD("number", Number) KEYWORD("bool", Bool)
				KEYWORD("true", True) KEYWORD("false", False)
				KEYWORD("if", If) KEYWORD("else", Else) KEYWORD("for", For) KEYWORD("while", While) KEYWORD("switch", Switch) KEYWORD("repeat", Repeat)
				KEYWORD("let", Let) KEYWORD("var", Var) KEYWORD("func", Func) KEYWORD("dfunc", Dfunc) KEYWORD("kernal", Kernal)
				KEYWORD("return", Return) KEYWORD("continue", Continue) KEYWORD("break", Break) KEYWORD("try", Try) KEYWORD("catch", Catch) KEYWORD("throw", Throw)
				KEYWORD("import", Import) KEYWORD("typedef", Typedef)KEYWORD("extension", Extension)KEYWORD("operator", Operator)
				KEYWORD("struct", Struct) KEYWORD("class", Class)KEYWORD("enum", Enum)KEYWORD("interface", Interface)
				KEYWORD("get", Get)KEYWORD("set", Set)KEYWORD("init", Init)KEYWORD("deinit", Deinit)
				KEYWORD("sizeof", Sizeof)KEYWORD("print", Print)
				KEYWORD("in", In)
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


				SYMBOL('~') SYMBOL(';')
				SYMBOL('{') SYMBOL('}')
				SYMBOL('(') SYMBOL(')')
				SYMBOL('[') SYMBOL(']')
				SYMBOL(',') SYMBOL('?')
				SYMBOL('.') SYMBOL(':')


			
				U_SYMBOL('=','=',Eq)
				U_SYMBOL('!','=',Ne)
				U_SYMBOL('*', '=', MulAgn) 
				U_SYMBOL('%', '=', ModAgn)
				U_SYMBOL('^','=',BXORAgn)
			
				D_SYMBOL('+', Inc,AddAgn)
				D_SYMBOL('-', Dec,SubAgn)
			
				D_SYMBOL('&', And,BAndAgn)
				D_SYMBOL('|', Or,BORAgn)
				T_SYMBOL('<',Le,Shl,ShlAgn)
				T_SYMBOL('>',Ge,Shr,ShrAgn)
			
		}
			

	}

	Token* Match(int tk) {
		if (token->type != tk) {
			printf("expected \"%s\" but got \"%s\" at \"%.5s\"\n", Token::Name(tk), Token::Name(token->type), src);
			system("PAUSE");
			exit(-1);
		}
		const auto t = token;
		Next();
		return t;
	}
};

#endif