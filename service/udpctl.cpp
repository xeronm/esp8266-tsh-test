#include "gtest/gtest.h"

#define SECRET_KEY1 "udpctl"
#define SECRET_KEY2 "very secret key"

extern "C" {
	#include "core/logging.h"
	#include "system/imdb.h"
	#include "system/comavp.h"
	#include "service/udpctl.h"
	#include "crypto/sha.h"
}


class UdpctlClass : public ::testing::Test {
protected:
	void SetUp()
	{
		buflen = 4096;
		buffer_in = (char*)os_malloc(buflen);
		buffer_out = (char*)os_malloc(buflen);

		port = 3902;
		addr.addr = 0x050AA8C0;

		dtlv_ctx_init_encode(&dtlv_conf, conf, sizeof(conf));
		dtlv_avp_encode_octets(&dtlv_conf, UDPCTL_AVP_SECRET, os_strlen(SECRET_KEY1), SECRET_KEY1);

		imdb_def_t db_def = { 1024, BLOCK_CRC_NONE, false, 0, 0 };
		imdb_init(&db_def, 0, &hmdb);
		imdb_class_def_t	cdef = { "data", false, true, false, 10, 1, 4, 4, 8 };
		imdb_class_create(hmdb, &cdef, &hdata);
	}

	void TearDown()
	{
		imdb_done(hmdb);
		os_free(buffer_in);
		os_free(buffer_out);
	}

	void EncodeAuth(bool valid_key)
	{
		udpctl_packet_auth_t* packet = (udpctl_packet_auth_t *)buffer_out;
		length_out = sizeof(udpctl_packet_auth_t);
		packet->base_sec.base.service_id = htobe16(UDPCTL_SERVICE_ID);
		packet->base_sec.base.code = UCTL_CMD_CODE_AUTH;
		packet->base_sec.base.flags = PACKET_FLAG_REQUEST | PACKET_FLAG_SECURED;
		packet->base_sec.base.identifier = htobe16(1);
		packet->base_sec.base.length = htobe16(length_out);

		unsigned char	key[64];
		size_t keylen;
		if (valid_key) {
			keylen = os_strlen(SECRET_KEY1);
			os_memcpy(key, SECRET_KEY1, keylen);
		}
		else {
			keylen = os_strlen(SECRET_KEY2);
			os_memcpy(key, SECRET_KEY2, keylen);
		}

		os_memset(packet->base_sec.digest, 0, sizeof(udpctl_digest_t));
		packet->base_sec.digest[31] = 1;
		os_memset(packet->base_sec.digest, 0, sizeof(udpctl_digest_t));

		udpctl_digest_t digest_out;
		hmac(SHA256, (const unsigned char *)buffer_out, length_out,
			key, keylen,
			(uint8_t *)&digest_out);
		os_memcpy(packet->base_sec.digest, digest_out, sizeof(udpctl_digest_t));
	}

	void EncodeControl(bool auth_mistmatch, bool no_msg, bool no_msgtype)
	{
		udpctl_packet_sec_t* packet = (udpctl_packet_sec_t *)buffer_out;
		length_out = sizeof(udpctl_packet_sec_t);
		packet->base.service_id = htobe16(UDPCTL_SERVICE_ID);
		packet->base.code = UCTL_CMD_CODE_SRVMSG;
		packet->base.flags = PACKET_FLAG_REQUEST | PACKET_FLAG_SECURED;
		packet->base.identifier = htobe16(2);

		unsigned char	key[64];
		size_t keylen = os_strlen(SECRET_KEY1);
		os_memcpy(key, SECRET_KEY1, keylen);

		udpctl_packet_sec_t* packet_in = (udpctl_packet_sec_t *)buffer_in;

		os_memset(packet->digest, 0, sizeof(udpctl_digest_t));
		packet->digest[31] = 2;
		if (auth_mistmatch)
			os_memset(packet->digest, 0, sizeof(udpctl_digest_t));
		else
			os_memcpy(packet->digest, packet_in->digest, sizeof(udpctl_digest_t));

		dtlv_ctx_t dtlv_ctx;
		dtlv_ctx_init_encode(&dtlv_ctx, d_pointer_add(char, buffer_out, length_out), 512);
		if (!no_msg) {
			dtlv_avp_t* gavp1;
			dtlv_avp_encode_grouping(&dtlv_ctx, UDPCTL_SERVICE_ID, COMMON_AVP_SVC_MESSAGE, &gavp1);
			if (!no_msgtype) {
				dtlv_avp_encode_uint16(&dtlv_ctx, COMMON_AVP_SVC_MESSAGE_TYPE, SVCS_MSGTYPE_INFO);
			}
			dtlv_avp_encode_group_done(&dtlv_ctx, gavp1);
		}

		length_out += dtlv_ctx.datalen;
		packet->base.length = htobe16(length_out);

		udpctl_digest_t digest_out;
		hmac(SHA256, (const unsigned char *)buffer_out, length_out,
			key, keylen,
			(uint8_t *)&digest_out);
		os_memcpy(packet->digest, digest_out, sizeof(udpctl_digest_t));
	}


