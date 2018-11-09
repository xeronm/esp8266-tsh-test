#include "gtest/gtest.h"

extern "C" {
	#include "system/imdb.h"
	#include "service/lsh.h"
	#include "crypto/crc.h"
}

#define BUFFER_LENGTH		2048
#define LSH_PRINT_DUMP

void check_dump(sh_hndlr_t hstmt, char* buf, uint16 crc) {
	ASSERT_EQ(
		stmt_dump(hstmt, buf, BUFFER_LENGTH, true), SH_ERR_SUCCESS);
	size_t len = strlen(buf);
	uint16 crcd = crc16((unsigned char*)buf, len);

#ifdef LSH_PRINT_DUMP
	sh_stmt_info_t info;
	stmt_info(hstmt, &info);
	printf("bytecode dump (slen:%u, crc:%u, dlen:%u):\n%s\n", info.length, crcd, len, buf);
#endif
	if (crc) {
		ASSERT_EQ(crcd, crc);
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

		lsh_on_start(hmdb, hdata, NULL);
	}
	void TearDown()
	{
		lsh_on_stop();

		imdb_done(hmdb);
	}

	char			buf[BUFFER_LENGTH];
	sh_hndlr_t		hstmt;
	imdb_hndlr_t	hmdb;
	imdb_hndlr_t	hdata;
};


TEST_F(LshClassSimple, ServiceControl)
{
	ASSERT_EQ(
		lsh_on_start(hmdb, hdata, NULL), SVCS_ERR_SUCCESS);
	ASSERT_EQ(
		lsh_on_stop(), SVCS_ERR_SUCCESS);
}

TEST_F(LshClass, AtomicOperations)
{
	ASSERT_EQ(
		stmt_parse(" 2 + 5 ", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 28694);

	ASSERT_EQ(
		stmt_parse(" 2 * 5 ", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 17829);

	ASSERT_EQ(
		stmt_parse("!1", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 36277);

	ASSERT_EQ(
		stmt_parse("func1()", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 18015);

	ASSERT_EQ(
		stmt_parse("func2( 5 )", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 34014);

	ASSERT_EQ(
		stmt_parse("func3( 5, \"abc\", 10 )", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 26302);

	// Fixme: should optimized
	ASSERT_EQ(
		stmt_parse("# var1 := 10", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 8154);
}

TEST_F(LshClass, Variable)
{
	ASSERT_EQ(
		stmt_parse("# var1 := 10; func1( var1 )", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 15147);

	ASSERT_EQ(
		stmt_parse("## var1 := 10; func1( var1 )", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 31971);

	ASSERT_EQ(
		stmt_parse("## var1 := 5; func2( var1 )", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 38461);

	ASSERT_EQ(
		stmt_parse("# var2; # var3;", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 22193);

	// Fixme: wrong result
	ASSERT_EQ(
		stmt_parse("var1 := get_value();", "", &hstmt), SH_CODE_VARIABLE_UNDEF);
}

TEST_F(LshClass, ControlsOperations)
{
	ASSERT_EQ(
		stmt_parse("2*3;4+5", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 33579);

	ASSERT_EQ(
		stmt_parse("2+3 ? 4*5 ; 6-7", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 34002);

 	ASSERT_EQ(
		stmt_parse("2+3 ? 4*5 : 6-7", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 30971);

	ASSERT_EQ(
		stmt_parse("(2 < 3) ?? { 4*5 } : { 6-7; }", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 11990);

	// Fixme: wrong results
	ASSERT_EQ(
		stmt_parse("call( 2+3 ? 4*5 : 6-7 )", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 0);

	ASSERT_EQ(
		stmt_parse("# var1; var1 := (2=1) ? 1 : 0", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 0);

	ASSERT_EQ(
		stmt_parse("# var1; (2=1) ? var1 := 1 : 0", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 0);
}

TEST_F(LshClass, PriorityOperatons)
{
	ASSERT_EQ(
		stmt_parse(" 2 * 5 * 3 ", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 22405);

	ASSERT_EQ(
		stmt_parse(" 2 + 5 + 3 ", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 12942);

	ASSERT_EQ(
		stmt_parse("2 + 5*3", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 37847);
	ASSERT_EQ(
		stmt_parse("2*5 + 3", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 6376);

	ASSERT_EQ(
		stmt_parse("!2 + 5*3 + 6*1", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 20917);

 	ASSERT_EQ(
		stmt_parse("2*5 + 3*6 + !1", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 32249);


	ASSERT_EQ(
		stmt_parse("2*(5 + 3*4)", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 12461);

	ASSERT_EQ(
		stmt_parse("(2*4 + 5)*3", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 58874);

	ASSERT_EQ(
		stmt_parse("2*((2*4 + 5) + 3) + 7", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 43033);

	ASSERT_EQ(
		stmt_parse("( 2 + 3 ) * 5 + (7)", "", &hstmt), SH_ERR_SUCCESS);
	check_dump(hstmt, buf, 19410);
}

TEST_F(LshClass, ComplexStatement)
{
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