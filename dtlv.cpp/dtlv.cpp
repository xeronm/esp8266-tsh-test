#include "gtest/gtest.h"

extern "C" {
	#include "proto/dtlv.h"
	#include "core/logging.h"
	#include "core/utils.h"
}


class DtlvClass : public ::testing::Test {
protected:
	void SetUp()
	{
		buflen = 65535;
		buffer = (char*)os_malloc(buflen);
	}

	void TearDown()
	{
		os_free(buffer);
	}

	char*		buffer;
	dtlv_size_t buflen;
};

TEST_F(DtlvClass, BasicEncodingDecoding)
{
	char* stravp = "test_octets_avp\0";
	char buf[512];
	char* resjson = "{\"1\":240,\"2\":65520,\"3\":4294967280,\"4\":\"test_octets_avp\",\"5\":\"746573745f6f63746574735f61767000\"}";	

	dtlv_ctx_t	dtlv_ctx;
	ASSERT_EQ( dtlv_ctx_init_encode(&dtlv_ctx, buffer, buflen), DTLV_ERR_SUCCESS);
	ASSERT_EQ( dtlv_avp_encode_uint8(&dtlv_ctx, 1, 0xF0), DTLV_ERR_SUCCESS);
	ASSERT_EQ( dtlv_avp_encode_uint16(&dtlv_ctx, 2, 0xFFF0), DTLV_ERR_SUCCESS);
	ASSERT_EQ( dtlv_avp_encode_uint32(&dtlv_ctx, 3, 0xFFFFFFF0), DTLV_ERR_SUCCESS);
	ASSERT_EQ( dtlv_avp_encode_char(&dtlv_ctx, 4, stravp), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_avp_encode_octets(&dtlv_ctx, 5, os_strlen(stravp)+1, stravp), DTLV_ERR_SUCCESS);

	dtlv_davp_t avp;
	uint8 vuint8;
	uint16 vuint16;
	uint32 vuint32;
	ASSERT_EQ(dtlv_ctx_init_decode(&dtlv_ctx, buffer, dtlv_ctx.datalen), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_avp_decode(&dtlv_ctx, &avp), DTLV_ERR_SUCCESS);
	ASSERT_EQ(avp.havpd.datatype, DTLV_TYPE_INTEGER);
	ASSERT_EQ(dtlv_avp_get_uint8(&avp, &vuint8), DTLV_ERR_SUCCESS);
	ASSERT_EQ(vuint8, 0xF0);
	ASSERT_EQ(dtlv_avp_decode(&dtlv_ctx, &avp), DTLV_ERR_SUCCESS);
	ASSERT_EQ(avp.havpd.datatype, DTLV_TYPE_INTEGER);
	ASSERT_EQ(dtlv_avp_get_uint16(&avp, &vuint16), DTLV_ERR_SUCCESS);
	ASSERT_EQ(vuint16, 0xFFF0);
	ASSERT_EQ(dtlv_avp_decode(&dtlv_ctx, &avp), DTLV_ERR_SUCCESS);
	ASSERT_EQ(avp.havpd.datatype, DTLV_TYPE_INTEGER);
	ASSERT_EQ(dtlv_avp_get_uint32(&avp, &vuint32), DTLV_ERR_SUCCESS);
	ASSERT_EQ(vuint32, 0xFFFFFFF0);
	ASSERT_EQ(dtlv_avp_decode(&dtlv_ctx, &avp), DTLV_ERR_SUCCESS);
	ASSERT_EQ(avp.havpd.datatype, DTLV_TYPE_CHAR);
	ASSERT_STREQ(stravp, avp.avp->data);

	ASSERT_EQ(dtlv_avp_decode(&dtlv_ctx, &avp), DTLV_ERR_SUCCESS);
	ASSERT_EQ(avp.havpd.datatype, DTLV_TYPE_OCTETS);
	ASSERT_STREQ(stravp, avp.avp->data);

	ASSERT_EQ(dtlv_ctx_init_decode(&dtlv_ctx, buffer, dtlv_ctx.datalen), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_decode_to_json(&dtlv_ctx, buf), DTLV_ERR_SUCCESS);
	ASSERT_STREQ(buf, resjson);
}

