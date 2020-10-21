﻿// Copyright 2019 The Dragonfly Authors. All Rights Reserved.
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

#ifndef DEBUG_H
#define DEBUG_H
#include <llvm/Bitcode/BitcodeWriter.h>

class Debugger {
public:
    static std::basic_ostream<wchar_t>* out;
	// collection of pointers to the beginning of each line of source code.
	static std::vector<wchar_t*> lines;
    static bool is_std_out;
    // Once called ALERT, error_existed will be true and will not be changed, and the compiler won't generate IR.
    static bool error_existed;
	// Once called ALERT, error_occurred will be true, and all AST parsing will stop immediately,
    static bool error_occurred;			
    // error_occurred will be false once lexer moved to the next line.
    // only_tokenize is used for testing lexer, when it is true, compiler will print token buffer, and won't parse AST.
    static bool  only_tokenize;
	// to Solve an error, the lexer will skip current line, but for special cases like NEWLINE, lexer will not skip.
    static bool skip_line;		
    // line is the number of current line, tab is 1 if there are tabs in this line, otherwise is 0
	// ch is the right location of error token, chp is the left location of error token. 
    static int line, ch, chp, tab;		
    static int log_color;				// the color the debugger going to use while print infos.
    
    static void SetStream(const bool t);
    // Write all output to file , for debug and testing.
    static void WriteOutput(const char* file);

    // this special micro will be called when the error token is "Newline", then we need to go back a line to print debug info.

    enum Color {
        kDarkBlue = 1,
        kDarkGreen,
        kDarkTeal,
        kDarkRed,
        kDarkPink,
        kDarkYellow,
        kGray,
        kDarkGray,
        kBlue,
        kGreen,
        kTeal,
        kRed,
        kPink,
        kYellow,
        kWhite
    };

    static void SetColor(const int c);

    // this function is implemented in main.cpp,
    // because there are some issues with include<windows.h> before other headers.
    static void PrintErrorInfo(const std::wstring type, int l=-1,int c = -1);

    //This function will print the current token and the line of code it belongs.
    static void PrintErrorPostfix(int l = -1, int c = -1,int cp=-1);

    static void CatchNewline();

	
	static void Error(const std::wstring info);
	static void ErrorNonBreak(const std::wstring info);
	static llvm::Value* ErrorV(const std::wstring info, int line, int ch);
	static llvm::Value* Debugger::ErrorV(const char* str, int line, int ch);

    static void Warn(const std::wstring info);

    // this micro should be called each time AST parsed a node, to stop immediately if there are error.
    // #define VERIFY {if(Debugger::error_occurred)return nullptr;}

};

class ErrorMsg {
public:
	static wchar_t* cannot_resolve_symbol;
	static wchar_t* invalid_token;
	static wchar_t* expected;
	static wchar_t* expected_but;
	static wchar_t* unexpected;

	static std::wstring Format(wchar_t* format, ...);
};
	


#endif