﻿#ifndef DEBUGGER
#define DEBUGGER

#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <codecvt>

#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/Module.h>
#include <llvm/Bitcode/BitcodeWriter.h>

namespace lexer {
    class Token;
    static wchar_t* src;
    static wchar_t* root;
    static wchar_t peek;
    static long size;
    static Token* token;
    static std::wstring string_val;
    static double number_val;
}


namespace debugger {

    static std::basic_ostream<wchar_t>* out = nullptr;
    static bool is_std_out;

    static bool
    error_existed;			// Once called ALERT, error_existed will be true and will not be changed, and the compiler won't generate IR.
    static bool
    error_occurred;			// Once called ALERT, error_occurred will be true, and all AST parsing will stop immediately,
    // error_occurred will be false once lexer moved to the next line.
    static bool
    only_tokenize;			// only_tokenize is used for testing lexer, when it is true, compiler will print token buffer, and won't parse AST.
    static bool skip_line =
        true;		// to Solve an error, the lexer will skip current line, but for special cases like NEWLINE, lexer will not skip.

    static std::vector<wchar_t*> lines;	// collection of pointers to the beginning of each line of source code.
    // line is the number of current line, tab is 1 if there are tabs in this line, otherwise is 0
    static int line, ch, chp,
               tab;		// ch is the right location of error token, chp is the left location of error token. 
    static int log_color;				// the color the debugger going to use while print infos.
    static wchar_t* end;				// pointer to the last character of  sourcecode

    static void SetStream(const bool t) {
        is_std_out = t;
        out = is_std_out ? &std::wcout : new std::wstringstream();
    }

    // Write all output to file , for debug and testing.
    static void WriteOutput(const char* file) {
        if (is_std_out)return;
        std::wofstream stream;
        stream.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>));
        stream.open(file);
        stream << dynamic_cast<std::wstringstream*>(out)->str();
        stream.close();
    }
    // Write human-readable ir to file , for debug and testing.
    static void WriteReadableIr(llvm::Module* module, const char* file, bool print = false) {
        std::string ir;
        llvm::raw_string_ostream ir_stream(ir);
        ir_stream << *module;
        ir_stream.flush();
        std::ofstream file_stream;
        file_stream.open(file);
        file_stream << ir;
        file_stream.close();
        if (print)std::cout << ir;
    }
    // Write compilable ir to file , for further compilation.
    static void WriteBitCodeIr(llvm::Module* module, const char* file) {
        std::error_code ec;
        llvm::raw_fd_ostream os(file, ec, llvm::sys::fs::F_None);
        WriteBitcodeToFile(*module, os);
        os.flush();
    }

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

    inline void SetColor(const int c);	// this function is implemented in main.cpp,
    // because there are some issues with include<windows.h> before other headers.
    static void PrintErrorInfo(const std::wstring type, const bool show_location = true) {
        if (show_location)
            *out << L"[" << line + 1 << L"," << ch << L"]: ";
        SetColor(log_color);
        *out << type << L": ";
        SetColor(kWhite);
    }

    //This function will print the current token and the line of code it belongs.
    static void PrintErrorPostfix() {
        auto pt = lines[line];							//find the start pointer of this line.
        std::wstring str;
        while (*pt != L'\n' && pt < end)str += *pt++;		//push all the characters to str

        SetColor(kDarkTeal);
        *out << str << std::endl;						// print error line of code
        SetColor(log_color);

        auto space = chp - 1;							// calculate red arrow location
        auto error_token = ch - chp - 1;
        space = space < 0 ? 0 : space;					// catch when location is negative
        error_token = error_token < 0 ? 0 : error_token;
        *out << std::wstring(space, L' ') << L"↑" << std::wstring(error_token, L'`') << std::endl;
        SetColor(kWhite);
    }

    inline void AlertNewline() {
        chp = ch = --lexer::src - lines[--line];
        lines.pop_back();
        skip_line = false;
    }

    inline void AlertNonBreak(const std::wstring info) {
        error_existed = true;
        log_color = kRed;
        PrintErrorInfo(L"error");
        *out << info << std::endl;
        PrintErrorPostfix();
    }

    inline void Alert(const std::wstring info) {
        error_occurred = true;
        AlertNonBreak(info);
    }

    inline void Warn(const std::wstring info) {
        log_color = kYellow;
        PrintErrorInfo(L"warning");
        *out << info << std::endl;
        PrintErrorPostfix();
    }
    // this micro should be called each time AST parsed a node, to stop immediately if there are error.
#define VERIFY {if(error_occurred)return nullptr;}


}
#endif

//To avoid the Circular Dependency, The first half of this file will be included first, and the second half will be included after "gen.h"
#ifdef PARSER
#ifndef OUTPUT
#define OUTPUT
#include "parser.h"

// This function takes a type enum like SubAssign, K_float, and return its name "-=" "float"
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

// implement all the ToString function for Expression nodes below.
inline void Factor::ToString() { *debugger::out<<"[Factor]"; }
inline void NumberConst::ToString()
{

	switch (type)
	{
	case K_int:*debugger::out << "["<< static_cast<int>(value)<<"]";return;
	case K_float:*debugger::out << "["<<value<<"]", value; return;
	case K_double:*debugger::out << "[" << value << "]"; return;
	default:*debugger::out << "[" << Token::Name(type) << "]"; return;
	}
}
inline void String::ToString() { *debugger::out << "[\"" << value << "\"]"; }
inline void Boolean::ToString() { *debugger::out << "["<< (value ? "true" : "false")<<"]"; }
inline void Field::ToString() { *debugger::out << "[" <<names[0] << "]"; }
inline void FuncCall::ToString()
{
	*debugger::out << "[CALL " <<names[0]<<" ( ";
	for (auto & arg : args)
	{
		arg->ToString();
		*debugger::out << ",";
	}
	*debugger::out << " )]\n";
}
inline void Unary::ToString() { *debugger::out << "<"<< Token::Name(op)<<">"; expr->ToString(); }
inline void Binary::ToString() { *debugger::out << "("; LHS->ToString(); *debugger::out << " " << Token::Name(op) << " "; RHS->ToString(); *debugger::out << ")"; }
inline void Ternary::ToString()
{
	*debugger::out << "[";
	a->ToString();
	*debugger::out << "?";
	b->ToString();
	*debugger::out << ":";
	c->ToString();
	*debugger::out << "]";
}

#endif
#endif
