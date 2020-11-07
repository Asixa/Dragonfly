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

#include <iostream>
#include <sstream>
#include <codecvt>
#include <string>
#include <vector>
#include <fstream>

#include "frontend/lexer.h"
#include <windows.h>
#include "frontend/preprocessor.h"
using namespace frontend;
std::basic_ostream<wchar_t>* Debugger::out = nullptr;
bool Debugger::is_std_out = false;
bool Debugger::error_existed = false;
bool Debugger::error_occurred = false;
bool Debugger::only_tokenize = false;
bool Debugger::skip_line = true;

int Debugger::line = 0;
int Debugger::ch = 0;
int Debugger::chp = 0;
int Debugger::tab = 0;
int Debugger::log_color = 0;
std::vector<wchar_t*> Debugger::lines;

void Debugger::SetStream(const bool t) {
    is_std_out = t;
    out = is_std_out ? &std::wcout : new std::wstringstream();
}

void Debugger::WriteOutput(const char* file) {
    if (is_std_out)return;
    std::wofstream stream;
    stream.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>));
    stream.open(file);
    stream << dynamic_cast<std::wstringstream*>(out)->str();
    stream.close();
}

void Debugger::SetColor(const int c) {
    static HANDLE handle;
    if (handle == nullptr)handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(handle, c);
}

void Debugger::PrintHeader(const std::wstring type, int l, const int c) {
    if (l!=-1) {
		*out << L"[" << Preprocessor::MapFileNumber(l) << " ";
		SetColor(kYellow);
		*out << l << L"," << c;
		SetColor(kWhite);
		*out << L"]: ";
    }
    SetColor(log_color);
    *out << type << L": ";
    SetColor(kWhite);
}

void Debugger::PrintCode(int l, int c, int cp) {
    if(l==-1) {
		l = line;
		c = ch;
		cp = chp;
    }
    const auto roughly = c == -1;
    auto pt = lines[l];							//find the start pointer of this line.
    std::wstring str;
    while (*pt != L'\n' && pt < Lexer::end)str += *pt++;		//push all the characters to str

    SetColor(kDarkTeal);
	*out << str;
    if(roughly) {
		SetColor(log_color);
		*out << L" ¡û";
    }
    *out << std::endl;						// print error line of code
    SetColor(log_color);
	if (!roughly) {
		auto space = cp - 1;							// calculate red arrow location
		auto error_token = c - cp - 1;
		space = space < 0 ? 0 : space;					// catch when location is negative
		error_token = error_token < 0 ? 0 : error_token;
		*out << std::wstring(space, L' ') << L"¡ü" << std::wstring(error_token, L'`') << std::endl;
	}
    SetColor(kWhite);
}

void Debugger::CatchNewline() {
    chp = ch = --Lexer::src - lines[--line];
    lines.pop_back();
    skip_line = false;
}

