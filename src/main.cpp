
#include"lexer.h"
#include "parser.h"
#include "gen.h"
#include <ctime>
#include <sstream>
#include <fstream>
#include <codecvt>
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

	// const auto start = clock();
	//
	// lexer::LoadFile("../example/basic.df"); 
	// parser::Parse();
	// printf("\nfinished in %.0fms...\n", static_cast<double>(clock() - start));

	
	
	
	auto src = readFile("../example/UTF8.df");
	
	// std::wcout << "你好世界" << std::endl;
	std::wcout <<src << std::endl;
	
	system("pause");
}
  