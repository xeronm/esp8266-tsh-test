#include "gtest/gtest.h"

extern "C" {
	#include "system/imdb.h"
	#include "core/logging.h"
	#include "service/syslog.h"
}


class SyslogClass : public ::testing::Test {
protected:
	void SetUp()
	{
		imdb_def_t db_def = { 1024, BLOCK_CRC_NONE, false, 0, 0 };
		imdb_init(&db_def, 0, &hmdb);
		imdb_class_def_t	cdef = { "data", false, true, false, 10, 1, 4, 4, 8 };
		imdb_class_create(hmdb, &cdef, &hdata);

		dtlv_ctx_init_encode(&dtlv_conf, conf, sizeof(conf));
		dtlv_avp_encode_uint8(&dtlv_conf, SYSLOG_AVP_LOG_SEVERITY, LOGGING_SEVERITY);
	}
	void TearDown()
	{
		imdb_done(hmdb);
	}

	char			conf[256];
        dtlv_ctx_t		dtlv_conf;
	imdb_hndlr_t	hmdb;
	imdb_hndlr_t	hdata;
};

TEST_F(SyslogClass, ServiceNotRun)
{
	log_printf(LOG_INFO, "googletest", "=====================");
	log_printf(LOG_INFO, "googletest", "test message - %d", 1);
	log_printf(LOG_INFO, "googletest", "test message - %d", 2);

	char buf[200];
	int i;
	for (i = 0; i < 20; i++) {
		os_memcpy(&buf[i * 10], "0123456789", 10);
	}
	log_bprintf(LOG_INFO, "googletest", buf, 80, "test message with dump - %d", 3);
	log_bprintf(LOG_INFO, "googletest", buf, 200, "test message with long dump - %d", 4);
	log_printf(LOG_INFO, "googletest", "=====================");
}


TEST_F(SyslogClass, ServiceRunning)
{
        svcs_resource_t svcres = { hmdb, 0, hdata };
	svcs_errcode_t ret = syslog_on_start( (const svcs_resource_t *) &svcres, &dtlv_conf);
	ASSERT_EQ(ret, SVCS_ERR_SUCCESS);

	log_severity_t  log_severity = log_severity_get ();
        log_severity_set (LOG_WARNING);

	log_printf(LOG_INFO, "googletest", "=====================");
	log_printf(LOG_WARNING, "googletest", "=====================");
	log_printf(LOG_WARNING, "googletest", "test message - %d", 1);
	log_printf(LOG_WARNING, "googletest", "test message - %d", 2);

	char buf[200];
	int i;
	for (i = 0; i < 20; i++) {
		os_memcpy(&buf[i * 10], "0123456789", 10);
	}
	log_bprintf(LOG_WARNING, "googletest", buf, 80, "test message with dump - %d", 3);
	log_bprintf(LOG_WARNING, "googletest", buf, 200, "test message with long dump - %d", 4);
	log_printf(LOG_WARNING, "googletest", "=====================");

        log_severity_set (log_severity);

	imdb_hndlr_t hcur;

	ret = syslog_query(&hcur);
	ASSERT_EQ(ret, SVCS_ERR_SUCCESS);

	imdb_errcode_t ret2;
	uint16 rcnt;
	void* ptrs[10];
	ret2 = imdb_class_fetch(hcur, 10, &rcnt, ptrs); 
	ASSERT_EQ(rcnt, 6);
	ASSERT_EQ(ret2, IMDB_CURSOR_NO_DATA_FOUND);
	int j;
	for (i = 0; i < rcnt; i++) {
		syslog_logrec_t* lrec = (syslog_logrec_t*)ptrs[i];
		os_printf("%d.%d: %s\n", lrec->rec_no, lrec->rec_ctime, (char *)&lrec->vardata);
	}

	ret2 = imdb_class_close(hcur);
	ASSERT_EQ(ret2, IMDB_ERR_SUCCESS);

	ret = syslog_on_stop();
	ASSERT_EQ(ret, SVCS_ERR_SUCCESS);
}

