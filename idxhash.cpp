#include "gtest/gtest.h"

extern "C" {
	#include "misc/idxhash.h"
}

class IdxHash : public ::testing::Test {
protected:
	void SetUp()
	{
		ih_errcode_t res = ih_init8(buf, 1024, 16, 0, true, 2, &hndlr);
	}
	void TearDown()
	{
	}

	ih_hndlr_t hndlr;
	char	buf[1024];
};


TEST_F(IdxHash, TestAdd)
{
	uint16* value = 0;
	ASSERT_EQ(ih_hash8_add(hndlr, "x", 0, (char**)&value), IH_ERR_SUCCESS); *value = 1;
	ASSERT_EQ(ih_hash8_add(hndlr, "y", 0, (char**)&value), IH_ERR_SUCCESS); *value = 2;
	ASSERT_EQ(ih_hash8_add(hndlr, "z", 0, (char**)&value), IH_ERR_SUCCESS); *value = 3;
	ASSERT_EQ(ih_hash8_add(hndlr, "a", 0, (char**)&value), IH_ERR_SUCCESS); *value = 4;
	ASSERT_EQ(ih_hash8_add(hndlr, "b", 0, (char**)&value), IH_ERR_SUCCESS); *value = 5;
	ASSERT_EQ(ih_hash8_add(hndlr, "c", 0, (char**)&value), IH_ERR_SUCCESS); *value = 6;
	ASSERT_EQ(ih_hash8_add(hndlr, "d", 0, (char**)&value), IH_ERR_SUCCESS); *value = 7;
	ASSERT_EQ(ih_hash8_add(hndlr, "i", 0, (char**)&value), IH_ERR_SUCCESS); *value = 8;
	ASSERT_EQ(ih_hash8_add(hndlr, "j", 0, (char**)&value), IH_ERR_SUCCESS); *value = 9;
	ASSERT_EQ(ih_hash8_add(hndlr, "k", 0, (char**)&value), IH_ERR_SUCCESS); *value = 10;
	ASSERT_EQ(ih_hash8_add(hndlr, "l", 0, (char**)&value), IH_ERR_SUCCESS); *value = 11;
	ASSERT_EQ(ih_hash8_add(hndlr, "m", 0, (char**)&value), IH_ERR_SUCCESS); *value = 12;
	ASSERT_EQ(ih_hash8_add(hndlr, "n", 0, (char**)&value), IH_ERR_SUCCESS); *value = 13;
	ASSERT_EQ(ih_hash8_add(hndlr, "sysdate", 0, (char**)&value), IH_ERR_SUCCESS); *value = 14;
	ASSERT_EQ(ih_hash8_add(hndlr, "first_date", 0, (char**)&value), IH_ERR_SUCCESS); *value = 15;
	ASSERT_EQ(ih_hash8_add(hndlr, "last_date", 0, (char**)&value), IH_ERR_SUCCESS); *value = 16;
	ASSERT_EQ(ih_hash8_add(hndlr, "last_event", 0, (char**)&value), IH_ERR_SUCCESS); *value = 17;

	ASSERT_EQ(ih_hash8_add(hndlr, "i", 0, (char**)&value), IH_ENTRY_EXISTS); *value = 18;
	ASSERT_EQ(ih_hash8_add(hndlr, "y", 0, (char**)&value), IH_ENTRY_EXISTS); *value = 19;
}

TEST_F(IdxHash, TestSearch)
{
	uint16* value = 0;
	ASSERT_EQ(ih_hash8_add(hndlr, "x", 0, (char**)&value), IH_ERR_SUCCESS); *value = 1;
	ASSERT_EQ(ih_hash8_add(hndlr, "first_date", 0, (char**)&value), IH_ERR_SUCCESS); *value = 2;
	ASSERT_EQ(ih_hash8_add(hndlr, "d", 0, (char**)&value), IH_ERR_SUCCESS); *value = 3;
	ASSERT_EQ(ih_hash8_add(hndlr, "i", 0, (char**)&value), IH_ERR_SUCCESS); *value = 4;

	ASSERT_EQ(ih_hash8_search(hndlr, "x", 0, (char**)&value), IH_ERR_SUCCESS);
	ASSERT_EQ(*value, 1);
	ASSERT_EQ(ih_hash8_search(hndlr, "first_date", 0, (char**)&value), IH_ERR_SUCCESS);
	ASSERT_EQ(*value, 2);
	ASSERT_EQ(ih_hash8_search(hndlr, "d", 0, (char**)&value), IH_ERR_SUCCESS);
	ASSERT_EQ(*value, 3);
	ASSERT_EQ(ih_hash8_search(hndlr, "i", 0, (char**)&value), IH_ERR_SUCCESS);
	ASSERT_EQ(*value, 4);

	ASSERT_EQ(ih_hash8_search(hndlr, "last_date", 0, (char**)&value), IH_ENTRY_NOTFOUND);
}
