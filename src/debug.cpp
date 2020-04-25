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

#include "lexer.h"

#include <windows.h>




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

void Debugger::PrintErrorInfo(const std::wstring type, const bool show_location) {
    if (show_location)
        *out << L"[" << line + 1 << L"," << ch << L"]: ";
    SetColor(log_color);
    *out << type << L": ";
    SetColor(kWhite);
}

void Debugger::PrintErrorPostfix() {
    auto pt = lines[line];							//find the start pointer of this line.
    std::wstring str;
    while (*pt != L'\n' && pt < Lexer::end)str += *pt++;		//push all the characters to str

    SetColor(kDarkTeal);
    *out << str << std::endl;						// print error line of code
    SetColor(log_color);

    auto space = chp - 1;							// calculate red arrow location
    auto error_token = ch - chp - 1;
    space = space < 0 ? 0 : space;					// catch when location is negative
    error_token = error_token < 0 ? 0 : error_token;
    *out << std::wstring(space, L' ') << L"¡ü" << std::wstring(error_token, L'`') << std::endl;
    SetColor(kWhite);
}

void Debugger::AlertNewline() {
    chp = ch = --Lexer::src - lines[--line];
    lines.pop_back();
    skip_line = false;
}

void Debugger::AlertNonBreak(const std::wstring info) {
    error_existed = true;
    log_color = kRed;
    PrintErrorInfo(L"error");
    *out << info << std::endl;
    PrintErrorPostfix();
}

void Debugger::Alert(const std::wstring info) {
    error_occurred = true;
    AlertNonBreak(info);
}

void Debugger::Warn(const std::wstring info) {
    log_color = kYellow;
    PrintErrorInfo(L"warning");
    *out << info << std::endl;
    PrintErrorPostfix();
}
