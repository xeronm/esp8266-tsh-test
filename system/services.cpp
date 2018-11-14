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
	}
	void TearDown()
	{
		imdb_done(hmdb);
		svcctl_stop();
	}

	imdb_hndlr_t	hmdb;
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

	svcs_service_def_t sdef = {false, _on_start, _on_stop, NULL, NULL, 0, NULL };
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
