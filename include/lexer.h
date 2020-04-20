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

enum {
    OTHER_KEYWORDS
#define ENUM(NAME) K_##NAME,
    KEYWORDS(ENUM)
#define ENUM(NAME,_) NAME,
    OPERATORS(ENUM)
#undef ENUM
};


class lexer {
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
    static long size;
    static Token* token;
    static std::wstring string_val;
    static double number_val;

    static void LoadFile(const char* file);

    static wchar_t* Move();

    static void MoveLine();

    static bool IsCjk(const wchar_t t);

    static bool IsChar();

    static void Next();

    static void Find(const wchar_t start, const wchar_t end);

    static void Match(const int tk);

    static bool Check(const int t);

    static bool Check(const std::vector<int> t);

    static bool CheckType();


};


#endif
