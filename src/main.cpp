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


#include "frontend/lexer.h"
#include "AST/program.h"
#include "frontend/package-manager.h"
#include "frontend/preprocessor.h"
#include "backend/cuda/cuda-context.h"
#include "backend/cpu/cpu-context.h"

int main(int argc, char** argv) {

    std::wcout.imbue(std::locale(""));
	PackageManager::QueryPackages();
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
    
    const auto start = clock();
	Preprocessor::AddFile(filename);
	Preprocessor::Process();

	// auto ctx = CudaContext::Create(nullptr);
	const auto context = CPUContext::Create(AST::Parse());
	printf("--------------------analysis-----------------------\n");
	DFContext::Analysis();
    if (!Debugger::is_std_out)
        std::wcout << dynamic_cast<std::wstringstream*>(Debugger::out)->str();
	printf("--------------------gen-----------------------\n");
	if(!Debugger::error_existed)DFContext::Gen();
    Debugger::WriteOutput("log.txt");

	std::cout << "took a total of " << static_cast<double>(clock() - start) << "ms\n\n";

    if (argc == 1)system("pause");
     
    return 0;
}
 