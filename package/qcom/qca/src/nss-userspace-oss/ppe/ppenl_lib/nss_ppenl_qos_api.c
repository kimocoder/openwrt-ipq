/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

#include <nss_ppenl_base.h>
#include "nss_ppenl_sock.h"
#include <nss_ppenl_qos_api.h>
#include "nss_ppenl_qos.h"

static struct nss_ppenl_qos_ctx nss_qos_ctx;
static void nss_ppenl_qos_resp(void *user_ctx, struct nss_ppenl_qos_req *req, void *resp_ctx) __attribute__((unused));
/*
 * ppecfg_qos_resp()
 * 	ppecfg log based on response from netlink
 */
static void nss_ppenl_qos_resp(void *user_ctx, struct nss_ppenl_qos_req *qos_req, void *resp_ctx)
{
	int ret = 0;

	if (!qos_req) {
		return;
	}

	uint8_t cmd = nss_ppenl_cmn_get_cmd_type(&qos_req->cm);

	switch (cmd) {
		case NSS_PPE_QOS_GET_INT_PRI:
			ret = qos_req->config.ret;
			if (ret == PPECFG_QOS_RET_CLASS_NON_LEAF) {
				nss_ppenl_sock_log_error("PPE queues are attached to only leaf classes and "
						"the class_id entered is not a leaf class.\n"
						"Please enter a leaf class_id or leaf qdisc handle_id.\n");
				return;
			} else if (ret == PPECFG_QOS_RET_INVALID_CLASS) {
				nss_ppenl_sock_log_error("The input handle_id does not exist in the"
						"qdisc heirarchy configured on the given interface\n");
				return;
			} else if (ret == PPECFG_QOS_RET_INVALID_DEV) {
				nss_ppenl_sock_log_error("The physical interface name is invalid.\n"
						"Please enter a valid interface name.");
				return;
			} else if ((ret != PPECFG_QOS_RET_SUCCESS)) {
				nss_ppenl_sock_log_error("QoS req create failed with error: %d\n", ret);
				return;
			}

			printf(" Handle-Id: %x\n PPE-Queue#: %d\n INT_PRI: %d\n",qos_req->config.handle_id,
					qos_req->config.ucast_qid, qos_req->config.int_pri);
			break;

		default:
			nss_ppenl_sock_log_error("unsupported message cmd type(%d)", cmd);
	}
}

/*
 * nss_ppenl_qos_sock_cb()
 *	NSS NL QoS callback
 */
int nss_ppenl_qos_sock_cb(struct nl_msg *msg, void *arg)
{
	pid_t pid = getpid();

	struct nss_ppenl_qos_ctx *ctx = (struct nss_ppenl_qos_ctx *)arg;
	struct nss_ppenl_sock_ctx *sock = &ctx->sock;
	struct nss_ppenl_qos_req *req = nss_ppenl_sock_get_data(msg);

	if (!req) {
		nss_ppenl_sock_log_error("%d:failed to get NSS NL QoS header\n", pid);
		return NL_SKIP;
	}
	uint8_t cmd = nss_ppenl_cmn_get_cmd_type(&req->cm);

	switch (cmd) {
	case NSS_PPE_QOS_GET_INT_PRI:
	{
		void *cb_data = nss_ppenl_cmn_get_cb_data(&req->cm, sock->family_id);

		if (!cb_data) {
			return NL_SKIP;
		}

		/*
		 * Note: The callback user can modify the CB content so it
		 * needs to locally save the response data for further use
		 * after the callback is completed
		 */
		struct nss_ppenl_qos_resp resp;
		memcpy(&resp, cb_data, sizeof(struct nss_ppenl_qos_resp));

		/*
		 * clear the ownership of the CB so that callback user can
		 * use it if needed
		 */
		nss_ppenl_cmn_clr_cb_owner(&req->cm);

		if (!resp.cb) {
			nss_ppenl_sock_log_info("%d:no QoS response callback for cmd(%d)\n", pid, cmd);
			return NL_SKIP;
		}

		resp.cb(sock->user_ctx, req, resp.data);
		return NL_OK;
	}

	default:
		nss_ppenl_sock_log_error("%d:unsupported message cmd type(%d)\n", pid, cmd);
		return NL_SKIP;
	}
}

