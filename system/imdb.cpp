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
#define HEADER_CLASS_SIZE	128
#define HEADER_PAGE_SIZE	56
#define HEADER_BLOCK_SIZE	24
#define ROWID_SIZE		16
#define CURSOR_SIZE		56
#endif

#define SLOT_TYPE2_SIZE		4
#define SLOT_TYPE4_SIZE		8

class IMDBFixedClass : public ::testing::Test {
protected:
	void SetUp()
	{
		block_size = 4096;
		obj_size = 1024;
		page_blocks = 6;
		class_pages = 3;
		
		imdb_def_t db_def = { (block_size_t) (block_size - 1), BLOCK_CRC_NONE, false, 0, 0 };
		imdb_init(&db_def, &hmdb);
		imdb_class_def_t	cdef = { "testobj", false, false, false, 25, class_pages, page_blocks, (obj_size_t) (obj_size - 1) };
		imdb_class_create(hmdb, &cdef, &hcls);
	}
	void TearDown()
	{
		imdb_done(hmdb);
	}

	imdb_hndlr_t	hmdb;
	imdb_hndlr_t	hcls;
	block_size_t	block_size;
	uint8 		page_blocks;
	uint8		class_pages;
	obj_size_t	obj_size;
};

class IMDBFixedRecycleClass : public ::testing::Test {
protected:
	void SetUp()
	{
		block_size = 4096;
		obj_size = 1024;

		imdb_def_t db_def = { (block_size_t) (block_size - 1), BLOCK_CRC_NONE, false, 0, 0 };
		imdb_init(&db_def, &hmdb);
		imdb_class_def_t	cdef = { "testobj", true, false, false, 25, 1, 4, (obj_size_t) (obj_size - 1) };
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
		imdb_init(&db_def, &hmdb);
		imdb_class_def_t	cdef = { "testobj", false, true, false, 25, 3, 8, 0 };
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
		imdb_init(&db_def, &hmdb);
		imdb_class_def_t	cdef = { "testobj", true, true, false, 25, 1, 4, 0 };
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
		imdb_init(&db_def, &hmdb);
		{
			imdb_class_def_t	cdef = { "testobj32", false, false, false, 25, 10000, 64, 32 };
			imdb_class_create(hmdb, &cdef, &hcls_32);
		}
		{
			imdb_class_def_t	cdef = { "testobj64", false, false, false, 25, 10000, 64, 64 };
			imdb_class_create(hmdb, &cdef, &hcls_64);
		}
		{
			imdb_class_def_t	cdef = { "testobj256", false, false, false, 25, 10000, 64, 256 };
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
	uint32		alloc_count;
};

class IMDBFileClass : public ::testing::Test {
protected:
	void SetUp()
	{
		block_size = 4096;
		obj_size_div = 64;

                fio_user_format(0);

		log_severity = log_severity_get ();
        	log_severity_set (LOG_INFO);

		imdb_def_t db_def2 = { block_size, BLOCK_CRC_NONE, true, 3, 256 };
		imdb_init(&db_def2, &hfdb);

		imdb_class_def_t	cdef = { "testobj", false, true, false, 25, 3, 8, 0 };
		ASSERT_EQ(imdb_class_create(hfdb, &cdef, &hcls), IMDB_ERR_SUCCESS);

		imdb_class_def_t	cdef2 = { "testobj2", false, true, false, 25, 3, 8, 0 };
		ASSERT_EQ(imdb_class_create(hfdb, &cdef2, &hcls2), IMDB_ERR_SUCCESS);
	}
	void TearDown()
	{
		imdb_done(hfdb);
        	log_severity_set (log_severity);
	}

	imdb_hndlr_t	hfdb;
	imdb_hndlr_t	hcls;
	imdb_hndlr_t	hcls2;
	block_size_t	block_size;
	uint32		alloc_count;
	obj_size_t	obj_size_div;
	log_severity_t  log_severity;
};


imdb_errcode_t forall_sum (void *ptr, void *data) {
    uint32 * pdata = (uint32 *) data;
    uint32 * pitem = (uint32 *) ptr;
    *pdata = *pdata + *pitem;
    return IMDB_ERR_SUCCESS;
}

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
	ret = imdb_class_info(hmdb, hcls, &class_info);
	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);

