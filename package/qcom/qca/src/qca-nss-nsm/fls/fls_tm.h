/*
 **************************************************************************
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#include <net/netfilter/nf_conntrack.h>
#include <linux/jiffies.h>

#define FLS_TM_STATS_PUSH_PERIOD msecs_to_jiffies(1000)
#define FLS_TM_FLAG_BREAK 0x01
#define FLS_TM_FLAG_RESET 0x02

struct fls_tm_flow {
	uint32_t src_ip_addr[4];		/* Source Ip Address */
	uint32_t dst_ip_addr[4];		/* Destination Ip Address */
	uint16_t src_port;			/* Source Port */
	uint16_t dst_port;			/* Destination Port */
	uint8_t src_mac_addr[ETH_ALEN];		/* Source Neighbor Mac */
	uint8_t dst_mac_addr[ETH_ALEN];		/* Desination Neighbor Mac */
	uint8_t proto;				/* Protocol */
	uint8_t ip_version;			/* Ip Version */
	uint64_t org_bytes;			/* Original Direction Bytes */
	uint64_t ret_bytes;			/* Return Direction Bytes */
	uint64_t org_pkts;			/* Original Direction Packets */
	uint64_t ret_pkts;			/* Return Direction Packets */
	char src_if[IFNAMSIZ];			/* Buffer containing source interface name */
	char dst_if[IFNAMSIZ];			/* Buffer containing destination interface name */
	uint8_t flags;				/* Flags used for processing by TM APP */
};

bool fls_tm_init(void);
void fls_tm_deinit(void);
