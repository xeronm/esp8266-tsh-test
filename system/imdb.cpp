#include "gtest/gtest.h"

extern "C" {
	#include "system/imdb.h"
	#include "core/logging.h"
}

#ifdef IMDB_SMALL_RAM
#define HEADER_CLASS_SIZE	128
#define HEADER_PAGE_SIZE	48
#define HEADER_BLOCK_SIZE	8
#define ROWID_SIZE			4
#define CURSOR_SIZE			40
#else
#define HEADER_CLASS_SIZE	152
#define HEADER_PAGE_SIZE	64
#define HEADER_BLOCK_SIZE	12
#define ROWID_SIZE			8
#define CURSOR_SIZE			48
#endif

#define IMDB_INIT_BLOCKS	4
#define IMDB_INIT_PAGES		1

#define SLOT_TYPE2_SIZE		4
#define SLOT_TYPE4_SIZE		8

class IMDBFixedClass : public ::testing::Test {
protected:
	void SetUp()
	{
		block_size = 4096;
		obj_size = 1024;
		
		imdb_def_t db_def = { (block_size_t) (block_size - 1), BLOCK_CRC_NONE, false, 0, 0 };
		imdb_init(&db_def, 0, &hmdb);
		imdb_class_def_t	cdef = { "testobj", false, false, false, 25, 3, 0, 8, (obj_size_t) (obj_size - 1) };
		imdb_class_create(hmdb, &cdef, &hcls);
	}
	void TearDown()
	{
		imdb_done(hmdb);
	}

	imdb_hndlr_t	hmdb;
	imdb_hndlr_t	hcls;
	block_size_t	block_size;
	obj_size_t	obj_size;
};

class IMDBFixedRecycleClass : public ::testing::Test {
protected:
	void SetUp()
	{
		block_size = 4096;
		obj_size = 1024;

		imdb_def_t db_def = { (block_size_t) (block_size - 1), BLOCK_CRC_NONE, false, 0, 0 };
		imdb_init(&db_def, 0, &hmdb);
		imdb_class_def_t	cdef = { "testobj", true, false, false, 25, 1, 0, 8, (obj_size_t) (obj_size - 1) };
		imdb_class_create(hmdb, &cdef, &hcls);
	}
	void TearDown()
	{
		imdb_done(hmdb);
	}

	imdb_hndlr_t	hmdb;
	imdb_hndlr_t	hcls;
	block_size_t	block_size;
	obj_size_t	obj_size;
};

class IMDBVariableClass : public ::testing::Test {
protected:
	void SetUp()
	{
		block_size = 4096;
		obj_size_div = 64;

		imdb_def_t db_def = { (block_size_t) (block_size - 1), BLOCK_CRC_NONE, false, 0, 0 };
		imdb_init(&db_def, 0, &hmdb);
		imdb_class_def_t	cdef = { "testobj", false, true, false, 25, 3, 0, 8, 0 };
		imdb_class_create(hmdb, &cdef, &hcls);
	}
	void TearDown()
	{
		imdb_done(hmdb);
	}

	imdb_hndlr_t	hmdb;
	imdb_hndlr_t	hcls;
	block_size_t	block_size;
	obj_size_t	obj_size_div;
};

class IMDBVariableRecycleClass : public ::testing::Test {
protected:
	void SetUp()
	{
		block_size = 4096;
		obj_size_div = 64;

		imdb_def_t db_def = { (block_size_t) (block_size - 1), BLOCK_CRC_NONE, false, 0, 0 };
		imdb_init(&db_def, 0, &hmdb);
		imdb_class_def_t	cdef = { "testobj", true, true, false, 25, 1, 0, 8, 0 };
		imdb_class_create(hmdb, &cdef, &hcls);
	}
	void TearDown()
	{
		imdb_done(hmdb);
	}

	imdb_hndlr_t	hmdb;
	imdb_hndlr_t	hcls;
	block_size_t	block_size;
	obj_size_t	obj_size_div;
};

class OSPerfClass : public ::testing::Test {
protected:
	void SetUp()
	{
		alloc_count = 1000;
	}
	void TearDown()
	{
	}
	uint32			alloc_count;
};

