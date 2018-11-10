#include "gtest/gtest.h"

extern "C" {
	#include "system/imdb.h"
	#include "service/lsh.h"
	#include "crypto/crc.h"
}

#define BUFFER_LENGTH		2048
//#define LSH_PRINT_DUMP

void check_dump(sh_hndlr_t hstmt, char* buf, uint16 crc_expect) {
	ASSERT_EQ(
		stmt_dump(hstmt, buf, BUFFER_LENGTH, true), SH_ERR_SUCCESS);
	size_t len = strlen(buf);
	uint16 crc_dump = crc16((unsigned char*)buf, len);

	if (crc_expect) {
#ifdef LSH_PRINT_DUMP
            if (crc_expect != crc_dump) {
		sh_stmt_info_t info;
		stmt_info(hstmt, &info);
		printf("bytecode dump (dlen:%u, crc:%u, bclen:%u):\n%s\n", info.length, crc_dump, len, buf);
            }
#endif
	    ASSERT_EQ(crc_dump, crc_expect);
	}

	stmt_free(hstmt);
}

class LshClassSimple : public ::testing::Test {
protected:
	void SetUp()
	{
		hmdb = 0;
		imdb_def_t db_def = { 0, BLOCK_CRC_NONE, false, 0, 0 };
		imdb_init(&db_def, 0, &hmdb);
		imdb_class_def_t	cdef = { "data", false, true, false, 10, 1, 4, 4, 8 };
		imdb_class_create(hmdb, &cdef, &hdata);
	}
	void TearDown()
	{
		imdb_done(hmdb);
	}

	char			buf[BUFFER_LENGTH];
	sh_hndlr_t		hstmt;
	imdb_hndlr_t	hmdb;
	imdb_hndlr_t	hdata;
};

class LshClass : public ::testing::Test {
protected:
	void SetUp()
	{
		hmdb = 0;
		imdb_def_t db_def = { 0, BLOCK_CRC_NONE, false, 0, 0 };
		imdb_init(&db_def, 0, &hmdb);
		imdb_class_def_t	cdef = { "data", false, true, false, 10, 1, 4, 4, 8 };
		imdb_class_create(hmdb, &cdef, &hdata);

                svcres = { hmdb, 0, hdata };
		lsh_on_start((const svcs_resource_t *) &svcres, NULL);
	}
	void TearDown()
	{
		lsh_on_stop();

		imdb_done(hmdb);
	}

        svcs_resource_t svcres;
	char			buf[BUFFER_LENGTH];
	sh_hndlr_t		hstmt;
	imdb_hndlr_t	hmdb;
	imdb_hndlr_t	hdata;
};


TEST_F(LshClassSimple, ServiceControl)
{
        svcs_resource_t svcres = { hmdb, 0, hdata };
	ASSERT_EQ(
		lsh_on_start((const svcs_resource_t *) &svcres, NULL), SVCS_ERR_SUCCESS);
	ASSERT_EQ(
		lsh_on_stop(), SVCS_ERR_SUCCESS);
}

