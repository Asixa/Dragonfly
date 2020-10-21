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

#include "frontend/keywords.h"
#include "frontend/debug.h"

#include <vector>
#include <string>
namespace parser {
	struct Type {
	public:
		int ty = -1;
		int array = -1;
		std::string str="";
		explicit Type() :array(-1), ty(-1) { }
		explicit Type(const std::string s) :str(s), ty(0), array(-1) { }
		explicit Type(const int t) :ty(t), array(-1) {};
		bool empty() const {
			return ty == -1||(ty==0&&str.empty());
		}
		bool operator==(const Type& rhs)
		{
			return ty == rhs.ty&&str==rhs.str&&array==rhs.array;
		}
		
		Type& operator=(Type& copy)
			//    ^^^^  Notice the double &&
		{
			ty = copy.ty;
			array = copy.array;
			str = copy.str;
			return *this;
		}
	};
}

class Lexer {
public:

    class Token {
    public:
        // The type of the token
		int type;
		// The value used to store secondary type.
		// For example the bit length of number literals.
        int value;
        explicit Token(const int t, const int v = 0) : type(t), value(v) {}
        // Takes a token type, returns its name in string.
        static const char* Name(int type);
    };

	// Current position of source code memeory..
    static wchar_t* src;
	// The head of source code memeory.
    static wchar_t* root;
	// pointer to the last character of source code memeory.
	static wchar_t* end;
	// Current character.
    static wchar_t peek;
	// Length of the source code snippet.
    static long size;
	// The current token.
    static Token* token;
	// Last encountered string literals.
	static std::string string_val;
	static std::wstring wstring_val;
	// Last encountered number literals.
    static double number_val;

    //Initialize the lexer with the given file.
    static void LoadFile(const char* file);
	//Lexer ignores current line.
    static wchar_t* Move();
	//Lexer ignores current line.
    static void MoveLine();
	//Check if given wchar is Chinese, Janpanese, Or Korean Character.
    static bool IsCjk(wchar_t c);
	//Check if current peek is 'a'~'z' or 'A'~'Z'
    static bool IsChar();
    //Calculate the next token
	static void Next();

	static void NextOneToken();
	static std::string MangleStr(const std::wstring str);

    static std::wstring Str2W(const std::string& str);
	static parser::Type MatchType();


    static void Find(wchar_t start, wchar_t end);
	//If current token is in expected type, move to next token.
	//Other wise throw an error to debugger.
    static void Match(int ty);
    //Check if current token's type equal to the given type.
    static bool Check(const int ty);
	//Check if current token's type belong to the given list of type.
    static bool Check(const std::vector<int> tys);
	//Check if current token's type is basic type, eg: int double.
    static bool IsBasicType();

	static void MatchSemicolon();
	static void SkipNewlines();
};



#endif
