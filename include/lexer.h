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

#include "keywords.h"
#include "debug.h"

#include <vector>
#include <string>


class Lexer {


public:

    class Token {
    public:
        int type, value;
        explicit Token(const int t, const int v = 0) : type(t), value(v) {}
        static const char* Name(int type);
    };

    static wchar_t* src;
    static wchar_t* root;
    static wchar_t peek;
	// last current token
    static long size;
	// the current token
    static Token* token;
	// last encountered string literals
    static std::wstring string_val;
	// last encountered number literals
    static double number_val;

    //Initialize the lexer with the given file.
    static void LoadFile(const char* file);
	//Lexer ignores current line.
    static wchar_t* Move();
	//Lexer ignores current line.
    static void MoveLine();
	//Check if given wchar is Chinese, Janpanese, Or Korean Character.
    static bool IsCjk(wchar_t t);
	//Check if current peek is 'a'~'z' or 'A'~'Z'
    static bool IsChar();

    //Calculate the next token
    static void Next();

    static void Find(wchar_t start, wchar_t end);

    static void Match(int tk);
    //Check if current token's type equal to the given type.
    static bool Check(const int t);
	//Check if current token's type belong to the given list of type.
    static bool Check(const std::vector<int> t);
	//Check if current token's type is basic type, eg: int double.
    static bool CheckType();
};


#endif