	void DecodeAuth(bool error, bool notsec, char* auth_resp)
	{
		udpctl_packet_sec_t* packet = (udpctl_packet_sec_t *)buffer_in;
		ASSERT_EQ(packet->base.service_id, htobe16(UDPCTL_SERVICE_ID));
		ASSERT_EQ(packet->base.code, UCTL_CMD_CODE_AUTH);
		ASSERT_EQ(packet->base.identifier, htobe16(1));
		ASSERT_EQ(packet->base.flags, (error ? PACKET_FLAG_ERROR : 0) | (notsec ? 0 : PACKET_FLAG_SECURED));

		char buf[512];
		dtlv_ctx_t dtlv_ctx;
		size_t hdrlen = (notsec) ? sizeof(udpctl_packet_t) : sizeof(udpctl_packet_auth_t);
		ASSERT_EQ(dtlv_ctx_init_decode(&dtlv_ctx, d_pointer_add(char, buffer_in, hdrlen), length_in - hdrlen), DTLV_ERR_SUCCESS);
		ASSERT_EQ(dtlv_decode_to_json(&dtlv_ctx, buf), DTLV_ERR_SUCCESS);
		if (auth_resp) {
		    ASSERT_STREQ(auth_resp, buf);
		}
	}

	void DecodeControl(bool error, char* ctl_resp)
	{
		udpctl_packet_sec_t* packet = (udpctl_packet_sec_t *)buffer_in;
		ASSERT_EQ(packet->base.service_id, htobe16(UDPCTL_SERVICE_ID));
		ASSERT_EQ(packet->base.code, UCTL_CMD_CODE_SRVMSG);
		ASSERT_EQ(packet->base.identifier, htobe16(2));
		ASSERT_EQ(packet->base.flags, (error ? PACKET_FLAG_ERROR : 0) | PACKET_FLAG_SECURED);

		char buf[512];
		dtlv_ctx_t dtlv_ctx;
		size_t hdrlen = sizeof(udpctl_packet_sec_t);
		ASSERT_EQ(dtlv_ctx_init_decode(&dtlv_ctx, d_pointer_add(char, buffer_in, hdrlen), length_in - hdrlen), DTLV_ERR_SUCCESS);
		ASSERT_EQ(dtlv_decode_to_json(&dtlv_ctx, buf), DTLV_ERR_SUCCESS);
		ASSERT_STREQ(ctl_resp, buf);
	}

	char			conf[256];
        dtlv_ctx_t		dtlv_conf;
	char*			buffer_in;
	packet_size_t		length_in;
	char*			buffer_out;
	packet_size_t		length_out;
	size_t			buflen;
	imdb_hndlr_t	hmdb;
	imdb_hndlr_t	hdata;
	ipv4_addr_t		addr;
	ip_port_t		port;
};


TEST_F(UdpctlClass, AuthRequestNotSec)
{
        svcs_resource_t svcres = { hmdb, 0, hdata };
	ASSERT_EQ(udpctl_on_start ((const svcs_resource_t *) &svcres, NULL), SVCS_ERR_SUCCESS);

	EncodeAuth(true);
	length_in = buflen;
	ASSERT_EQ(udpctl_sync_request(&addr, &port, buffer_out, length_out, buffer_in, &length_in), UDPCTL_SERVER_NOTSECURED);
	char auth_resp[] = "{\"9\":0,\"100\":256,\"3\":4,\"4\":\"requires not secured messages\",\"8\":3}";
	DecodeAuth(true, true, auth_resp);

	ASSERT_EQ(udpctl_on_stop(), SVCS_ERR_SUCCESS);
}

TEST_F(UdpctlClass, AuthRequestSuccess)
{
        svcs_resource_t svcres = { hmdb, 0, hdata };
	ASSERT_EQ(udpctl_on_start ((const svcs_resource_t *) &svcres, &dtlv_conf), SVCS_ERR_SUCCESS);

	EncodeAuth(true);
	length_in = buflen;
	ASSERT_EQ(udpctl_sync_request(&addr, &port, buffer_out, length_out, buffer_in, &length_in), UDPCTL_ERR_SUCCESS);
	char auth_resp[] = "{\"9\":0,\"100\":256,\"3\":1,\"102\":60}";
	DecodeAuth(false, false, auth_resp);

	ASSERT_EQ(udpctl_on_stop(), SVCS_ERR_SUCCESS);
}


TEST_F(UdpctlClass, ControlRequestMessageAbsent)
{
        svcs_resource_t svcres = { hmdb, 0, hdata };
	ASSERT_EQ(udpctl_on_start ((const svcs_resource_t *) &svcres, &dtlv_conf), SVCS_ERR_SUCCESS);

	EncodeAuth(true);
	length_in = buflen;
	ASSERT_EQ(udpctl_sync_request(&addr, &port, buffer_out, length_out, buffer_in, &length_in), UDPCTL_ERR_SUCCESS);

	char auth_resp[] = "{\"9\":0,\"100\":256,\"3\":1,\"102\":60}";
	DecodeAuth(false, false, auth_resp);

	EncodeControl(false, true, false);
	length_in = buflen;
	ASSERT_EQ(udpctl_sync_request(&addr, &port, buffer_out, length_out, buffer_in, &length_in), UDPCTL_INVALID_COMMAND);

	char ctl_resp[] = "{\"9\":0,\"9\":0,\"100\":256,\"3\":2,\"4\":\"invalid protocol command: AVP Message is absent\",\"8\":9}";
	DecodeControl(true, ctl_resp);

	ASSERT_EQ(udpctl_on_stop(), SVCS_ERR_SUCCESS);
}

