#include <iostream>
#include <codecvt>
#include <fstream>
#include <sstream>

#include "lexer.h"


wchar_t* Lexer::src = nullptr;
wchar_t* Lexer::root = nullptr;
wchar_t Lexer::peek;
long Lexer::size = 0;
Lexer::Token* Lexer::token = nullptr;
std::wstring Lexer::string_val;
double Lexer::number_val = 0;

const char* Lexer::Token::Name(const int type)
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

void Lexer::LoadFile(const char* file) {
    std::wifstream wif(file);
    if (wif.fail()) Debugger::PrintErrorInfo(L"No such file or directory", false);
    wif.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
    std::wstringstream wss;
    wss << wif.rdbuf();
    size = std::size(wss.str()) + 1;
    root = src = _wcsdup((wss.str() + L"\n").c_str());
    Debugger::lines.push_back(src);
    Debugger::end = root + size;
}

wchar_t* Lexer::Move() {
    auto p = src;
    Debugger::ch++;
    if (*src == '\t') {
        Debugger::ch += 7;
        Debugger::tab = 1;
    }
    else if (*src > 128) Debugger::ch++;

    src++;
    return p;
}

void Lexer::MoveLine() {
    if (Debugger::skip_line) {
        while (*src != 0 && *src != '\n')src++;
        // line++;
        // ch = tab= chp = 0;
        // printf("[%c]", *src);
        // src++;
        // lines.push_back(src);
    }
    Debugger::skip_line = true;
}

bool Lexer::IsCjk(const wchar_t t) {
    return t >= L'\u2E80' && t <= L'\u2FD5'
        || t >= L'\u3190' && t <= L'\u319f'
        || t >= L'\u3400' && t <= L'\u4DBF'
        || t >= L'\u4E00' && t <= L'\u9FCC'
        || t >= L'\uF900' && t <= L'\uFAAD';
}

bool Lexer::IsChar() {
    return peek >= 'a' && peek <= 'z' || peek >= 'A' && peek <= 'Z';
}

void Lexer::Next() {
    Debugger::chp = Debugger::ch;
    wchar_t* last_pos;
    int hash;
    while ((peek = *src)) {
        Move();
        if ((src - root) > size) {
            size = src - root;
            token = nullptr;
            return;
        }
        if (peek == '\n') {
            Debugger::line++;
            Debugger::chp = Debugger::ch = 0;
            Debugger::lines.push_back(src);
            token = new Token(NewLine);
            return;
        }
        while (peek == ' ' || peek == '\t' || peek == '\n') {
            Debugger::chp = Debugger::ch;
            peek = *src;
            if (peek == '\n') {
                Debugger::line++;
                Debugger::ch = Debugger::tab = Debugger::chp = 0;
                Move();
                Debugger::lines.push_back(src);
                token = new Token(NewLine);
                return;
            }
            Move();
        }

        if (peek == '#') {
            MoveLine();
            Next();
            return;
        }
        else if (peek == '/')						// skip comments
        {
            if (*src == '/') {
                MoveLine();
                Next();
                return;
            }
            if (*src == '=') {
                Move();
                token = new Token(DivAgn);
            }
            else token = new Token('/');
            return;
        }
        else if (IsChar() || (peek == '_') || IsCjk(peek)) {
            last_pos = src - 1;
            hash = peek;
            string_val = L"";
            string_val += peek;
            while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || (*src
                == '_') || IsCjk(*src)) {
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

        else if (peek >= '0' && peek <= '9') {
            // parse number, four kinds: dec(123) hex(0x123) oct(017) Sci(10E3)
            number_val = 0;
            auto type = K_int;
            double value = peek - '0';
            if (value > 0) {
                auto decimal = 0;
                // dec, starts with [1-9]
                while ((*src >= '0' && *src <= '9') || *src == '.') {
                    if (*src == '.') {
                        type = K_double;
                        if (decimal != 0) {
                            Debugger::Alert(L"There are more than 1 dot in the number");
                            token = nullptr;
                            return;
                        }
                        Move();
                        decimal = 1;
                    }
                    else if (decimal != 0) value = value + (*Move() - '0') * pow(0.1, decimal++);
                    else value = value * 10 + (*Move() - '0');
                }
                if (*src == 'f' || *src == 'F') {
                    Move();
                    type = K_float;
                }
            }
            else // starts with number 0
            {
                if (*src == '.') {
                    type = K_double;
                    auto decimal = 1;
                    Move();
                    while ((*src >= '0' && *src <= '9') || *src == '.') {
                        if (*src == '.') {
                            Debugger::Alert(L"There are more than 1 dot in the number");
                            token = nullptr;
                            return;
                        }
                        else if (decimal != 0) value = value + (*Move() - '0') * pow(0.1, decimal++);
                        else value = value * 10 + (*Move() - '0');
                    }
                    if (*src == 'f' || *src == 'F') {
                        Move();
                        type = K_float;
                    }
                }
                if (*src == 'x' || *src == 'X') //hex
                {
                    Move();
                    peek = *src;
                    while ((peek >= '0' && peek <= '9') || (peek >= 'a' && peek <= 'f') || (peek >= 'A' && peek <= 'F')
                    ) {
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

        else if (peek == '"' || peek == '\'') {
            wchar_t value;
            string_val = L"";
            while (*src != 0 && *src != peek) {
                value = *Move();
                if (value == '\\') {
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
                if (peek == '"') {
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

        Debugger::Alert((std::wstringstream() << "invaild token: \"" << peek << "\" ").str());
        token = nullptr;
        return;
    }
}

void Lexer::Find(const wchar_t start, const wchar_t end) {
    auto i = 1;
    wchar_t t;
    while ((t = *Move())) {
        if (t == start)i++;
        else if (t == end)
            if (--i == 0) return;
    }
}

void Lexer::Match(const int tk) {

    if (token->type != tk) {
        if (token->type == NewLine) { Debugger::AlertNewline(); }
        Debugger::Alert(
            (std::wstringstream() << L"expected \"" << Token::Name(tk) << L"\" but got \"" << Token::Name(token->type)
                << L"\" instead").str());
        return;
    }
    Next();
}

bool Lexer::Check(const int t) {
    return token->type == t;
}

bool Lexer::Check(const std::vector<int> t) {
    return std::find(t.begin(), t.end(), token->type) != t.end();
}

bool Lexer::CheckType() {
    return Lexer::token->type >= K_int && Lexer::token->type <= K_double;
}


