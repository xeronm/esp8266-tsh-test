#include "gtest/gtest.h"

extern "C" {
	#include "misc/idxhash.h"
}

class IdxHashNullTermKey : public ::testing::Test {
protected:
	void SetUp()
	{
		ih_errcode_t res = ih_init8(buf, 1025, 16, 0, 2, &hndlr);
	}
	void TearDown()
	{
	}

	ih_hndlr_t hndlr;
	char	buf[1024];
};

class IdxHashFixedKey : public ::testing::Test {
protected:
	void SetUp()
	{
		ih_errcode_t res = ih_init8(buf, 641, 16, sizeof(void*), 2, &hndlr);
	}
	void TearDown()
	{
	}

	ih_hndlr_t hndlr;
	char	buf[1024];
};


TEST_F(IdxHashNullTermKey, TestAdd)
{
	uint16* value = 0;
	ASSERT_EQ(ih_hash8_add(hndlr, "x", 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 1;
	ASSERT_EQ(ih_hash8_add(hndlr, "y", 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 2;
	ASSERT_EQ(ih_hash8_add(hndlr, "z", 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 3;
	ASSERT_EQ(ih_hash8_add(hndlr, "a", 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 4;
	ASSERT_EQ(ih_hash8_add(hndlr, "b", 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 5;
	ASSERT_EQ(ih_hash8_add(hndlr, "c", 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 6;
	ASSERT_EQ(ih_hash8_add(hndlr, "d", 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 7;
	ASSERT_EQ(ih_hash8_add(hndlr, "i", 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 8;
	ASSERT_EQ(ih_hash8_add(hndlr, "j", 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 9;
	ASSERT_EQ(ih_hash8_add(hndlr, "k", 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 10;
	ASSERT_EQ(ih_hash8_add(hndlr, "l", 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 11;
	ASSERT_EQ(ih_hash8_add(hndlr, "m", 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 12;
	ASSERT_EQ(ih_hash8_add(hndlr, "n", 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 13;
	ASSERT_EQ(ih_hash8_add(hndlr, "sysdate", 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 14;
	ASSERT_EQ(ih_hash8_add(hndlr, "first_date", 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 15;
	ASSERT_EQ(ih_hash8_add(hndlr, "last_date", 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 16;
	ASSERT_EQ(ih_hash8_add(hndlr, "last_event", 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 17;

	ASSERT_EQ(ih_hash8_add(hndlr, "i", 0, (char**)&value, 0), IH_ENTRY_EXISTS); *value = 18;
	ASSERT_EQ(ih_hash8_add(hndlr, "y", 0, (char**)&value, 0), IH_ENTRY_EXISTS); *value = 19;
}

TEST_F(IdxHashNullTermKey, TestSearch)
{
	uint16* value = 0;
	ASSERT_EQ(ih_hash8_add(hndlr, "x", 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 1;
	ASSERT_EQ(ih_hash8_add(hndlr, "first_date", 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 2;
	ASSERT_EQ(ih_hash8_add(hndlr, "d", 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 3;
	ASSERT_EQ(ih_hash8_add(hndlr, "i", 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 4;
 
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

TEST_F(IdxHashFixedKey, TestAddSearchDel)
{
	uint16* value = 0;
        void* key = (void*) 0xFFFF;
	ASSERT_EQ(ih_hash8_add(hndlr, (const char *) &key, 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 1; 
	key = (void*) 0xFF1F;
	ASSERT_EQ(ih_hash8_add(hndlr, (const char *) &key, 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 2; 
	key = (void*) 0xFF2F;
	ASSERT_EQ(ih_hash8_add(hndlr, (const char *) &key, 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 3; 
	key = (void*) 0xFF3F;
	ASSERT_EQ(ih_hash8_add(hndlr, (const char *) &key, 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 4;

        key = (void*) 0xFF1F;
	ASSERT_EQ(ih_hash8_search(hndlr, (const char *) &key, 0, (char**)&value), IH_ERR_SUCCESS);
	ASSERT_EQ(*value, 2);
        key = (void*) 0xFF3F;
	ASSERT_EQ(ih_hash8_search(hndlr, (const char *) &key, 0, (char**)&value), IH_ERR_SUCCESS);
	ASSERT_EQ(*value, 4);

        key = (void*) 0xFF4F;
	ASSERT_EQ(ih_hash8_search(hndlr, (const char *) &key, 0, (char**)&value), IH_ENTRY_NOTFOUND);

        key = (void*) 0xFF1F;
	ASSERT_EQ(ih_hash8_remove(hndlr, (const char *) &key, 0), IH_ERR_SUCCESS);
	ASSERT_EQ(ih_hash8_search(hndlr, (const char *) &key, 0, (char**)&value), IH_ENTRY_NOTFOUND);
}


void forall_sum (const char *key, ih_size_t keylen, const char *value, ih_size_t valuelen, void * data) {
    //printf("%p, %u, %p, %u - %p=%u\n", key, keylen, value, valuelen, *((void**)key), *((uint16*)value));
    uint32 *sum = (uint32 *) data;
    *sum = *sum + *((uint16*)value);
}

TEST_F(IdxHashFixedKey, TestForAll)
{
	uint16* value = 0;
        void* key = (void*) 0xFFFF;
	ASSERT_EQ(ih_hash8_add(hndlr, (const char *) &key, 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 1; 
	key = (void*) 0xFF1F;
	ASSERT_EQ(ih_hash8_add(hndlr, (const char *) &key, 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 2; 
	key = (void*) 0xFF2F;
	ASSERT_EQ(ih_hash8_add(hndlr, (const char *) &key, 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 3; 
	key = (void*) 0xFF3F;
	ASSERT_EQ(ih_hash8_add(hndlr, (const char *) &key, 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 4;

        key = (void*) 0xFF1F;
	ASSERT_EQ(ih_hash8_remove(hndlr, (const char *) &key, 0), IH_ERR_SUCCESS);
	key = (void*) 0xFF4F;
	ASSERT_EQ(ih_hash8_add(hndlr, (const char *) &key, 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 5;

	uint32 sum = 0;
        ih_hash8_forall (hndlr, forall_sum, (void *) &sum);
	ASSERT_EQ(sum, (1 + 3 + 4 + 5));
}

TEST_F(IdxHashFixedKey, TestFullFillAndCompact)
{
	uint16* value = 0;
        void* key = (void*) 0xFFFF;

	int i = 0;
        ih_errcode_t res;
	while ((res = ih_hash8_add(hndlr, (const char *) &key, 0, (char**)&value, 0)) != IH_BUFFER_OVERFLOW) {
		ASSERT_EQ(res, IH_ERR_SUCCESS); 
		*value = i + 1; 
		key = (void*) ((char *) key + 1);
		i++;
	}
	ASSERT_EQ(i, 24);

	int j;
	for (j = 0; j < 5; j++) {
		key = (void*) ((char *)0xFFFF + j);
		ASSERT_EQ(ih_hash8_remove(hndlr, (const char *) &key, 0), IH_ERR_SUCCESS);
	}

	for (j = 6; j < 16; j+=2) {
		key = (void*) ((char *)0xFFFF + j);
		ASSERT_EQ(ih_hash8_remove(hndlr, (const char *) &key, 0), IH_ERR_SUCCESS);
	}

	key = (void*) 0xFF1F;
	ASSERT_EQ(ih_hash8_add(hndlr, (const char *) &key, 0, (char**)&value, 0), IH_ERR_SUCCESS); *value = 1000;

        key = (void*) 0xFF1F;
	ASSERT_EQ(ih_hash8_search(hndlr, (const char *) &key, 0, (char**)&value), IH_ERR_SUCCESS);

	for (j = 5; j < 16; j+=2) {
		key = (void*) ((char *)0xFFFF + j);
		ASSERT_EQ(ih_hash8_search(hndlr, (const char *) &key, 0, (char**)&value), IH_ERR_SUCCESS);
	}

	for (j = 16; j < 24; j++) {
		key = (void*) ((char *)0xFFFF + j);
		ASSERT_EQ(ih_hash8_search(hndlr, (const char *) &key, 0, (char**)&value), IH_ERR_SUCCESS);
	}

}
