#include "debug.h"
#include "lexer.h"
#include "parser.h"
#include "gen.h"
#include "debug.h"
#include <ctime>
#include "llvm/Bitcode/BitcodeWriter.h"

#include <windows.h>
HANDLE handle;
enum Color { Darkblue = 1, Darkgreen, Darkteal, Darkred, Darkpink, Darkyellow, Gray, Darkgray, Blue, Green, Teal, Red, Pink, Yellow, White };
inline void SetColor(const int c) {
	if (handle == nullptr)handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, c);
}

using namespace std;

int main(int argc, char** argv)
{
	std::wcout.imbue(std::locale(""));

	SetColor(Darkteal);
	std::cout<<  "Hello卧槽+=-2"<<std::endl;
	SetColor(Red);
	std::cout<<  "````````````↑"<<std::endl;
	SetColor(White);
	const auto start = clock();
	LoadFile("F:/Codes/Projects/Academic/ComputerCompiler/Dragonfly/example/UTF8.df"); 
	Parse()->Gen();
	printf("\n-----------------\n");
	the_module->dump();


	std::error_code ec;
	raw_fd_ostream os("a.ll", ec, llvm::sys::fs::F_None);
	WriteBitcodeToFile(*the_module, os);
	os.flush();
	
	printf("\nfinished in %.0fms...\n", static_cast<double>(clock() - start));
	system("pause");
}
  