class IMDBFixedPerfClass : public ::testing::Test {
protected:
	void SetUp()
	{
		block_size = 8192;
		alloc_count = 1000;
 
		imdb_def_t db_def = { block_size, BLOCK_CRC_NONE, false, 0, 0 };
		imdb_init(&db_def, 0, &hmdb);
		{
			imdb_class_def_t	cdef = { "testobj32", false, false, false, 25, 10000, 0, 64, 32 };
			imdb_class_create(hmdb, &cdef, &hcls_32);
		}
		{
			imdb_class_def_t	cdef = { "testobj64", false, false, false, 25, 10000, 0, 64, 64 };
			imdb_class_create(hmdb, &cdef, &hcls_64);
		}
		{
			imdb_class_def_t	cdef = { "testobj256", false, false, false, 25, 10000, 0, 64, 256 };
			imdb_class_create(hmdb, &cdef, &hcls_256);
		}
	}
	void TearDown()
	{
		imdb_done(hmdb);
	}
 
	imdb_hndlr_t	hmdb;
	imdb_hndlr_t	hcls_32;
	imdb_hndlr_t	hcls_64;
	imdb_hndlr_t	hcls_256;
	block_size_t	block_size;
	uint32			alloc_count;
};

class IMDBFileClass : public ::testing::Test {
protected:
	void SetUp()
	{
		block_size = 8192;
		obj_size_div = 64;

		imdb_def_t db_def = { block_size, BLOCK_CRC_NONE, false, 0, 0 };
		imdb_init(&db_def, 0, &hmdb);

		imdb_def_t db_def2 = { block_size, BLOCK_CRC_WRITE, true, 4, 256 };
		imdb_init(&db_def2, hmdb, &hfdb);

		imdb_class_def_t	cdef = { "testobj", false, true, false, 25, 3, 0, 8, 0 };
		ASSERT_EQ(imdb_class_create(hfdb, &cdef, &hcls), IMDB_ERR_SUCCESS);
	}
	void TearDown()
	{
		imdb_done(hmdb);
	}

	imdb_hndlr_t	hmdb;
	imdb_hndlr_t	hfdb;
	imdb_hndlr_t	hcls;
	block_size_t	block_size;
	uint32		alloc_count;
	obj_size_t	obj_size_div;
};

TEST_F(IMDBFixedClass, StorageParameters)
{
	imdb_info_t imdb_inf;
	imdb_errcode_t ret = imdb_info(hmdb, &imdb_inf, NULL, 0);
	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
	ASSERT_EQ(imdb_inf.size_block, HEADER_BLOCK_SIZE);
	ASSERT_EQ(imdb_inf.size_page, HEADER_PAGE_SIZE);
	ASSERT_EQ(imdb_inf.size_class, HEADER_CLASS_SIZE);
	ASSERT_EQ(imdb_inf.size_rowid, ROWID_SIZE);
	ASSERT_EQ(imdb_inf.size_cursor, CURSOR_SIZE);

	imdb_class_info_t class_info;
	ret = imdb_class_info(hcls, &class_info);
	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);

	ASSERT_EQ(class_info.cdef.obj_size, obj_size);
	ASSERT_EQ(class_info.pages, 1);
	ASSERT_EQ(class_info.blocks, class_info.cdef.init_blocks);

	ASSERT_EQ(class_info.blocks_free, class_info.cdef.init_blocks - 1);
	ASSERT_EQ(class_info.slots_free, 1);
	ASSERT_EQ(class_info.slots_free_size, block_size - HEADER_CLASS_SIZE);

	imdb_hndlr_t hcur;
	void* ptrs[10];
	os_memset(ptrs, 0, sizeof(ptrs));
	uint16 rcnt;
	ret = imdb_class_query(hcls, PATH_NONE, &hcur);
	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);

	ret = imdb_class_fetch(hcur, 10, &rcnt, ptrs);
	ASSERT_EQ(ret, IMDB_CURSOR_NO_DATA_FOUND);
	ASSERT_EQ(rcnt, 0);

	ret = imdb_class_close(hcur);
	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
}

TEST_F(IMDBFixedClass, InsertOne)
{
	void* ptr;
	imdb_errcode_t ret = imdb_clsobj_insert(hcls, &ptr, 0);
	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);

	imdb_class_info_t class_info;
	imdb_class_info(hcls, &class_info);

	ASSERT_EQ(class_info.pages, 1);
	ASSERT_EQ(class_info.blocks_free, class_info.cdef.init_blocks - 1);
	ASSERT_EQ(class_info.slots_free, 1);
	ASSERT_EQ(class_info.slots_free_size, block_size - HEADER_CLASS_SIZE - (obj_size+SLOT_TYPE2_SIZE));
}