TEST_F(DtlvClass, ListEncodingDecoding)
{
	char buf[512];
	char* resjson = "{\"1\":240,\"1\":240,\"2\":[241,242,243]}";
	dtlv_ctx_t	dtlv_ctx;

	ASSERT_EQ(dtlv_ctx_init_encode(&dtlv_ctx, buffer, buflen), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_avp_encode_uint8(&dtlv_ctx, 1, 0xF0), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_avp_encode_uint8(&dtlv_ctx, 1, 0xF0), DTLV_ERR_SUCCESS);

	dtlv_avp_t* gavp1;
	os_memset(&gavp1, 0, sizeof(dtlv_avp_t));
	ASSERT_EQ(dtlv_avp_encode_list(&dtlv_ctx, 0, 2, DTLV_TYPE_INTEGER, &gavp1), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_avp_encode_uint8(&dtlv_ctx, 2, 0xF1), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_avp_encode_uint8(&dtlv_ctx, 2, 0xF2), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_avp_encode_uint8(&dtlv_ctx, 2, 0xF3), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_avp_encode_uint8(&dtlv_ctx, 4, 0xF4), DTLV_PATH_ERROR);
	ASSERT_EQ(dtlv_avp_encode_group_done(&dtlv_ctx, gavp1), DTLV_ERR_SUCCESS);

	ASSERT_EQ(dtlv_ctx_init_decode(&dtlv_ctx, buffer, dtlv_ctx.datalen), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_decode_to_json(&dtlv_ctx, buf), DTLV_ERR_SUCCESS);
	ASSERT_STREQ(buf, resjson);
}