	ASSERT_EQ(class_info.cdef.obj_size, obj_size);
	ASSERT_EQ(class_info.pages, 1);
	ASSERT_EQ(class_info.blocks, class_info.cdef.page_blocks);

	ASSERT_EQ(class_info.blocks_free, class_info.cdef.page_blocks - 1);
	ASSERT_EQ(class_info.slots_free, 1);
	ASSERT_EQ(class_info.slots_free_size, block_size - HEADER_CLASS_SIZE);

	imdb_hndlr_t hcur;
	void* ptrs[10];
	os_memset(ptrs, 0, sizeof(ptrs));
	uint16 rcnt;
	ret = imdb_class_query(hmdb, hcls, PATH_NONE, &hcur);
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
	imdb_errcode_t ret = imdb_clsobj_insert(hmdb, hcls, &ptr, 0);
	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);

	imdb_class_info_t class_info;
	imdb_class_info(hmdb, hcls, &class_info);

	ASSERT_EQ(class_info.pages, 1);
	ASSERT_EQ(class_info.blocks_free, class_info.cdef.page_blocks - 1);
	ASSERT_EQ(class_info.slots_free, 1);
	ASSERT_EQ(class_info.slots_free_size, block_size - HEADER_CLASS_SIZE - (obj_size+SLOT_TYPE2_SIZE));
}

TEST_F(IMDBFixedClass, BlockFillAndAllocateNext)
{
	void* ptr;

	int i;
	for (i = 0; i < 3; i++) {
		imdb_errcode_t ret = imdb_clsobj_insert(hmdb, hcls, &ptr, 0);
		ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
	}

	imdb_class_info_t class_info;
	imdb_class_info(hmdb, hcls, &class_info);

	ASSERT_EQ(class_info.pages, 1);
	ASSERT_EQ(class_info.blocks_free, class_info.cdef.page_blocks - 2);
	ASSERT_EQ(class_info.slots_free, 1);
	ASSERT_EQ(class_info.slots_free_size, block_size - HEADER_BLOCK_SIZE);
}

TEST_F(IMDBFixedClass, PageFill)
{
	void* ptr;

	int i;
	for (i = 0; i < ((block_size - HEADER_BLOCK_SIZE)/obj_size)*page_blocks; i++) {
		imdb_errcode_t ret = imdb_clsobj_insert(hmdb, hcls, &ptr, 0);
		ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
	}

	imdb_class_info_t class_info;
	imdb_class_info(hmdb, hcls, &class_info);

	ASSERT_EQ(class_info.pages, 1);
	ASSERT_EQ(class_info.blocks_free, 0);
	ASSERT_EQ(class_info.slots_free, 0);
	ASSERT_EQ(class_info.slots_free_size, 0);
}

TEST_F(IMDBFixedClass, PageFillAndAllocateNext)
{
	void* ptr;

	int i;
	for (i = 0; i < ((block_size - HEADER_BLOCK_SIZE)/obj_size)*page_blocks + 1; i++) {
		imdb_errcode_t ret = imdb_clsobj_insert(hmdb, hcls, &ptr, 0);
		ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
	}

	imdb_class_info_t class_info;
	imdb_class_info(hmdb, hcls, &class_info);

	ASSERT_EQ(class_info.pages, 2);
	ASSERT_EQ(class_info.blocks, 2*class_info.cdef.page_blocks);
	ASSERT_EQ(class_info.blocks_free, class_info.cdef.page_blocks - 1);
	ASSERT_EQ(class_info.slots_free, 1);
	ASSERT_EQ(class_info.slots_free_size, block_size - HEADER_PAGE_SIZE - (obj_size + SLOT_TYPE2_SIZE));
}