TEST_F(IMDBFixedClass, BlockFillAndAllocateNext)
{
	void* ptr;

	int i;
	for (i = 0; i < 3; i++) {
		imdb_errcode_t ret = imdb_clsobj_insert(hcls, &ptr, 0);
		ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
	}

	imdb_class_info_t class_info;
	imdb_class_info(hcls, &class_info);

	ASSERT_EQ(class_info.pages, 1);
	ASSERT_EQ(class_info.blocks_free, class_info.cdef.init_blocks - 2);
	ASSERT_EQ(class_info.slots_free, 1);
	ASSERT_EQ(class_info.slots_free_size, block_size - HEADER_BLOCK_SIZE);
}

TEST_F(IMDBFixedClass, PageFill)
{
	void* ptr;

	int i;
	for (i = 0; i < 12; i++) {
		imdb_errcode_t ret = imdb_clsobj_insert(hcls, &ptr, 0);
		ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
	}

	imdb_class_info_t class_info;
	imdb_class_info(hcls, &class_info);

	ASSERT_EQ(class_info.pages, 1);
	ASSERT_EQ(class_info.blocks_free, 0);
	ASSERT_EQ(class_info.slots_free, 0);
	ASSERT_EQ(class_info.slots_free_size, 0);
}

TEST_F(IMDBFixedClass, PageFillAndAllocateNext)
{
	void* ptr;

	int i;
	for (i = 0; i < 13; i++) {
		imdb_errcode_t ret = imdb_clsobj_insert(hcls, &ptr, 0);
		ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
	}

	imdb_class_info_t class_info;
	imdb_class_info(hcls, &class_info);

	ASSERT_EQ(class_info.pages, 2);
	ASSERT_EQ(class_info.blocks, class_info.cdef.init_blocks + class_info.cdef.page_blocks);
	ASSERT_EQ(class_info.blocks_free, class_info.cdef.page_blocks - 1);
	ASSERT_EQ(class_info.slots_free, 1);
	ASSERT_EQ(class_info.slots_free_size, block_size - HEADER_PAGE_SIZE - (obj_size + SLOT_TYPE2_SIZE));
}

TEST_F(IMDBFixedClass, PagesMaxFill)
{
	void* ptr;

	int i;
	for (i = 0; i < 61; i++) {
		imdb_errcode_t ret = imdb_clsobj_insert(hcls, &ptr, 0);
		ASSERT_EQ(ret, ((i==60)? IMDB_ALLOC_PAGES_MAX : IMDB_ERR_SUCCESS) );
	}

	imdb_info_t imdb_inf;
	imdb_info(hmdb, &imdb_inf, NULL, 0);
	ASSERT_EQ(imdb_inf.stat.block_recycle, 0);
	ASSERT_EQ(imdb_inf.stat.slot_data, 60);
	ASSERT_EQ(imdb_inf.stat.slot_split, 40);

	imdb_class_info_t class_info;
	imdb_class_info(hcls, &class_info);

	ASSERT_EQ(imdb_inf.stat.block_alloc, class_info.blocks + IMDB_INIT_BLOCKS);
	ASSERT_EQ(imdb_inf.stat.page_alloc, class_info.pages + IMDB_INIT_PAGES);

	ASSERT_EQ(class_info.pages, 3);
	ASSERT_EQ(class_info.blocks, class_info.cdef.init_blocks + class_info.cdef.page_blocks*2);
	ASSERT_EQ(class_info.blocks_free, 0);
	ASSERT_EQ(class_info.slots_free, 0);
	ASSERT_EQ(class_info.slots_free_size, 0);
}

TEST_F(IMDBFixedClass, DeleteOneAndInsertAfter)
{
	void* ptr;

	int i;
	void* ptr4del;
	for (i = 0; i < 3; i++) {
		imdb_errcode_t ret = imdb_clsobj_insert(hcls, &ptr, 0);
		if (i == 1) ptr4del = ptr;
		ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
	}

	imdb_errcode_t ret = imdb_clsobj_delete(hcls, ptr4del);
	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);

	imdb_class_info_t class_info;
	imdb_class_info(hcls, &class_info);

	ASSERT_EQ(class_info.pages, 1);
	ASSERT_EQ(class_info.blocks_free, class_info.cdef.init_blocks - 2);
	ASSERT_EQ(class_info.slots_free, 2);
	ASSERT_EQ(class_info.slots_free_size, block_size - HEADER_BLOCK_SIZE + (obj_size+SLOT_TYPE2_SIZE) );

	ret = imdb_clsobj_insert(hcls, &ptr, 0);
	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
	ASSERT_EQ(ptr, ptr4del);

	imdb_class_info(hcls, &class_info);

	ASSERT_EQ(class_info.pages, 1);
	ASSERT_EQ(class_info.blocks_free, class_info.cdef.init_blocks - 2);
	ASSERT_EQ(class_info.slots_free, 1);
	ASSERT_EQ(class_info.slots_free_size, block_size - HEADER_BLOCK_SIZE);
}