TEST_F(DtlvClass, GroupingEncodingDecoding)
{
	char* stravp = "test_octets_avp\0";
	char buf[512];
	char* resjson = "{\"1\":240,\"2\":65520,\"3\":{\"1\":240,\"2\":65520,\"3\":4294967280,\"4\":\"746573745f6f63746574735f617670\"},\"63.4\":{\"1\":240,\"2\":65520,\"3\":4294967280,\"4\":\"746573745f6f63746574735f617670\"},\"5\":65520,\"6\":4294967280,\"7\":\"746573745f6f63746574735f617670\"}";	

	dtlv_ctx_t dtlv_ctx;
	ASSERT_EQ( dtlv_ctx_init_encode(&dtlv_ctx, buffer, buflen), DTLV_ERR_SUCCESS);
	ASSERT_EQ( dtlv_avp_encode_uint8(&dtlv_ctx, 1, 0xF0), DTLV_ERR_SUCCESS);
	ASSERT_EQ( dtlv_avp_encode_uint16(&dtlv_ctx, 2, 0xFFF0), DTLV_ERR_SUCCESS);

	dtlv_avp_t* gavp1;
	os_memset(&gavp1, 0, sizeof(dtlv_avp_t));
	ASSERT_EQ(dtlv_avp_encode_grouping(&dtlv_ctx, 0, 3, &gavp1), DTLV_ERR_SUCCESS);
	ASSERT_EQ( dtlv_avp_encode_uint8(&dtlv_ctx, 1, 0xF0), DTLV_ERR_SUCCESS);
	ASSERT_EQ( dtlv_avp_encode_uint16(&dtlv_ctx, 2, 0xFFF0), DTLV_ERR_SUCCESS);
	ASSERT_EQ( dtlv_avp_encode_uint32(&dtlv_ctx, 3, 0xFFFFFFF0), DTLV_ERR_SUCCESS);
	ASSERT_EQ( dtlv_avp_encode_octets(&dtlv_ctx, 4, os_strlen(stravp), stravp), DTLV_ERR_SUCCESS);
	ASSERT_EQ( dtlv_avp_encode_group_done(&dtlv_ctx, gavp1), DTLV_ERR_SUCCESS);

	dtlv_avp_t* gavp2;
	os_memset(&gavp2, 0, sizeof(dtlv_avp_t));
	ASSERT_EQ( dtlv_avp_encode(&dtlv_ctx, 63, 4, DTLV_TYPE_OBJECT, 0, false, &gavp2), DTLV_ERR_SUCCESS);
	ASSERT_EQ( dtlv_avp_encode_uint8(&dtlv_ctx, 1, 0xF0), DTLV_ERR_SUCCESS);
	ASSERT_EQ( dtlv_avp_encode_uint16(&dtlv_ctx, 2, 0xFFF0), DTLV_ERR_SUCCESS);
	ASSERT_EQ( dtlv_avp_encode_uint32(&dtlv_ctx, 3, 0xFFFFFFF0), DTLV_ERR_SUCCESS);
	ASSERT_EQ( dtlv_avp_encode_octets(&dtlv_ctx, 4, os_strlen(stravp), stravp), DTLV_ERR_SUCCESS);
	ASSERT_EQ( dtlv_avp_encode_group_done(&dtlv_ctx, gavp2), DTLV_ERR_SUCCESS);

	ASSERT_EQ( dtlv_avp_encode_uint16(&dtlv_ctx, 5, 0xFFF0), DTLV_ERR_SUCCESS);
	ASSERT_EQ( dtlv_avp_encode_uint32(&dtlv_ctx, 6, 0xFFFFFFF0), DTLV_ERR_SUCCESS);
	ASSERT_EQ( dtlv_avp_encode_octets(&dtlv_ctx, 7, os_strlen(stravp), stravp), DTLV_ERR_SUCCESS);

	dtlv_davp_t avp;
	uint8 vuint8;
	ASSERT_EQ(dtlv_ctx_init_decode(&dtlv_ctx, buffer, dtlv_ctx.datalen), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_avp_decode(&dtlv_ctx, &avp), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_avp_decode(&dtlv_ctx, &avp), DTLV_ERR_SUCCESS);

	ASSERT_EQ(dtlv_avp_decode(&dtlv_ctx, &avp), DTLV_ERR_SUCCESS);
	ASSERT_EQ(avp.havpd.datatype, DTLV_TYPE_OBJECT);
	ASSERT_EQ(dtlv_avp_decode(&dtlv_ctx, &avp), DTLV_ERR_SUCCESS);
	ASSERT_EQ(avp.havpd.datatype, DTLV_TYPE_OBJECT);
	ASSERT_EQ(avp.havpd.nscode.comp.namespace_id, 63);
	ASSERT_EQ(avp.havpd.nscode.comp.code, 4);

	dtlv_ctx_t	dtlv_ctx2;
	ASSERT_EQ(dtlv_ctx_init_decode(&dtlv_ctx2, avp.avp->data, d_avp_data_length(avp.havpd.length)), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_avp_decode(&dtlv_ctx2, &avp), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_avp_get_uint8(&avp, &vuint8), DTLV_ERR_SUCCESS);
	ASSERT_EQ(vuint8, 0xF0);

	ASSERT_EQ(dtlv_ctx_init_decode(&dtlv_ctx, buffer, dtlv_ctx.datalen), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_decode_to_json(&dtlv_ctx, buf), DTLV_ERR_SUCCESS);
	ASSERT_STREQ(buf, resjson);

}

TEST_F(DtlvClass, ErrorDecoding)
{
	char buf[512];
	size_t len;
	len = hex2buf(buffer, buflen, "00000000");

	char* resjson = "{}";

	dtlv_ctx_t	dtlv_ctx;
	ASSERT_EQ(dtlv_ctx_init_decode(&dtlv_ctx, buffer, len), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_decode_to_json(&dtlv_ctx, buf), DTLV_AVP_INVALID_LEN);
	ASSERT_STREQ(buf, resjson);

	len = hex2buf(buffer, buflen, "00050000");
	ASSERT_EQ(dtlv_ctx_init_decode(&dtlv_ctx, buffer, len), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_decode_to_json(&dtlv_ctx, buf), DTLV_AVP_OUT_OF_BOUNDS);
	ASSERT_STREQ(buf, resjson);

	char* resjson2 = "{\"0\":null}";
	len = hex2buf(buffer, buflen, "00040000");
	ASSERT_EQ(dtlv_ctx_init_decode(&dtlv_ctx, buffer, len), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_decode_to_json(&dtlv_ctx, buf), DTLV_ERR_SUCCESS);
	ASSERT_STREQ(buf, resjson2);
}


