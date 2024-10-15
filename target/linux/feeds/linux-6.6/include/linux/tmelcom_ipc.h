/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */
#ifndef _TMELCOM_IPC_H_
#define _TMELCOM_IPC_H_

#define TMEL_MAX_FUSE_ADDR_SIZE 8
#define SECBOOT_SW_ID_ROOTPD 0xD

struct tmel_msg_param_type_buf_in {
	u32 buf;
	u32 buf_len;
};

struct tmel_msg_param_type_buf_out {
	u32 buf;
	u32 buf_len;
	u32 out_buf_len;
};

struct tmel_msg_param_type_buf_in_out {
	u32 buf;
	u32 buf_len;
	u32 out_buf_len;
};

struct tmel_fuse_payload {
	u32 fuse_addr;
	u32 lsb_val;
	u32 msb_val;
} __packed;

struct tmel_fuse_read_multiple_msg {
	u32 status;
	struct tmel_msg_param_type_buf_in_out fuse_read_data;
} __packed;

struct tmel_qwes_init_att_msg {
	u32 status;
	struct tmel_msg_param_type_buf_out rsp;
} __packed;

struct tmel_qwes_device_att_msg {
	u32 status;
	struct tmel_msg_param_type_buf_in req;
	struct tmel_msg_param_type_buf_in ext_claim;
	struct tmel_msg_param_type_buf_out rsp;
} __packed;

struct tmel_qwes_device_prov_msg {
	u32 status;
	struct tmel_msg_param_type_buf_in req;
	struct tmel_msg_param_type_buf_out rsp;
} __packed;

struct tmel_secboot_sec_auth_req {
	u32 sw_id;
	struct tmel_msg_param_type_buf_in elf_buf;
	struct tmel_msg_param_type_buf_in region_list;
	u32 relocate;
} __packed;

struct tmel_secboot_sec_auth_resp {
	u32 first_seg_addr;
	u32 first_seg_len;
	u32 entry_addr;
	u32 extended_error;
	u32 status;
} __packed;

struct tmel_secboot_sec_auth {
	struct tmel_secboot_sec_auth_req req;
	struct tmel_secboot_sec_auth_resp resp;
} __packed;

struct tmel_secboot_teardown_req {
	u32 sw_id;
	u32 secondary_sw_id;
} __packed;

struct tmel_secboot_teardown_resp {
	u32 status;
} __packed;

struct tmel_secboot_teardown {
	struct tmel_secboot_teardown_req req;
	struct tmel_secboot_teardown_resp resp;
} __packed;

struct tmel_licensing_check_msg {
	u32 status;
	struct tmel_msg_param_type_buf_in request;
	struct tmel_msg_param_type_buf_out response;
} __packed;

struct tmel_ttime_get_req_params {
	u32 status;
	struct tmel_msg_param_type_buf_out params;
} __packed;

struct tmel_ttime_set {
	u32 status;
	struct tmel_msg_param_type_buf_in ttime;
} __packed;

struct tmel_licensing_install {
	u32 status;
	struct tmel_msg_param_type_buf_in license;
	u32 flags;
	struct tmel_msg_param_type_buf_out identifier;
} __packed;

struct tmel_licensing_ToBeDel_licenses {
	u32 status;
	struct tmel_msg_param_type_buf_out toBeDelLicenses;
} __packed;

struct tmel_log_config {
	u8 component_id;
	u8 log_level;
};

struct tmel_log_get_message {
	u32 status;
	struct tmel_msg_param_type_buf_out log_buf;
} __packed;

struct tmel_log_set_config_message {
	u32 status;
	struct tmel_msg_param_type_buf_in log;
} __packed;

struct tmel_secure_io {
	u32 reg_addr;
	u32 reg_val;
} __packed;

struct tmel_secure_io_read {
	u32 status;
	struct tmel_msg_param_type_buf_in_out read_buf;
} __packed;

struct tmel_secure_io_write {
	u32 status;
	struct tmel_msg_param_type_buf_in write_buf;
} __packed;

struct tmel_get_arb_version_req {
	u32 sw_id;
} __packed;

struct tmel_get_arb_version_rsp {
	u8 oem_version;
	u8 qti_version;
	u8 oem_is_valid;
	u8 qti_is_valid;
	u32 status;
} __packed;

struct tmel_get_arb_version {
	struct tmel_get_arb_version_req req;
	struct tmel_get_arb_version_rsp rsp;
} __packed;

struct tmel_update_arb_version_rsp {
	u32 status;
} __packed;

struct tmel_update_arb_version_req {
	struct tmel_update_arb_version_rsp rsp;
} __packed;

struct tmel_response_cbuffer {
	u32 data;
	u32 len;
	u32 len_used;
} __packed;

struct tmel_km_ecdh_ipkey_req {
	u32 feature_id;
	u32 key_id;
} __packed;

struct tmel_seq_status_rsp {
	u32 tmel_err_status;
	u32 seq_err_status;
	u32 seq_kp_err_status0;
	u32 seq_kp_err_status1;
	u32 seq_rsp_status;
} __packed;