TEST_F(IMDBFixedClass, PagesMaxFill)
{
	void* ptr;

	int i;
	uint8 max_objs = ((block_size - HEADER_BLOCK_SIZE)/obj_size)*page_blocks*class_pages;
	for (i = 0; i < max_objs + 1; i++) {
		imdb_errcode_t ret = imdb_clsobj_insert(hmdb, hcls, &ptr, 0);
		ASSERT_EQ(ret, ((i==max_objs)? IMDB_ALLOC_PAGES_MAX : IMDB_ERR_SUCCESS) );
	}

	imdb_info_t imdb_inf;
	imdb_info(hmdb, &imdb_inf, NULL, 0);
	ASSERT_EQ(imdb_inf.stat.block_recycle, 0);
	ASSERT_EQ(imdb_inf.stat.slot_data, max_objs);
	ASSERT_EQ(imdb_inf.stat.slot_split, 36);

	imdb_class_info_t class_info;
	imdb_class_info(hmdb, hcls, &class_info);

	ASSERT_EQ(imdb_inf.stat.block_alloc, class_info.blocks);
	ASSERT_EQ(imdb_inf.stat.page_alloc, class_info.pages);

	ASSERT_EQ(class_info.pages, class_pages);
	ASSERT_EQ(class_info.blocks, class_pages*class_info.cdef.page_blocks);
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
		imdb_errcode_t ret = imdb_clsobj_insert(hmdb, hcls, &ptr, 0);
		if (i == 1) ptr4del = ptr;
		ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
	}

	imdb_errcode_t ret = imdb_clsobj_delete(hmdb, hcls, ptr4del);
	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);

	imdb_class_info_t class_info;
	imdb_class_info(hmdb, hcls, &class_info);

	ASSERT_EQ(class_info.pages, 1);
	ASSERT_EQ(class_info.blocks_free, class_info.cdef.page_blocks - 2);
	ASSERT_EQ(class_info.slots_free, 2);
	ASSERT_EQ(class_info.slots_free_size, block_size - HEADER_BLOCK_SIZE + (obj_size+SLOT_TYPE2_SIZE) );

	ret = imdb_clsobj_insert(hmdb, hcls, &ptr, 0);
	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
	ASSERT_EQ(ptr, ptr4del);

	imdb_class_info(hmdb, hcls, &class_info);

	ASSERT_EQ(class_info.pages, 1);
	ASSERT_EQ(class_info.blocks_free, class_info.cdef.page_blocks - 2);
	ASSERT_EQ(class_info.slots_free, 1);
	ASSERT_EQ(class_info.slots_free_size, block_size - HEADER_BLOCK_SIZE);
}

TEST_F(IMDBFixedRecycleClass, PageFillAndRecycle)
{
	void* ptr;

	imdb_errcode_t ret;
	int i;
	for (i = 0; i < 16; i++) {
		ret = imdb_clsobj_insert(hmdb, hcls, &ptr, 0);
		ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
		os_memcpy(ptr, &i, sizeof(i));
	}

	imdb_info_t imdb_inf;
	imdb_info(hmdb, &imdb_inf, NULL, 0);
	ASSERT_EQ(imdb_inf.stat.block_recycle, 2);

	imdb_class_info_t class_info;
	imdb_class_info(hmdb, hcls, &class_info);

	ASSERT_EQ(class_info.pages, 1);
	ASSERT_EQ(class_info.blocks, class_info.cdef.page_blocks);
	ASSERT_EQ(class_info.blocks_free, 0);
	ASSERT_EQ(class_info.slots_free, 1);
	//ASSERT_EQ(class_info.slots_free_size, block_size - HEADER_PAGE_SIZE - (obj_size + SLOT_TYPE2_SIZE));

	// query elements
	imdb_hndlr_t hcur;
	void* ptrs[10];
	os_memset(ptrs, 0, sizeof(ptrs));
	uint16 rcnt;
	ret = imdb_class_query(hmdb, hcls, PATH_NONE, &hcur);
	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);

	ret = imdb_class_fetch(hcur, 10, &rcnt, ptrs);
	ASSERT_EQ(rcnt, 10);
	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
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

	// count elements
        uint32 cnt = 0;
        ASSERT_EQ (imdb_class_forall (hmdb, hcls, &cnt, imdb_forall_count), IMDB_ERR_SUCCESS);
        ASSERT_EQ(cnt, 10);
}

