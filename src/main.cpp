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

#include "debug.h"
#include "lexer.h"
#include "parser.h"
#include "gen.h"

int main(int argc, char** argv) {
    
    std::string filename;
    if (argc == 1) {
        filename = "../tests/codes/basicIf/input.df";
    }
    else if (argc > 1) {
        filename = argv[1];
    }

    debugger::SetStream(argc == 1);

    debugger::only_tokenize = argc > 2;
    // only_tokenize = true;
    debugger::error_existed = debugger::only_tokenize;

    std::wcout.imbue(std::locale(""));
    const auto start = clock();
    lexer::LoadFile(filename.c_str());

    auto program = parser::Parse();
    if (!debugger::is_std_out)
        std::wcout << dynamic_cast<std::wstringstream*>(debugger::out)->str();
    if (!debugger::error_existed) {
        program->Gen();
        debugger::WriteReadableIr(gen::the_module.get(), "ir.txt", true);
        debugger::WriteBitCodeIr(gen::the_module.get(), "a.ll");
        std::cout << "Compiled successfully, took a total of " << static_cast<double>(clock() - start) << "ms\n\n";
    }
    else std::cout << "Compiler stopped due to errors occurred\n\n";

    debugger::WriteOutput("log.txt");

    if (argc == 1)system("pause");

    return 0;
}
