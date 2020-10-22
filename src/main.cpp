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
#include <iostream>
#include <sstream>

#include "frontend/lexer.h"
#include "frontend/parser.h"
#include "frontend/package-manager.h"
#include "frontend/preprocessor.h"

int main(int argc, char** argv) {


	PackageManager::QueryPackages();
	// system("pause");
	// return 0;
    std::string filename;
    if (argc == 1) {
		// filename = "../tests/codes/default3.df";
		filename = "../tests/codes/generic2.df";
		// filename = "../tests/codes/b/input.df";
    }
    else if (argc > 1) {
        filename = argv[1];
    }

    Debugger::SetStream(argc == 1);
    Debugger::only_tokenize = argc > 2;
	
    Debugger::error_existed = Debugger::only_tokenize;
    std::wcout.imbue(std::locale(""));
    const auto start = clock();
	// Lexer::LoadFile(filename.c_str());
	Preprocessor::AddFile(filename);
	Preprocessor::Process();
	const auto context = std::make_shared<DFContext>();

    context->program = AST::Parse();
    if (!Debugger::is_std_out)
        std::wcout << dynamic_cast<std::wstringstream*>(Debugger::out)->str();
	context->program->Gen(context);
    if (!Debugger::error_existed) {
        
		context->WriteReadableIr(context->module.get(), "ir.txt", true);
		context->WriteBitCodeIr(context->module.get(), "a.ll");
        std::cout << "Compiled successfully, took a total of " << static_cast<double>(clock() - start) << "ms\n\n";
    }
    else std::cout << "Compiler stopped due to errors occurred\n\n";
    Debugger::WriteOutput("log.txt");
    if (argc == 1)system("pause");
     
    return 0;
}
 