TEST_F(IMDBVariableClass, StorageBasicsVariable)
{
	imdb_class_info_t class_info;
	imdb_errcode_t ret = imdb_class_info(hmdb, hcls, &class_info);

	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);

	ASSERT_EQ(class_info.cdef.obj_size, 0);
//	ASSERT_EQ(class_info.cdef.block_size, block_size);
	ASSERT_EQ(class_info.pages, 1);
	ASSERT_EQ(class_info.blocks, class_info.cdef.page_blocks);

	ASSERT_EQ(class_info.blocks_free, class_info.cdef.page_blocks - 1);
	ASSERT_EQ(class_info.slots_free, 1);
	ASSERT_EQ(class_info.slots_free_size, block_size - HEADER_CLASS_SIZE);

	imdb_hndlr_t hcur;
	void* ptrs[10];
	os_memset(ptrs, 0, sizeof(ptrs));
	uint16 rcnt;
	ret = imdb_class_query(hmdb, hcls, PATH_NONE, &hcur);
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
	imdb_errcode_t ret = imdb_clsobj_insert(hmdb, hcls, &ptr, obj_size_div - 1);
	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);

	imdb_class_info_t class_info;
	imdb_class_info(hmdb, hcls, &class_info);

	ASSERT_EQ(class_info.pages, 1);
	ASSERT_EQ(class_info.blocks_free, class_info.cdef.page_blocks - 1);
	ASSERT_EQ(class_info.slots_free, 1);
	ASSERT_EQ(class_info.slots_free_size, block_size - HEADER_CLASS_SIZE - (obj_size_div + SLOT_TYPE4_SIZE));
}

TEST_F(IMDBVariableClass, Insert128)
{
	void* ptr;

	imdb_errcode_t ret;
	uint32 i;
	uint32 sum_a = 0;
	for (i = 0; i < 128; i++) {
		ret = imdb_clsobj_insert(hmdb, hcls, &ptr, obj_size_div*(1 + i % 16));
		ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
		os_memcpy(ptr, &i, sizeof(uint32));
                sum_a += i;
	}

	imdb_info_t imdb_inf;
	imdb_class_info_t info_array[10];
	imdb_info(hmdb, &imdb_inf, &info_array[0], 10);

	imdb_class_info_t class_info;
	imdb_class_info(hmdb, hcls, &class_info);

	ASSERT_EQ(class_info.pages, 3);
	ASSERT_EQ(class_info.blocks, 3*class_info.cdef.page_blocks);
	ASSERT_EQ(class_info.blocks_free, 5);
	ASSERT_EQ(class_info.slots_free, 19);
	ASSERT_EQ(class_info.fl_skip_count, 99);
	ASSERT_EQ(class_info.slots_free_size, 6544);

	imdb_hndlr_t hcur;
	void* ptrs[32];
	os_memset(ptrs, 0, sizeof(ptrs));
	uint16 rcnt;
	ret = imdb_class_query(hmdb, hcls, PATH_NONE, &hcur);
	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);

	uint32 sum_b = 0;
	while (ret == IMDB_ERR_SUCCESS) {
		ret = imdb_class_fetch(hcur, 32, &rcnt, ptrs);
		if (ret == IMDB_CURSOR_NO_DATA_FOUND) {
			ASSERT_EQ(rcnt, 0);
			break;
		}
		ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
		ASSERT_EQ(rcnt, 32);
		uint32 j;
		for (i = 0; i < rcnt; i++) {
			os_memcpy(&j, (void*)ptrs[i], sizeof(uint32));
			sum_b += j;
		}
	}
        ASSERT_EQ(sum_b, sum_a);

        sum_b = 0;
        ASSERT_EQ (imdb_class_forall (hmdb, hcls, &sum_b, forall_sum), IMDB_ERR_SUCCESS);
        ASSERT_EQ(sum_b, sum_a);

	ASSERT_EQ (imdb_class_close(hcur), IMDB_ERR_SUCCESS);
}


