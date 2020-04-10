#include <iostream>
#include <sstream>
#include <fstream>
#include <codecvt>
#include "gtest/gtest.h"
#include <windows.h>
namespace test {
// 	-------------------------------------------------------------------------------
// 	888888ba                                               .8888b dP         
// 	88    `8b                                              88   " 88         
// 	88     88 88d888b. .d8888b. .d8888b. .d8888b. 88d888b. 88aaa  88 dP    dP
// 	88     88 88'  `88 88'  `88 88'  `88 88'  `88 88'  `88 88     88 88    88
// 	88    .8P 88       88.  .88 88.  .88 88.  .88 88    88 88     88 88.  .88
// 	8888888P  dP       `88888P8 `8888P88 `88888P' dP    dP dP     dP `8888P88
// 	                                 .88                                  .88
// 	                             d8888P                               d8888P 
//
// 	    d888888P  88888888b .d88888b  d888888P  88888888b  888888ba 
// 	       @8     88        88.    "'    88     88         88    `8b
// 	       88     88aaaa    `Y88888b.    88     88aaaa     88aaaa8P'
// 	       88     88              `8b    88     88         88   `8b.
// 	       88     88        d8'   .8P    88     88         88     88
// 	       dP     88888888P  Y88888P     dP     88888888P  dP     dP
// 	-------------------------------------------------------------------------------

#define T(t,n,a,b)														\
TEST(t, n)																\
{																		\
	Init(a, b);															\
	EXPECT_EQ(Compare("log.txt", "/log.txt"),0);						\
	if(b==Normal)EXPECT_EQ(Compare("ir.txt", "/ir.txt"), 0);			\
}
	
	enum { Normal, OnlyTokenize, TestCatchErrors };
	// Normal:			the compiler will generate both log and ir, like it should be.
	// OnlyTokenize:	the compiler will only do the tokenization, to test lexer.
	// TestCatchErrors:	The given codes contain error, to check if debugger can catch that.
	
	std::string folder;
	// this function compare if two files are exactly same..
	int Compare(std::string file1, std::string file2)
	{
		std::wifstream stream1(file1);
		EXPECT_EQ(stream1.fail(), 0) << "No such file or directory";
		stream1.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
		std::wstringstream wss1;
		wss1 << stream1.rdbuf();

		std::wifstream stream2(folder + file2);
		EXPECT_EQ(stream2.fail(), 0) << "No such file or directory :" << folder + file2;
		stream2.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
		std::wstringstream wss2;
		wss2 << stream2.rdbuf();

		wchar_t string1[256], string2[256];
		auto j = 0;
		while (!wss1.eof())
		{
			wss2.getline(string1, 256);
			wss1.getline(string2, 256);
			j++;
			EXPECT_EQ(wcscmp(string1, string2), 0) << "-the strings are not equal" << "at line" << j << "\n data: " << string1 << "\nlabel: " << string2 << "\n";
		}
		return 0;
	}
	// this function execute a compiler process, generate the ir.txt and log.txt.
	void Init(const wchar_t* path, int type)
	{
		folder = reinterpret_cast<const char*>(path);
		STARTUPINFO info = { sizeof(info) };
		std::wstring cmd = L"dragonfly.exe "+std::wstring(path) + L"/input.df"+(type == OnlyTokenize ?L" token":L"");
		int ret = 0;
		PROCESS_INFORMATION process_info;
		if (CreateProcess(L"dragonfly.exe", const_cast<wchar_t*>(cmd.c_str()),
			nullptr, nullptr, TRUE, 0, NULL, NULL, &info, &process_info))
		{
			WaitForSingleObject(process_info.hProcess, 10000); //wait ten seconds
			DWORD exit_code=999;
			GetExitCodeProcess(process_info.hProcess, &exit_code);
			ret = exit_code;
			CloseHandle(process_info.hProcess);
			CloseHandle(process_info.hThread);
		}

		EXPECT_EQ(ret, 0)
		<< (ret==259?"Timeout ":"")
		<< (ret < -1 ? "Crashed (Or LLVM Aborted) " : "")
		<< (ret ==3 ? "Aborted " : "")
		<<ret;
	}

// ----------------------------------------------------------------------------------------------
// Test suit name    test name          folder location               test type   
T(ParserTest,		a,					L"../tests/codes/a",			Normal)
T(DebuggerTest,		BasicError,			L"../tests/codes/b",			TestCatchErrors)
T(LexerTest,		all_symbols,		L"../tests/codes/all_symbols",	OnlyTokenize)

T(ExpressionTest,   DivisionTest,       L"../tests/codes/ExpressionTest/DivisionTest", Normal)











	
}

