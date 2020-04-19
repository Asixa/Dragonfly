
#include "debug.h"
#include "lexer.h"
#include "parser.h"
#include "gen.h"
#include <ctime>
#include <windows.h>

namespace debugger {
	HANDLE handle;
	inline void SetColor(const int c) {
		if (handle == nullptr)handle = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(handle, c);
	}
}

int main(int argc, char** argv)
{
	
	std::string filename;
	if(argc==1)
	{
		filename = "../tests/codes/basicIf/input.df";
	}
	else if(argc>1)
	{
		filename = argv[1];
	}

	debugger::SetStream(argc == 1);
      
	debugger::only_tokenize = argc>2;
	// only_tokenize = true;
	debugger::error_existed = debugger::only_tokenize;
	
	std::wcout.imbue(std::locale(""));
	const auto start = clock();
    lexer::LoadFile(filename.c_str());

	auto program = parser::Parse();
    if(!debugger::is_std_out)
    std::wcout << dynamic_cast<std::wstringstream*>(debugger::out)->str();
	if (!debugger::error_existed)
	{
		program->Gen();
		debugger::WriteReadableIr(the_module.get(),"ir.txt",true);
		debugger::WriteBitCodeIr(the_module.get(),"a.ll");
		std::cout <<"Compiled successfully, took a total of "<<static_cast<double>(clock() - start)<<"ms\n\n";
	}
	else std::cout<<"Compiler stopped due to errors occurred\n\n";
	
	debugger::WriteOutput("log.txt");

	if(argc==1)system("pause");

	return 0;
}


//*************** Print saved lines ***********************
// printf("-----------------\n");
// std::wcout << L"[line" << line << L"]" << L" [lines size" << lines.size() << L"]" << std::endl;
// for(int i=0;i<lines.size();i++)
// {
// 	auto pt = lines[i];
// 	std:std::wstring str;
// 	while (*pt != L'\n' && pt < lexer::end)str += *pt++;
// 	std::wcout <<L"["<<i<<L"]"<< str << std::endl;
// }
// printf("-----------------\n");
//*************** Print saved lines ***********************