TEST_F(IMDBVariableRecycleClass, PageFillAndRecycle)
{
	void* ptr;

	imdb_errcode_t ret;
	int i;
	for (i = 0; i < 40; i++) {
		ret = imdb_clsobj_insert(hmdb, hcls, &ptr, obj_size_div*(1 + i % 16));
		ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
		os_memcpy(ptr, &i, sizeof(i));
	}

	imdb_info_t imdb_inf;
	imdb_info(hmdb, &imdb_inf, NULL, 0);
	ASSERT_EQ(imdb_inf.stat.block_recycle, 2);

	imdb_class_info_t class_info;
	imdb_class_info(hmdb, hcls, &class_info);

	ASSERT_EQ(class_info.pages, 1);
	ASSERT_EQ(class_info.blocks, class_info.cdef.page_blocks);
	ASSERT_EQ(class_info.blocks_free, 0);
	ASSERT_EQ(class_info.slots_free, 1);
	//ASSERT_EQ(class_info.slots_free_size, block_size - HEADER_PAGE_SIZE - (obj_size + SLOT_TYPE2_SIZE));

	// query elements
	imdb_hndlr_t hcur;
	void* ptrs[25];
	os_memset(ptrs, 0, sizeof(ptrs));
	uint16 rcnt;
	ret = imdb_class_query(hmdb, hcls, PATH_NONE, &hcur);
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

	// count elements
        uint32 cnt = 0;
        ASSERT_EQ (imdb_class_forall (hmdb, hcls, &cnt, imdb_forall_count), IMDB_ERR_SUCCESS);
        ASSERT_EQ(cnt, 26);
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
		imdb_clsobj_insert(hmdb, hcls_32, &ptr, 0);
		imdb_clsobj_insert(hmdb, hcls_256, &ptr, 0);
		imdb_clsobj_insert(hmdb, hcls_64, &ptr, 0);
	}
}
#endif


TEST_F(IMDBFileClass, BasicOperations)
{
	void* ptr;
	imdb_errcode_t ret;
	uint32 i;

	void* ptr_del;
	ASSERT_EQ(imdb_clsobj_insert(hfdb, hcls, &ptr, sizeof(uint32)), IMDB_ERR_SUCCESS);
	i = 1; os_memcpy(ptr, &i, sizeof(uint32));
	ASSERT_EQ(imdb_clsobj_insert(hfdb, hcls, &ptr_del, sizeof(uint32)), IMDB_ERR_SUCCESS);
	i = 2; os_memcpy(ptr_del, &i, sizeof(uint32));
	ASSERT_EQ(imdb_clsobj_insert(hfdb, hcls, &ptr, sizeof(uint32)), IMDB_ERR_SUCCESS);
	i = 3; os_memcpy(ptr, &i, sizeof(uint32));

	ASSERT_EQ(imdb_clsobj_delete(hfdb, hcls, ptr_del), IMDB_ERR_SUCCESS);

	imdb_done (hfdb);

	imdb_def_t db_def2 = { block_size, BLOCK_CRC_NONE, true, 3, 256 };
	imdb_init(&db_def2, &hfdb);

	imdb_hndlr_t	hcls2;
        imdb_class_find (hfdb, "testobj", &hcls2);
        ASSERT_EQ(hcls, hcls2);

	imdb_hndlr_t hcur;
	void* ptrs[10];
	os_memset(ptrs, 0, sizeof(ptrs));
	uint16 rcnt;
	ret = imdb_class_query(hfdb, hcls2, PATH_NONE, &hcur);
	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);

	uint32 sum = 0;
	uint32 tcnt = 0;
	while (ret == IMDB_ERR_SUCCESS) {
		ret = imdb_class_fetch(hcur, 10, &rcnt, ptrs);
                tcnt += rcnt;

		uint32 j = 0;
		for (i = 0; i < rcnt; i++) {
			os_memcpy(&j, (void*)ptrs[i], sizeof(uint32));
                        sum += j;
		}
	}
	ASSERT_EQ(tcnt, 2);
	ASSERT_EQ(sum, 4);
}

