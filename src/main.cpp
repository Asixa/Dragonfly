
#include"lexer.h"
#include "parser.h"
#include "gen.h"
#include "debug.h"
#include <ctime>
std::wstring readFile(const char* filename)
{
	std::wifstream wif(filename);
	wif.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
	std::wstringstream wss;
	wss << wif.rdbuf();
	return wss.str();
}
int main(int argc, char** argv)
{
	std::wcout.imbue(std::locale(""));

	const auto start = clock();
	LoadFile("F:/Codes/Projects/Academic/ComputerCompiler/Dragonfly/example/all_symbol.df"); 
	Parse();
	printf("\nfinished in %.0fms...\n", static_cast<double>(clock() - start));

	system("pause");
}
  