#include "debug.h"
#include "lexer.h"
#include "parser.h"
#include "gen.h"
#include "debug.h"
#include <ctime>
#include "llvm/Bitcode/BitcodeWriter.h"
#include <windows.h>
HANDLE handle;

inline void SetColor(const int c) {
	if (handle == nullptr)handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, c);
}

using namespace std;

int main(int argc, char** argv)
{
	std::wcout.imbue(std::locale(""));

	const auto start = clock();
	LoadFile("../example/UTF8.df"); 
	auto program = Parse();

	// uncomment this to stop llvm gen.
	//error_existed = true;


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


	
	if (!error_existed)
	{
		printf("-----------------\n");
		program->Gen();
		the_module->dump();
		printf("-----------------\n");
		std::error_code ec;
		raw_fd_ostream os("a.ll", ec, llvm::sys::fs::F_None);
		WriteBitcodeToFile(*the_module, os);
		os.flush();

		printf("Compiled successfully, took a total of %.0fms\n\n", static_cast<double>(clock() - start));
	}
	else printf("Compiler stopped due to errors occurred\n\n");
	

	system("pause");
}
  