TEST_F(IMDBFixedRecycleClass, PageFillAndRecycle)
{
	void* ptr;

	imdb_errcode_t ret;
	int i;
	for (i = 0; i < 16; i++) {
		ret = imdb_clsobj_insert(hcls, &ptr, 0);
		ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
		os_memcpy(ptr, &i, sizeof(i));
	}

	imdb_info_t imdb_inf;
	imdb_info(hmdb, &imdb_inf, NULL, 0);
	ASSERT_EQ(imdb_inf.stat.block_recycle, 2);

	imdb_class_info_t class_info;
	imdb_class_info(hcls, &class_info);

	ASSERT_EQ(class_info.pages, 1);
	ASSERT_EQ(class_info.blocks, class_info.cdef.init_blocks);
	ASSERT_EQ(class_info.blocks_free, 0);
	ASSERT_EQ(class_info.slots_free, 1);
	//ASSERT_EQ(class_info.slots_free_size, block_size - HEADER_PAGE_SIZE - (obj_size + SLOT_TYPE2_SIZE));

	imdb_hndlr_t hcur;
	void* ptrs[10];
	os_memset(ptrs, 0, sizeof(ptrs));
	uint16 rcnt;
	ret = imdb_class_query(hcls, PATH_NONE, &hcur);
	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);

	ret = imdb_class_fetch(hcur, 10, &rcnt, ptrs);
	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
	ASSERT_EQ(rcnt, 10);
	int j;
	for (i = 0; i < rcnt; i++) {
		os_memcpy(&j, (void*)ptrs[i], sizeof(j));
		ASSERT_EQ(15 - i, j);
	}

	ret = imdb_class_fetch(hcur, 10, &rcnt, ptrs);
	ASSERT_EQ(ret, IMDB_CURSOR_NO_DATA_FOUND);
	ASSERT_EQ(rcnt, 0);

	ret = imdb_class_close(hcur);
	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
}

TEST_F(IMDBVariableClass, StorageBasicsVariable)
{
	imdb_class_info_t class_info;
	imdb_errcode_t ret = imdb_class_info(hcls, &class_info);

	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);

	ASSERT_EQ(class_info.cdef.obj_size, 0);
//	ASSERT_EQ(class_info.cdef.block_size, block_size);
	ASSERT_EQ(class_info.pages, 1);
	ASSERT_EQ(class_info.blocks, class_info.cdef.init_blocks);

	ASSERT_EQ(class_info.blocks_free, class_info.cdef.init_blocks - 1);
	ASSERT_EQ(class_info.slots_free, 1);
	ASSERT_EQ(class_info.slots_free_size, block_size - HEADER_CLASS_SIZE);

	imdb_hndlr_t hcur;
	void* ptrs[10];
	os_memset(ptrs, 0, sizeof(ptrs));
	uint16 rcnt;
	ret = imdb_class_query(hcls, PATH_NONE, &hcur);
	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);

	ret = imdb_class_fetch(hcur, 10, &rcnt, ptrs);
	ASSERT_EQ(ret, IMDB_CURSOR_NO_DATA_FOUND);
	ASSERT_EQ(rcnt, 0);

	ret = imdb_class_close(hcur);
	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
}



TEST_F(IMDBVariableClass, InsertOne)
{
	void* ptr;
	imdb_errcode_t ret = imdb_clsobj_insert(hcls, &ptr, obj_size_div - 1);
	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);

	imdb_class_info_t class_info;
	imdb_class_info(hcls, &class_info);

	ASSERT_EQ(class_info.pages, 1);
	ASSERT_EQ(class_info.blocks_free, class_info.cdef.init_blocks - 1);
	ASSERT_EQ(class_info.slots_free, 1);
	ASSERT_EQ(class_info.slots_free_size, block_size - HEADER_CLASS_SIZE - (obj_size_div + SLOT_TYPE4_SIZE));
}