TEST_F(LshClass, AtomicOperations)
{
	ASSERT_EQ(
		stmt_parse(" 2 + 5 ", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 5856);

	ASSERT_EQ(
		stmt_parse(" 2 * 5 ", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 9043);

	ASSERT_EQ(
		stmt_parse("!1", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 34390);
}

TEST_F(LshClass, FunctionCall)
{
	ASSERT_EQ(
		stmt_parse("func1()", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 19900);

	ASSERT_EQ(
		stmt_parse("func2( 5 )", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 57896);

	ASSERT_EQ(
		stmt_parse("func3( 5, \"abc\", 10 )", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 24607);
}


TEST_F(LshClass, VariableDeclareAndAssignment)
{
	ASSERT_EQ(
		stmt_parse("# var2; # var3; ## var0;", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 24164);

	// Fixme: should optimized
	ASSERT_EQ(
		stmt_parse("# var1 := 10", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 43893);

	ASSERT_EQ(
		stmt_parse("## var2 := 5", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 62956);

	ASSERT_EQ(
		stmt_parse("#var1 := get_value();", "", &hstmt), SH_ERR_SUCCESS);

	// Fixme: wrong result
	ASSERT_EQ(
		stmt_parse("var1 := get_value();", "", &hstmt), SH_CODE_VARIABLE_UNDEF);
}

TEST_F(LshClass, VariableUsage)
{
	ASSERT_EQ(
		stmt_parse("# var1 := 10; func1( var1 )", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 54353);

	ASSERT_EQ(
		stmt_parse("## var1 := 10; func1( var1 )", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 53107);

	ASSERT_EQ(
		stmt_parse("## var1 := 5; func2( var1 )", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 7412);
}

TEST_F(LshClass, ControlsOperations)
{
	ASSERT_EQ(
		stmt_parse("2*3;4+5", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 37019);

	ASSERT_EQ(
		stmt_parse("2+3 ? 4*5 ; 6-7", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 9222);

 	ASSERT_EQ(
		stmt_parse("2+3 ? 4*5 : 6-7", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 61580);

	ASSERT_EQ(
		stmt_parse("(2 < 3) ?? { 4*5 } : { 6-7; }", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 47490);
}

TEST_F(LshClass, ConditionalResult)
{
	// Fixme: wrong results
	ASSERT_EQ(
		stmt_parse("call( 2+3 ? 4*5 : 6-7 )", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 1);

	ASSERT_EQ(
		stmt_parse("# var1; var1 := (2=1) ? 1 : 0", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 1);

	ASSERT_EQ(
		stmt_parse("# var1; (2=1) ? var1 := 1 : 0", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 1);
}

TEST_F(LshClass, PriorityOperatons)
{
	ASSERT_EQ(
		stmt_parse(" 2 * 5 * 3 ", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 23654);

	ASSERT_EQ(
		stmt_parse(" 2 + 5 + 3 ", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 14701);

	ASSERT_EQ(
		stmt_parse("2 + 5*3", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 59608);
	ASSERT_EQ(
		stmt_parse("2*5 + 3", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 25575);

	ASSERT_EQ(
		stmt_parse("!2 + 5*3 + 6*1", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 312);

 	ASSERT_EQ(
		stmt_parse("2*5 + 3*6 + !1", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 18775);


	ASSERT_EQ(
		stmt_parse("2*(5 + 3*4)", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 25590);

	ASSERT_EQ(
		stmt_parse("(2*4 + 5)*3", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 63840);

	ASSERT_EQ(
		stmt_parse("2*((2*4 + 5) + 3) + 7", "", &hstmt), SH_ERR_SUCCESS); // Fixme: May optimized
	check_dump(hstmt, buf, 53025);

	ASSERT_EQ(
		stmt_parse("( 2 + 3 ) * 5 + (7)", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 22344);
}

TEST_F(LshClass, ComplexStatement)
{
	ASSERT_EQ(
		stmt_parse(
"## last_sdt; # sdt := sysdate(); print(last_sdt, sdt - last_sdt); (sdt % 2 = 0) ? print(0) : print(1); last_sdt := sdt;",
			"", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 58296);

	ASSERT_EQ(
		stmt_parse(
"	## last_dt;"
"	## last_ev;"
"	# hum := dht11_get_humidity();"
"	# relay_state := gpioget(12);"
"	# sys_date := sysdate();"
"	(last_ev = 0) ??"
"		{ last_ev := relay_state + 1 ; last_dt := sys_date };"
"	((hum >= 6000) && (last_dt + 30 < sys_date)) ??"
"		{ gpioset(12, 1); last_ev := 1; last_dt := sys_date };"
"	((hum < 5800) && (last_dt + 30 < sys_date)) ??"
"		{ gpioset(12, 0); last_ev := 2; last_dt := sys_date };"
"	((last_ev = 1) && (last_dt + 600 < sys_date)) ??"
"		{ gpioset(12, 0); last_ev := 3; last_dt := sys_date };",
			"", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 0);

	imdb_info_t imdb_inf;
	imdb_class_info_t info_array[10];
	imdb_info(hmdb, &imdb_inf, &info_array[0], 10);
}