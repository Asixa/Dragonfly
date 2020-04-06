
#include"lexer.h"
#include "parser.h"
#include "gen.h"
#include "debug.h"
#include <ctime>
#include "llvm/Bitcode/BitcodeWriter.h"


int main(int argc, char** argv)
{
	std::wcout.imbue(std::locale(""));
	
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
  