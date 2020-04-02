
#include"lexer.h"
#include "parser.h"
#include "gen.h"
#include <ctime>

int main(int argc, char** argv)
{
	const auto start = clock();
	
	lexer::LoadFile("../example/basic.df"); 
	parser::Parse();
	printf("\nfinished in %.0fms...\n", static_cast<double>(clock() - start));
	system("pause");
}
  