struct tmel_km_ecdh_ipkey_rsp {
	struct tmel_response_cbuffer rsp_buf;
	u32 status;
	struct tmel_seq_status_rsp seq_status;
} __packed;

struct tmel_km_ecdh_ipkey_msg {
	struct tmel_km_ecdh_ipkey_req req;
	struct tmel_km_ecdh_ipkey_rsp rsp;
} __packed;

#ifdef CONFIG_QCOM_TMELCOM
int tmelcom_probed(void);
int tmelcom_init_attestation(u32 *key_buf, u32 key_buf_len, u32 *key_buf_size);
int tmelcom_qwes_getattestation_report(u32 *req_buf, u32 req_buf_len,
				       u32 *extclaim_buf, u32 extclaim_buf_len,
				       u32 *resp_buf, u32 resp_buf_len,
				       u32 *resp_buf_size);
int tmelcom_qwes_device_provision(u32 *req_buf, u32 req_buf_len, u32 *resp_buf,
				  u32 resp_buf_len, u32 *resp_buf_size);
int tmelcom_fuse_list_read(struct tmel_fuse_payload *fuse, size_t size);
int tmelcom_secboot_sec_auth(u32 sw_id, void *metadata, size_t size);
int tmelcom_secboot_teardown(u32 sw_id, u32 secondary_sw_id);
int tmelcom_licensing_check(void *cbor_req, u32 req_len, void *cbor_resp,
			    u32 resp_len, u32 *used_resp_len);
int tmelcom_ttime_get_req_params(void *params_buf, u32 buf_len, u32 *used_buf_len);
int tmelcom_ttime_set(void *ttime_buf, u32 buf_len);
int tmelcom_licensing_install(void *license_buf, u32 license_len, void *ident_buf,
			      u32 ident_len, u32 *ident_used_len, u32 *flags);
int tmelcom_licensing_get_toBeDel_licenses(void *toBeDelLic_buf, u32 toBeDelLic_len,
					   u32 *used_toBeDelLic_len);
int tmelcom_set_tmel_log_config(void *buf, u32 size);
int tmelcom_get_tmel_log(void *buf, u32 max_buf_size, u32 *size);
int tmelcom_secure_io_read(struct tmel_secure_io *buf, size_t size);
int tmelcom_secure_io_write(struct tmel_secure_io *buf, size_t size);
int tmelcomm_secboot_get_arb_version(u32 type, u32 *version);
int tmelcomm_secboot_update_arb_version(u32 status);
int tmelcomm_get_ecc_public_key(u32 type, void *buf, u32 size, u32 *rsp_len);

#else
static inline int tmelcom_probed(void)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_fuse_list_read(struct tmel_fuse_payload *fuse,
					 size_t size)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_secboot_sec_auth(u32 sw_id, void *metadata,
					   size_t size)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_secboot_teardown(u32 sw_id, u32 secondary_sw_id)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_init_attestation(u32 *key_buf, u32 key_buf_len,
					   u32 *key_buf_size)
{
	return -EOPNOTSUPP;
}
static inline int tmelcom_qwes_getattestation_report(u32 *req_buf,
						     u32 req_buf_len,
						     u32 *extclaim_buf,
						     u32 extclaim_buf_len,
						     u32 *resp_buf,
						     u32 resp_buf_len,
						     u32 *resp_buf_size)
{
	return -EOPNOTSUPP;
}
static inline int tmelcom_qwes_device_provision(u32 *req_buf, u32 req_buf_len,
						u32 *resp_buf, u32 resp_buf_len,
						u32 *resp_buf_size)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_licensing_check(void *cbor_req, u32 req_len,
					  void *cbor_resp, u32 resp_len,
					  u32 *used_resp_len)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_ttime_get_req_params(void *params_buf, u32 buf_len,
					       u32 *used_buf_len)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_ttime_set(void *ttime_buf, u32 buf_len)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_licensing_install(void *license_buf, u32 license_len,
					    void *ident_buf, u32 ident_len,
					    u32 *ident_used_len, u32 *flags)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_licensing_get_toBeDel_licenses(void *toBeDelLic_buf,
							 u32 toBeDelLic_len,
							 u32 *used_toBeDelLic_len)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_set_tmel_log_config(void *buf, u32 size)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_get_tmel_log(void *buf, u32 max_buf_size,  u32 *size)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_secure_io_read(struct tmel_secure_io *buf, size_t size)
{
	return -EOPNOTSUPP;
}

static inline int tmelcom_secure_io_write(struct tmel_secure_io *buf, size_t size)
{
	return -EOPNOTSUPP;
}

static inline int tmelcomm_secboot_get_arb_version(u32 type, u32 *version)
{
	return -EOPNOTSUPP;
}

static inline int tmelcomm_secboot_update_arb_version(u32 status)
{
	return -EOPNOTSUPP;
}

static inline int tmelcomm_get_public_key(u32 type, void *buf, u32 *rsp_len)
{
	return -EOPNOTSUPP;
}
#endif /* CONFIG_QCOM_TMELCOM */
#endif /* _TMELCOM_IPC_H_ */
