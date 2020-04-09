#include "gtest/gtest.h"


inline void SetColor(const int c) {}
std::string folder;
bool allset;
bool donttest;
int Compare(std::string file1, std::string file2)
{
	// std::ifstream stream1, stream2;
	//
	// stream1.open(file1.c_str()); 
	// stream2.open((folder + file2).c_str());

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
		EXPECT_EQ(wcscmp(string1, string2), 0) << "-the strings are not equal" << "at line" << j << "\n" << string1 << "\n" << string2 << "\n";
	}
	//	auto str1 = wss1.str().c_str();
	//auto str2 = wss2.str().c_str();
	// EXPECT_EQ(strcmp(str1, str2), 0) << "-the strings are not equal\n" << wss1.str().size() << "\n" << wss2.str(). size()<< "\n";

	return 0;
}
struct Tester :public ::testing::Test
{
	void SetUp() override
	{

	}
	void TearDown() override
	{

	}
};


TEST_F(Tester, testLog)
{

	if (donttest)return;
	EXPECT_EQ(Compare("test/log.txt", "/log.txt"), 0);
}
TEST_F(Tester, testIR)
{
	if (donttest)return;
	EXPECT_EQ(Compare("test/ir.txt", "/ir.txt"), 0);
}