TEST_F(DtlvClass, DecodeByPath)
{
	dtlv_ctx_t	dtlv_ctx;
	ASSERT_EQ(dtlv_ctx_init_encode(&dtlv_ctx, buffer, buflen), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_avp_encode_uint8(&dtlv_ctx, 1, 0xF0), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_avp_encode_uint16(&dtlv_ctx, 2, 0xFFF0), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_avp_encode_uint32(&dtlv_ctx, 3, 0xFFFFFFF0), DTLV_ERR_SUCCESS);

	dtlv_avp_t* gavp1;
	os_memset(&gavp1, 0, sizeof(dtlv_avp_t));
	ASSERT_EQ(dtlv_avp_encode_grouping(&dtlv_ctx, 0, 10, &gavp1), DTLV_ERR_SUCCESS);

	dtlv_avp_t* gavp2;
	os_memset(&gavp2, 0, sizeof(dtlv_avp_t));
	ASSERT_EQ(dtlv_avp_encode_grouping(&dtlv_ctx, 0, 11, &gavp2), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_avp_encode_uint8(&dtlv_ctx, 1, 0xF1), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_avp_encode_uint16(&dtlv_ctx, 2, 0xFFF1), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_avp_encode_uint32(&dtlv_ctx, 3, 0xFFFFFFF1), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_avp_encode_group_done(&dtlv_ctx, gavp2), DTLV_ERR_SUCCESS);

	os_memset(&gavp2, 0, sizeof(dtlv_avp_t));
	ASSERT_EQ(dtlv_avp_encode_grouping(&dtlv_ctx, 0, 11, &gavp2), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_avp_encode_uint8(&dtlv_ctx, 1, 0xF2), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_avp_encode_uint16(&dtlv_ctx, 2, 0xFFF2), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_avp_encode_uint32(&dtlv_ctx, 3, 0xFFFFFFF2), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_avp_encode_group_done(&dtlv_ctx, gavp2), DTLV_ERR_SUCCESS);

	ASSERT_EQ(dtlv_avp_encode_uint16(&dtlv_ctx, 4, 0xFFF3), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_avp_encode_uint16(&dtlv_ctx, 5, 0xFFF4), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_avp_encode_group_done(&dtlv_ctx, gavp1), DTLV_ERR_SUCCESS);

	dtlv_nscode_t path[] = { {0, 10}, {0, 11}, {0, 0} };
	dtlv_nscode_t path2[] = { {0, 10}, {0, 11}, {0, 3}, {0, 0} };

	dtlv_davp_t avp_array[10];
	uint16 total_count;

	ASSERT_EQ(dtlv_ctx_init_decode(&dtlv_ctx, buffer, dtlv_ctx.datalen), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_avp_decode_bypath(&dtlv_ctx, path, avp_array, 10, false, &total_count), DTLV_ERR_SUCCESS);
	ASSERT_EQ(total_count, 2);

	ASSERT_EQ(dtlv_ctx_init_decode(&dtlv_ctx, buffer, dtlv_ctx.datalen), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_avp_decode_bypath(&dtlv_ctx, path2, avp_array, 10, false, &total_count), DTLV_ERR_SUCCESS);
	ASSERT_EQ(total_count, 2);
	uint32 vuint32;
	ASSERT_EQ(dtlv_avp_get_uint32(&avp_array[0], &vuint32), DTLV_ERR_SUCCESS);
	ASSERT_EQ(vuint32, 0xFFFFFFF1);
	ASSERT_EQ(dtlv_avp_get_uint32(&avp_array[1], &vuint32), DTLV_ERR_SUCCESS);
	ASSERT_EQ(vuint32, 0xFFFFFFF2);

	// total_count > array_length
	ASSERT_EQ(dtlv_ctx_init_decode(&dtlv_ctx, buffer, dtlv_ctx.datalen), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_avp_decode_bypath(&dtlv_ctx, path2, avp_array, 1, false, &total_count), DTLV_ERR_SUCCESS);
	ASSERT_EQ(total_count, 2);

	// limit_count test
	ASSERT_EQ(dtlv_ctx_init_decode(&dtlv_ctx, buffer, dtlv_ctx.datalen), DTLV_ERR_SUCCESS);
	ASSERT_EQ(dtlv_avp_decode_bypath(&dtlv_ctx, path2, avp_array, 1, true, &total_count), DTLV_FORALL_BREAK);
	ASSERT_EQ(total_count, 1);
}