TEST_F(IMDBVariableClass, Insert128)
{
	void* ptr;

	imdb_errcode_t ret;
	int i;
	for (i = 0; i < 128; i++) {
		ret = imdb_clsobj_insert(hcls, &ptr, obj_size_div*(1 + i % 16));
		ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
		os_memcpy(ptr, &i, sizeof(i));
	}

	imdb_info_t imdb_inf;
	imdb_class_info_t info_array[10];
	imdb_info(hmdb, &imdb_inf, &info_array[0], 10);

	imdb_class_info_t class_info;
	imdb_class_info(hcls, &class_info);

	ASSERT_EQ(class_info.pages, 3);
	ASSERT_EQ(class_info.blocks, class_info.cdef.init_blocks + class_info.cdef.page_blocks * 2);
	ASSERT_EQ(class_info.blocks_free, 1);
	ASSERT_EQ(class_info.slots_free, 18);
	ASSERT_EQ(class_info.fl_skip_count, 76);
	ASSERT_EQ(class_info.slots_free_size, 6692);

	imdb_hndlr_t hcur;
	void* ptrs[32];
	os_memset(ptrs, 0, sizeof(ptrs));
	uint16 rcnt;
	ret = imdb_class_query(hcls, PATH_NONE, &hcur);
	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);

	int cnt = 0;
	while (ret == IMDB_ERR_SUCCESS) {
		ret = imdb_class_fetch(hcur, 32, &rcnt, ptrs);
		if (ret == IMDB_CURSOR_NO_DATA_FOUND) {
			ASSERT_EQ(rcnt, 0);
			break;
		}
		ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
		ASSERT_EQ(rcnt, 32);
		int j;
		for (i = 0; i < rcnt; i++) {
			os_memcpy(&j, (void*)ptrs[i], sizeof(j));
//			ASSERT_EQ(cnt, j);
			cnt++;
		}
	}

	ret = imdb_class_close(hcur);
	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
}


TEST_F(IMDBVariableRecycleClass, PageFillAndRecycle)
{
	void* ptr;

	imdb_errcode_t ret;
	int i;
	for (i = 0; i < 40; i++) {
		ret = imdb_clsobj_insert(hcls, &ptr, obj_size_div*(1 + i % 16));
		ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
		os_memcpy(ptr, &i, sizeof(i));
	}

	imdb_info_t imdb_inf;
	imdb_info(hmdb, &imdb_inf, NULL, 0);
	ASSERT_EQ(imdb_inf.stat.block_recycle, 2);

	imdb_class_info_t class_info;
	imdb_class_info(hcls, &class_info);

	ASSERT_EQ(class_info.pages, 1);
	ASSERT_EQ(class_info.blocks, class_info.cdef.init_blocks);
	ASSERT_EQ(class_info.blocks_free, 0);
	ASSERT_EQ(class_info.slots_free, 1);
	//ASSERT_EQ(class_info.slots_free_size, block_size - HEADER_PAGE_SIZE - (obj_size + SLOT_TYPE2_SIZE));

	imdb_hndlr_t hcur;
	void* ptrs[25];
	os_memset(ptrs, 0, sizeof(ptrs));
	uint16 rcnt;
	ret = imdb_class_query(hcls, PATH_NONE, &hcur);
	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);

	ret = imdb_class_fetch(hcur, 25, &rcnt, ptrs);
	ASSERT_EQ(rcnt, 25);
	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
	int j;
	for (i = 0; i < rcnt; i++) {
		os_memcpy(&j, (void*)ptrs[i], sizeof(j));
		ASSERT_EQ(39 - i, j);
	}

	ret = imdb_class_fetch(hcur, 25, &rcnt, ptrs);
	ASSERT_EQ(ret, IMDB_CURSOR_NO_DATA_FOUND);
	ASSERT_EQ(rcnt, 1);
	for (i = 0; i < rcnt; i++) {
		os_memcpy(&j, (void*)ptrs[i], sizeof(j));
		ASSERT_EQ(14 - i, j);
	}

	ret = imdb_class_close(hcur);
	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
}

#ifdef PERF_TEST
TEST_F(OSPerfClass, PerfTest)
{
	void* ptr;

	int i;
	for (i = 0; i < alloc_count; i++) {
		ptr = os_malloc(32);
		ptr = os_malloc(256);
		ptr = os_malloc(64);
	}
}


TEST_F(IMDBFixedPerfClass, PerfTest)
{
	void* ptr;

	int i;
	for (i = 0; i < alloc_count; i++) {
		imdb_clsobj_insert(hcls_32, &ptr, 0);
		imdb_clsobj_insert(hcls_256, &ptr, 0);
		imdb_clsobj_insert(hcls_64, &ptr, 0);
	}
}
#endif


TEST_F(IMDBFileClass, BufferCacheTest)
{
	void* ptr;

	imdb_errcode_t ret;
	int i;
	for (i = 0; i < 128; i++) {
		ret = imdb_clsobj_insert(hcls, &ptr, obj_size_div*(1 + i % 16));
		ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
		os_memcpy(ptr, &i, sizeof(i));
	}
}
