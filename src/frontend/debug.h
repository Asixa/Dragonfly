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

#ifndef DEBUG_H 
#define DEBUG_H
#include <llvm/Bitcode/BitcodeWriter.h>

#include "fmt/core.h"
namespace frontend {
	class Debugger {
	public:

		class ErrorMsg {
		public:
			static wchar_t* cannot_resolve_symbol;
			static wchar_t* invalid_token;
			static wchar_t* expected;
			static wchar_t* expected_but;
			static wchar_t* unexpected;

			static std::wstring Format(wchar_t* format, ...);
		};

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
		static void PrintHeader(const std::wstring type, int l = -1, int c = -1);

		//This function will print the current token and the line of code it belongs.
		static void PrintCode(int l = -1, int c = -1, int cp = -1);

		static void CatchNewline();


		template <typename... T> static void Error(const std::string s, const T& ... args) {
			error_occurred = true;
			ErrorNonBreak(s,args...);
		}

		template <typename... T> static void ErrorNonBreak(const std::string s, const T& ... args) {
			error_existed = true;
			log_color = kRed;
			PrintHeader(L"error", line, ch);
			const std::string  message = fmt::format(s, args...);
			*out << message.c_str() << std::endl;
			PrintCode();
			throw - 1;
		}
		template <typename... T> static llvm::Value* ErrorV(int line, int c,const std::string s, const T& ... args) {
			error_existed = true;
			log_color = kRed;
			PrintHeader(L"error", line, ch);
			const std::string  message = fmt::format(s, args...);
			*out << message.c_str() << std::endl;
			if (line != -1)PrintCode(line, ch, ch);
			throw - 1;
		}

		template <typename... T>  static void Warn(const std::string s, const T& ... args) {
			log_color = kYellow;
			PrintHeader(L"warning", line, ch);
			const std::string  message = fmt::format(s, args...);
			*out << message.c_str() << std::endl;
			PrintCode();
		}

		template <typename... T> static void Info(std::string s, const T& ... args) {
			log_color = kGreen;
			PrintHeader(L"info", line, ch);
			const std::string  message = fmt::format(s, args...);
			*out << message.c_str() << std::endl;
			PrintCode();
		}
		template <typename... T> static void Debug(std::string s, const T& ... args) {
			log_color = kBlue;
			PrintHeader(L"debug", -1, ch);
            const std::string  message = fmt::format(s, args...);
			*out << message.c_str() << std::endl;
			// PrintCode();
		}

		// this micro should be called each time AST parsed a node, to stop immediately if there are error.
		// #define VERIFY {if(Debugger::error_occurred)return nullptr;}

	};


}

struct wrapper {
	std::wstring x;
};

template <>
struct fmt::formatter<wrapper> {
	[[maybe_unused]] static constexpr auto parse(const format_parse_context& ctx) { return ctx.begin(); }

	template <typename FormatContext>
	auto format(const wrapper& val, FormatContext& ctx) {
		// convert wide string in val to utf-8 using some available library...
		const char* utf8result = "converted wide string";
		return format_to(ctx.out(), "{}", utf8result);
	}
};


#endif