TEST_F(IMDBFileClass, BufferCacheTest)
{
	void* ptr;

	imdb_errcode_t ret;
	uint32 i;
	uint32 sum_a = 0;
	for (i = 0; i < 128; i++) {
		ret = imdb_clsobj_insert(hfdb, hcls, &ptr, obj_size_div*(1 + i % 16));
		ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
		os_memcpy(ptr, &i, sizeof(uint32));
                sum_a += i;
	}

	ASSERT_EQ (imdb_flush (hfdb), IMDB_ERR_SUCCESS);

	imdb_info_t imdb_inf;
	imdb_class_info_t info_array[10];
	imdb_info(hfdb, &imdb_inf, &info_array[0], 10);

        d_log_iprintf ("imdb", "stat block r/w: %u/%u, header r/w: %u/%u", imdb_inf.stat.block_read, imdb_inf.stat.block_write, imdb_inf.stat.header_read, imdb_inf.stat.header_write);
	ASSERT_EQ(imdb_inf.stat.block_write, 29);
	ASSERT_EQ(imdb_inf.stat.header_write, imdb_inf.stat.header_read);
	ASSERT_EQ(imdb_inf.stat.header_write, 5);

	imdb_class_info_t class_info;
	imdb_class_info(hfdb, hcls, &class_info);

	imdb_info(hfdb, &imdb_inf, &info_array[0], 10);
        d_log_iprintf ("imdb", "stat block r/w: %u/%u, header r/w: %u/%u", imdb_inf.stat.block_read, imdb_inf.stat.block_write, imdb_inf.stat.header_read, imdb_inf.stat.header_write);

	ASSERT_EQ(class_info.pages, 3);
	ASSERT_EQ(class_info.blocks, 3*class_info.cdef.page_blocks);
	ASSERT_EQ(class_info.blocks_free, 5);
	ASSERT_EQ(class_info.slots_free, 19);
	ASSERT_EQ(class_info.fl_skip_count, 21);
	ASSERT_EQ(class_info.slots_free_size, 6544);
	ASSERT_EQ(imdb_inf.stat.block_write, 29);


	imdb_hndlr_t hcur;
	void* ptrs[32];
	os_memset(ptrs, 0, sizeof(ptrs));
	uint16 rcnt;
	ret = imdb_class_query(hfdb, hcls, PATH_NONE, &hcur);
	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);

	uint32 sum_b = 0;
	uint32 tcnt = 0;
	while (ret == IMDB_ERR_SUCCESS) {
		ret = imdb_class_fetch(hcur, 32, &rcnt, ptrs);
                tcnt += rcnt;

		uint32 j = 0;
		for (i = 0; i < rcnt; i++) {
			os_memcpy(&j, (void*)ptrs[i], sizeof(uint32));
                        sum_b += j;
		}
	}
	ASSERT_EQ(tcnt, 128);
	ASSERT_EQ(sum_b, sum_a);

	imdb_info(hfdb, &imdb_inf, &info_array[0], 10);
        d_log_iprintf ("imdb", "stat block r/w: %u/%u, header r/w: %u/%u", imdb_inf.stat.block_read, imdb_inf.stat.block_write, imdb_inf.stat.header_read, imdb_inf.stat.header_write);
	ASSERT_EQ(imdb_inf.stat.block_write, 29);
	ASSERT_EQ(imdb_inf.stat.header_write, imdb_inf.stat.header_read);
	ASSERT_EQ(imdb_inf.stat.header_write, 5);

        sum_b = 0;
        ASSERT_EQ (imdb_class_forall (hfdb, hcls, &sum_b, forall_sum), IMDB_ERR_SUCCESS);
        ASSERT_EQ(sum_b, sum_a);

	ret = imdb_class_close(hcur);
	ASSERT_EQ(ret, IMDB_ERR_SUCCESS);
}