TEST_F(UdpctlClass, ControlRequestMessageTypeAbsent)
{
        svcs_resource_t svcres = { hmdb, 0, hdata };
	ASSERT_EQ(udpctl_on_start ((const svcs_resource_t *) &svcres, &dtlv_conf), SVCS_ERR_SUCCESS);

	EncodeAuth(true);
	length_in = buflen;
	ASSERT_EQ(udpctl_sync_request(&addr, &port, buffer_out, length_out, buffer_in, &length_in), UDPCTL_ERR_SUCCESS);

	char auth_resp[] = "{\"9\":0,\"100\":256,\"3\":1,\"102\":60}";
	DecodeAuth(false, false, auth_resp);

	EncodeControl(false, false, true);
	length_in = buflen;
	ASSERT_EQ(udpctl_sync_request(&addr, &port, buffer_out, length_out, buffer_in, &length_in), UDPCTL_INVALID_COMMAND);

	char ctl_resp[] = "{\"9\":0,\"9\":0,\"100\":256,\"3\":2,\"4\":\"invalid protocol command: AVP Message-Type must be first\",\"8\":9}";
	DecodeControl(true, ctl_resp);

	ASSERT_EQ(udpctl_on_stop(), SVCS_ERR_SUCCESS);
}

TEST_F(UdpctlClass, ControlRequestSuccess)
{
        svcs_resource_t svcres = { hmdb, 0, hdata };
	ASSERT_EQ(udpctl_on_start ((const svcs_resource_t *) &svcres, &dtlv_conf), SVCS_ERR_SUCCESS);

	EncodeAuth(true);
	length_in = buflen;
	ASSERT_EQ(udpctl_sync_request(&addr, &port, buffer_out, length_out, buffer_in, &length_in), UDPCTL_ERR_SUCCESS);

	char auth_resp[] = "{\"9\":0,\"100\":256,\"3\":1,\"102\":60}";
	DecodeAuth(false, false, auth_resp);

	EncodeControl(false, false, false);
	length_in = buflen;
	ASSERT_EQ(udpctl_sync_request(&addr, &port, buffer_out, length_out, buffer_in, &length_in), UDPCTL_ERR_SUCCESS);

	char ctl_resp[] = "{\"9\":0,\"4.10\":{},\"3\":3,\"8\":3}";
	DecodeControl(false, ctl_resp);

	ASSERT_EQ(udpctl_on_stop(), SVCS_ERR_SUCCESS);
}

TEST_F(UdpctlClass, AuthRequestSecretMistmatch)
{
        svcs_resource_t svcres = { hmdb, 0, hdata };
	ASSERT_EQ(udpctl_on_start ((const svcs_resource_t *) &svcres, NULL), SVCS_ERR_SUCCESS);
	ASSERT_EQ(udpctl_on_cfgupd(&dtlv_conf), SVCS_ERR_SUCCESS);

	EncodeAuth(false);
	length_in = buflen;
	ASSERT_EQ(udpctl_sync_request(&addr, &port, buffer_out, length_out, buffer_in, &length_in), UDPCTL_INVALID_DIGEST);

	char auth_resp[] = "{\"9\":0,\"100\":256,\"3\":4,\"4\":\"invalid message digest\",\"8\":7}";
	DecodeAuth(true, false, auth_resp);

	ASSERT_EQ(udpctl_on_stop(), SVCS_ERR_SUCCESS);
}

TEST_F(UdpctlClass, ControlRequestDigestMistmatch)
{
        svcs_resource_t svcres = { hmdb, 0, hdata };
	ASSERT_EQ(udpctl_on_start ((const svcs_resource_t *) &svcres, &dtlv_conf), SVCS_ERR_SUCCESS);

	EncodeAuth(true);
	length_in = buflen;
	ASSERT_EQ(udpctl_sync_request(&addr, &port, buffer_out, length_out, buffer_in, &length_in), UDPCTL_ERR_SUCCESS);

	char auth_resp[] = "{\"9\":0,\"100\":256,\"3\":1,\"102\":60}";
	DecodeAuth(false, false, auth_resp);

	EncodeControl(true, false, false);
	length_in = buflen;
	ASSERT_EQ(udpctl_sync_request(&addr, &port, buffer_out, length_out, buffer_in, &length_in), UDPCTL_INVALID_DIGEST);

	char ctl_resp[] = "{\"9\":0,\"100\":256,\"3\":4,\"4\":\"invalid message digest\",\"8\":7}";
	DecodeControl(true, ctl_resp);

	ASSERT_EQ(udpctl_on_stop(), SVCS_ERR_SUCCESS);
}
