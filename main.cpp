#include <cstdio>
#include "gtest/gtest.h"

class MyTest : public ::testing::Test {
};

TEST_F(MyTest, CheckPerimeter)
{
	int a = 10;
	int b = 10;
	ASSERT_TRUE(a = b);
}

