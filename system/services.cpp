#include "gtest/gtest.h"

extern "C" {
	#include "system/imdb.h"
	#include "system/services.h"
	#include "core/logging.h"
}


class ServicesClass : public ::testing::Test {
protected:
	void SetUp()
	{
		imdb_def_t db_def = { 1024, BLOCK_CRC_NONE, false, 0, 0 };
		imdb_init(&db_def, &hmdb);

        fio_user_format(0);

		log_severity = log_severity_get ();
        log_severity_set (LOG_INFO);

		imdb_def_t db_def2 = { 1024, BLOCK_CRC_NONE, true, 3, 256 };
		imdb_init(&db_def2, &hfdb);
	}

	void TearDown()
	{
		svcctl_stop();

		imdb_done(hmdb);
		imdb_done(hfdb);

        log_severity_set (log_severity);
	}

	imdb_hndlr_t	hmdb;
	imdb_hndlr_t	hfdb;
	log_severity_t  log_severity;
};

TEST_F(ServicesClass, StartStop)
{
	ASSERT_EQ(svcctl_start(hmdb, 0), SVCS_ERR_SUCCESS);
	ASSERT_EQ(svcctl_start(hmdb, 0), SVCS_ALREADY_RUN);

	ASSERT_EQ(svcctl_stop(), SVCS_ERR_SUCCESS);
	ASSERT_EQ(svcctl_stop(), SVCS_NOT_RUN);

	ASSERT_EQ(svcctl_start(hmdb, 0), SVCS_ERR_SUCCESS);
	ASSERT_EQ(svcctl_stop(), SVCS_ERR_SUCCESS);
}

svcs_errcode_t _on_start(const svcs_resource_t * svcres, dtlv_ctx_t* conf) { return SVCS_ERR_SUCCESS; }
svcs_errcode_t _on_stop() { return SVCS_ERR_SUCCESS; }

TEST_F(ServicesClass, ManageService)
{
	ASSERT_EQ(svcctl_start(hmdb, 0), SVCS_ERR_SUCCESS);

	svcs_service_def_t sdef = {false, false, _on_start, _on_stop, NULL, NULL, 0, NULL };
	ASSERT_EQ(svcctl_service_install(10, "service_A", &sdef), SVCS_ERR_SUCCESS);
	ASSERT_EQ(svcctl_service_install(20, "service_B", &sdef), SVCS_ERR_SUCCESS);

	ASSERT_EQ(svcctl_service_start(0, "service_A"), SVCS_ERR_SUCCESS);
	ASSERT_EQ(svcctl_service_start(0, "service_B"), SVCS_ERR_SUCCESS);

	ASSERT_EQ(svcctl_service_stop(0, "service_A"), SVCS_ERR_SUCCESS);
	ASSERT_EQ(svcctl_service_stop(0, "service_B"), SVCS_ERR_SUCCESS);

	ASSERT_EQ(svcctl_service_uninstall("service_A"), SVCS_ERR_SUCCESS);
	ASSERT_EQ(svcctl_service_uninstall("service_B"), SVCS_ERR_SUCCESS);

	ASSERT_EQ(svcctl_stop(), SVCS_ERR_SUCCESS);
}

TEST_F(ServicesClass, Configuration)
{
	ASSERT_EQ(svcctl_start(hmdb, hfdb), SVCS_ERR_SUCCESS);

	svcs_service_def_t sdef = {true, false, _on_start, _on_stop, NULL, NULL, 0, NULL };
	ASSERT_EQ(svcctl_service_install(10, "service_A", &sdef), SVCS_ERR_SUCCESS);

	char buf[512];
    dtlv_ctx_t conf;
	ASSERT_EQ( dtlv_ctx_init_encode(&conf, buf, 512), DTLV_ERR_SUCCESS);
	ASSERT_EQ( dtlv_avp_encode_uint8(&conf, 1, 0xF0), DTLV_ERR_SUCCESS);
	ASSERT_EQ( dtlv_avp_encode_uint32(&conf, 2, 0xF0), DTLV_ERR_SUCCESS);

	ASSERT_EQ(svcctl_service_conf_set(10, &conf), SVCS_ERR_SUCCESS);
	ASSERT_EQ(svcctl_service_conf_save(10), SVCS_ERR_SUCCESS);

	ASSERT_EQ( dtlv_ctx_init_encode(&conf, buf, 512), DTLV_ERR_SUCCESS);
	ASSERT_EQ( dtlv_avp_encode_uint8(&conf, 1, 0xF1), DTLV_ERR_SUCCESS);
	ASSERT_EQ( dtlv_avp_encode_uint32(&conf, 2, 0xF1), DTLV_ERR_SUCCESS);

	ASSERT_EQ(svcctl_service_conf_set(10, &conf), SVCS_ERR_SUCCESS);
	ASSERT_EQ(svcctl_service_conf_save(10), SVCS_ERR_SUCCESS);

	ASSERT_EQ( dtlv_ctx_init_encode(&conf, buf, 512), DTLV_ERR_SUCCESS);
	ASSERT_EQ( dtlv_avp_encode_uint8(&conf, 1, 0xF2), DTLV_ERR_SUCCESS);
	ASSERT_EQ( dtlv_avp_encode_uint32(&conf, 2, 0xF2), DTLV_ERR_SUCCESS);

	ASSERT_EQ(svcctl_service_conf_set(10, &conf), SVCS_ERR_SUCCESS);
	ASSERT_EQ(svcctl_service_conf_save(10), SVCS_ERR_SUCCESS);

	ASSERT_EQ(svcctl_stop(), SVCS_ERR_SUCCESS);
}

TEST_F(ServicesClass, SetEnabled)
{
	ASSERT_EQ(svcctl_start(hmdb, hfdb), SVCS_ERR_SUCCESS);

	svcs_service_def_t sdef = {true, false, _on_start, _on_stop, NULL, NULL, 0, NULL };
	ASSERT_EQ(svcctl_service_install(10, "service_A", &sdef), SVCS_ERR_SUCCESS);

	char buf[512];
    dtlv_ctx_t conf;
	ASSERT_EQ( dtlv_ctx_init_encode(&conf, buf, 512), DTLV_ERR_SUCCESS);
	ASSERT_EQ( dtlv_avp_encode_uint8(&conf, 1, 0xF0), DTLV_ERR_SUCCESS);

	ASSERT_EQ(svcctl_service_conf_set(10, &conf), SVCS_ERR_SUCCESS);
	ASSERT_EQ(svcctl_service_conf_save(10), SVCS_ERR_SUCCESS);

	ASSERT_EQ(svcctl_service_set_enabled(10, false), SVCS_ERR_SUCCESS);

	ASSERT_EQ(svcctl_stop(), SVCS_ERR_SUCCESS);
}