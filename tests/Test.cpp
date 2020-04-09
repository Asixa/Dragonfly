#include "LexerTest.h"


TEST_F(LexerTest, Check)
{
	EXPECT_EQ(true, false);
	EXPECT_EQ(true, false);
	EXPECT_EQ(true, true);
	EXPECT_EQ(1, 2);
	EXPECT_EQ(true, false);
	EXPECT_EQ(true, false);
}

TEST_F(LexerTest, Check2)
{

	EXPECT_EQ(1, 2);;
}