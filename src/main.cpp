
#define PRINT_TO_STREAM

#include "debug.h"
#include "lexer.h"
#include "parser.h"
#include "gen.h"
#include "debug.h"
#include <ctime>
#include <windows.h>
HANDLE handle;


inline void SetColor(const int c) {
	if (handle == nullptr)handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, c);
}
enum CHARACTER_ENCODING
{
	ANSI,
	Unicode,
	Unicode_big_endian,
	UTF8_with_BOM,
	UTF8_without_BOM
};
CHARACTER_ENCODING get_text_file_encoding(const wchar_t* filename)
{
	CHARACTER_ENCODING encoding;

	unsigned char uniTxt[] = { 0xFF, 0xFE };// Unicode file header
	unsigned char endianTxt[] = { 0xFE, 0xFF };// Unicode big endian file header
	unsigned char utf8Txt[] = { 0xEF, 0xBB };// UTF_8 file header

	DWORD dwBytesRead = 0;
	HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		hFile = nullptr;
		CloseHandle(hFile);
		printf("cannot open file");
	}
	const auto lp_header = new BYTE[2];
	ReadFile(hFile, lp_header, 2, &dwBytesRead, nullptr);
	CloseHandle(hFile);

	if (lp_header[0] == uniTxt[0] && lp_header[1] == uniTxt[1])// Unicode file
		encoding = CHARACTER_ENCODING::Unicode;
	else if (lp_header[0] == endianTxt[0] && lp_header[1] == endianTxt[1])//  Unicode big endian file
		encoding = CHARACTER_ENCODING::Unicode_big_endian;
	else if (lp_header[0] == utf8Txt[0] && lp_header[1] == utf8Txt[1])// UTF-8 file
		encoding = CHARACTER_ENCODING::UTF8_with_BOM;
	else
		encoding = CHARACTER_ENCODING::ANSI;   //Ascii

	delete[]lp_header;
	return encoding;
}
int main(int argc, char** argv)
{
	
	std::string filename;
	if(argc==1)
	{
		filename = "../tests/codes/all_symbols.df";
	}
	else if(argc>1)
	{
		filename = argv[1];
	}
	only_tokenlize = argc>2;
	error_existed = only_tokenlize;
	std::wcout.imbue(std::locale(""));
	const auto start = clock();
	LoadFile(filename.c_str());

	auto program = Parse();
	std::wcout << outstream.str();
	if (!error_existed)
	{
		program->Gen();
		WriteReadableIR(the_module.get(),"ir.txt",true);
		WriteBitCodeIR(the_module.get(),"a.ll");
		std::cout <<"Compiled successfully, took a total of "<<static_cast<double>(clock() - start)<<"ms\n\n";
	}
	else std::cout<<"Compiler stopped due to errors occurred\n\n";
	
	WriteOutput("log.txt");

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