/*
 * nss_ppenl_qos_sock_open()
 *	this opens the NSS QoS NL socket for usage
 */
int nss_ppenl_qos_sock_open(struct nss_ppenl_qos_ctx *ctx, void *user_ctx)
{
	pid_t pid = getpid();
	int error;

	if (!ctx) {
		nss_ppenl_sock_log_error("%d: invalid parameters passed\n", pid);
		return -EINVAL;
	}

	memset(ctx, 0, sizeof(*ctx));
	nss_ppenl_sock_set_family(&ctx->sock, NSS_PPENL_QOS_FAMILY);
	nss_ppenl_sock_set_user_ctx(&ctx->sock, user_ctx);

	/*
	 * try opening the socket with Linux
	 */
	error = nss_ppenl_sock_open(&ctx->sock, nss_ppenl_qos_sock_cb);
	if (error) {
		nss_ppenl_sock_log_error("%d:unable to open NSS QoS socket, error(%d)\n", pid, error);
		goto fail;
	}

	return 0;
fail:
	memset(ctx, 0, sizeof(*ctx));
	return error;
}

/*
 * nss_ppenl_qos_sock_close()
 *	close the NSS QoS NL socket
 */
void nss_ppenl_qos_sock_close(struct nss_ppenl_qos_ctx *ctx)
{
	nss_ppenl_sock_close(&ctx->sock);
}

/*
 * nss_ppenl_qos_sock_send()
 *	register callback and send the QoS message synchronously through the socket
 */
int nss_ppenl_qos_sock_send(struct nss_ppenl_qos_ctx *ctx, struct nss_ppenl_qos_req *req, nss_ppenl_qos_resp_cb_t cb)
{
	int32_t family_id = ctx->sock.family_id;
	struct nss_ppenl_qos_resp *resp;
	pid_t pid = getpid();
	bool has_resp = false;
	int error;

	if (!req) {
		nss_ppenl_sock_log_error("%d:invalid NSS QoS req\n", pid);
		return -ENOMEM;
	}

	if (cb) {

		nss_ppenl_cmn_set_cb_owner(&req->cm, family_id);

		resp = nss_ppenl_cmn_get_cb_data(&req->cm, family_id);
		assert(resp);

		resp->data = NULL;
		resp->cb = cb;
		has_resp = true;
	}

	error = nss_ppenl_sock_send(&ctx->sock, &req->cm, req, has_resp);
	if (error) {
		nss_ppenl_sock_log_error("%d:failed to send NSS QoS req, error(%d)\n", pid, error);
		return error;
	}

	return 0;
}

/*
 * nss_ppenl_qos_get_int_pri()
 * Send QoS req to PPE driver
 */
int nss_ppenl_qos_get_int_pri(struct nss_ppenl_qos_req *req) {

	int error;

	/*
	 * open the NSS NL QoS socket
	 */
	error = nss_ppenl_qos_sock_open(&nss_qos_ctx, NULL);
	if (error < 0) {
		nss_ppenl_sock_log_error("Failed to open QoS socket; error(%d)\n", error);
		return error;
	}

	/*
	 * send message
	 */
	error = nss_ppenl_qos_sock_send(&nss_qos_ctx, req, nss_ppenl_qos_resp);
	if (error < 0) {
		nss_ppenl_sock_log_error("Unable to send message\n");
		goto done;
	}
done:
	/*
	 * close the socket
	 */
	nss_ppenl_qos_sock_close(&nss_qos_ctx);
	return error;
}

/*
 * nss_ppenl_qos_init_req()
 *	init the req message
 */
void nss_ppenl_qos_init_req(struct nss_ppenl_qos_req *req, enum nss_ppe_qos_message_types type)
{
	if (type >= NSS_PPE_QOS_MAX_MSG_TYPES) {
		nss_ppenl_sock_log_error("Incorrect req type\n");
		return;
	}

	nss_ppenl_qos_req_init(req, type);
}
