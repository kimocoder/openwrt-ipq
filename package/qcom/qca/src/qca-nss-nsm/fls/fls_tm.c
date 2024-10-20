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

#include <linux/types.h>
#include <linux/workqueue.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_acct.h>
#include <net/netfilter/nf_conntrack_zones.h>
#include <net/netfilter/nf_conntrack_timestamp.h>
#include <linux/list_nulls.h>
#include <linux/rculist_nulls.h>
#ifdef FLS_ECM_CLASSIFIER_EMESH_ENABLE
#include <ecm_classifier_emesh_public.h>
#endif
#include "fls_tm.h"
#include "fls_tm_chardev.h"
#include "fls_debug.h"

struct delayed_work fls_tm_work;
struct workqueue_struct *fls_tm_workqueue;
unsigned int bucket;

/*
 * fls_tm_get_neigh_ipv4
 *	Returns neighbor reference for a given IPV4 address
 */
struct neighbour *fls_tm_get_neigh_ipv4(uint32_t ip_addr)
{
	struct neighbour *neigh;
	struct rtable *rt;
	struct dst_entry *dst;

	/*
	 * search for route entry
	 */
	rt = ip_route_output(&init_net, ip_addr, 0, 0, 0);
	if (IS_ERR(rt)) {
		return NULL;
	}

	dst = (struct dst_entry *)rt;

	/*
	 * neighbour lookup using IP address in the neighor table
	 */
	neigh = dst_neigh_lookup(dst, &ip_addr);
	if (likely(neigh)) {
		dst_release(dst);
		return neigh;
	}

	/*
	 * neighbour lookup using IP address, device in the arp table
	 */
	neigh = neigh_lookup(&arp_tbl, &ip_addr, dst->dev);
	if (likely(neigh)) {
		dst_release(dst);
		return neigh;
	}

	/*
	 * dst reference count was held during the lookup
	 */
	dst_release(dst);
	return NULL;
}

/*
 * fls_tm_get_macaddr_ipv4()
 * 	Return the hardware (MAC) address of the given IPv4 address, if any.
 */
int fls_tm_get_macaddr_ipv4(uint32_t ip_addr, uint8_t *mac_addr)
{
	struct neighbour *neigh;

	/*
	 * handle multicast IP address seperately
	 */
	if (ipv4_is_multicast(ip_addr)) {
		FLS_INFO("no support for mulicast clients IP:0x%x\n", ip_addr);
		return -EINVAL;
	}

	/*
	 * retrieve the neighbour
	 */
	neigh = fls_tm_get_neigh_ipv4(ip_addr);
	if (!neigh) {
		FLS_INFO("neighbour lookup failed for IP:0x%x\n", ip_addr);
		return -ENODEV;
	}

	if ((neigh->nud_state & NUD_VALID) == 0) {
		FLS_INFO("neighbour state is invalid for IP:0x%x\n", ip_addr);
		goto fail;
	}

	if (!neigh->dev) {
		FLS_INFO("neighbour device not found for IP:0x%x\n", ip_addr);
		goto fail;
	}

	if (is_multicast_ether_addr(neigh->ha)) {
		FLS_INFO( "neighbour MAC address is multicast or broadcast\n");
		goto fail;
	}

	ether_addr_copy(mac_addr, neigh->ha);
	neigh_release(neigh);
	return 0;
fail:

	neigh_release(neigh);
	return -ENODEV;
}

/*
 * fls_tm_get_neigh_ipv6()
 *	Returns neighbor reference for a given IPV6 address
 */
static struct neighbour *fls_tm_get_neigh_ipv6(uint32_t ip_addr[4])
{
	struct neighbour *neigh;
	struct dst_entry *dst;
	struct rt6_info *rt;
	struct in6_addr daddr;

	daddr.in6_u.u6_addr32[0] = ip_addr[0];
	daddr.in6_u.u6_addr32[1] = ip_addr[1];
	daddr.in6_u.u6_addr32[2] = ip_addr[2];
	daddr.in6_u.u6_addr32[3] = ip_addr[3];
	rt = rt6_lookup(&init_net, &daddr, NULL, 0, NULL, 0);
	if (!rt) {
		return NULL;
	}

	dst = (struct dst_entry *)rt;

	/*
	 * neighbour lookup using IP address in the neighbor table
	 */
	neigh = dst_neigh_lookup(dst, ip_addr);
	if (likely(neigh)) {
		neigh_hold(neigh);
		dst_release(dst);

		return neigh;
	}
	dst_release(dst);

	return NULL;
}

/*
 * fls_tm_get_macaddr_ipv6
 * 	Return the hardware (MAC) address of the given ipv6 address, if any.
 */
static int fls_tm_get_macaddr_ipv6(uint32_t ip_addr[4], uint8_t mac_addr[])
{
	struct neighbour *neigh;
	struct in6_addr addr;

	/*
	 * Convert from host to network order
	 */
	addr.s6_addr32[0] = ip_addr[0];
	addr.s6_addr32[1] = ip_addr[1];
	addr.s6_addr32[2] = ip_addr[2];
	addr.s6_addr32[3] = ip_addr[3];
	if (ipv6_addr_is_multicast(&addr)) {
		FLS_INFO("no support for mulicast clients %pI6c\n", ip_addr);
		return -EINVAL;
	}

	/*
	 * retrieve the neighbour
	 */
	neigh = fls_tm_get_neigh_ipv6(ip_addr);
	if (!neigh) {
		FLS_INFO("neighbour lookup failed for %pI6c\n", ip_addr);
		return -ENODEV;
	}

	if ((neigh->nud_state & NUD_VALID) == 0) {
		FLS_INFO("neighbour state is invalid for %pI6c\n", ip_addr);
		goto fail;
	}

	if (!neigh->dev) {
		FLS_INFO("neighbour device not found for %pI6c\n", ip_addr);
		goto fail;
	}

	if (is_multicast_ether_addr(neigh->ha)) {
		FLS_INFO("neighbour MAC address is multicast or broadcast\n");
		goto fail;
	}

	ether_addr_copy(mac_addr, neigh->ha);
	neigh_release(neigh);
	return 0;
fail:

	neigh_release(neigh);
	return -ENODEV;
}

/*
 * fls_tm_print_tm_flow;
 *	Print characteristics of singular tm flow
 */

void fls_tm_print_tm_flow(struct fls_tm_flow *tm_flow)
{
	if (tm_flow->ip_version == 4) {
		FLS_TRACE("\nsrc=%pI4 dst=%pI4 ",
			&tm_flow->src_ip_addr[0], &tm_flow->dst_ip_addr[0]);
	} else {
		FLS_TRACE("\nsrc=%pI6 dst=%pI6 ",
			&tm_flow->src_ip_addr, &tm_flow->dst_ip_addr);
	}
	FLS_TRACE("sport=%hu dport=%hu src_mac_addr=%pM dst_mac_addr=%pM\n protocol=%u org_bytes=%u org_pkts=%u ret_bytes=%u ret_pkts=%u\n",
		tm_flow->src_port, tm_flow->dst_port,
		tm_flow->src_mac_addr, tm_flow->dst_mac_addr,
		tm_flow->proto,
		tm_flow->org_bytes, tm_flow->org_pkts,
		tm_flow->ret_bytes, tm_flow->ret_pkts);
}

/*
 * fls_tm_fill_tm_flow
 *	Fill one flow message to be sent to FTM via character device
 */
void fls_tm_fill_tm_flow(struct nf_conn *ct, struct nf_conntrack_tuple *tuple, const struct nf_conntrack_l4proto *l4proto, struct nf_conn_acct *ct_acct, struct fls_tm_flow *tm_flow)
{
	switch (tuple->src.l3num) {
	case NFPROTO_IPV4:
		tm_flow->src_ip_addr[0] = tuple->src.u3.ip;
		tm_flow->dst_ip_addr[0] = tuple->dst.u3.ip;
		/*
		 * If we fail to get mac fill with 0 to avoid unexpected behavior
		 */
		if (fls_tm_get_macaddr_ipv4(tm_flow->src_ip_addr[0], tm_flow->src_mac_addr)) {
			memset(tm_flow->src_mac_addr, 0, sizeof(tm_flow->src_mac_addr));
		}
		if (fls_tm_get_macaddr_ipv4(tm_flow->dst_ip_addr[0], tm_flow->dst_mac_addr)) {
			memset(tm_flow->dst_mac_addr, 0, sizeof(tm_flow->dst_mac_addr));
		}
		tm_flow->ip_version = 4;
		memset(tm_flow->src_if, 0, IFNAMSIZ);
		memset(tm_flow->dst_if, 0, IFNAMSIZ);
#ifdef FLS_ECM_CLASSIFIER_EMESH_ENABLE
		ecm_classifier_emesh_sawf_get_iface_names_ipv4(ct, tm_flow->src_if, tm_flow->dst_if);
#endif
		break;
	case NFPROTO_IPV6:
		memcpy(tm_flow->src_ip_addr, &tuple->src.u3.ip6, sizeof(uint32_t) * 4);
		memcpy(tm_flow->dst_ip_addr, &tuple->dst.u3.ip6, sizeof(uint32_t) * 4);
		/*
		 * If we fail to get mac fill with 0 to avoid unexpected behavior
		 */
		if (fls_tm_get_macaddr_ipv6(tm_flow->src_ip_addr, tm_flow->src_mac_addr)) {
			memset(tm_flow->src_mac_addr, 0, sizeof(tm_flow->src_mac_addr));
		}
		if (fls_tm_get_macaddr_ipv6(tm_flow->dst_ip_addr, tm_flow->dst_mac_addr)) {
			memset(tm_flow->dst_mac_addr, 0, sizeof(tm_flow->dst_mac_addr));
		}
		tm_flow->ip_version = 6;
		memset(tm_flow->src_if, 0, IFNAMSIZ);
		memset(tm_flow->dst_if, 0, IFNAMSIZ);
#ifdef FLS_ECM_CLASSIFIER_EMESH_ENABLE
		ecm_classifier_emesh_sawf_get_iface_names_ipv6(ct, tm_flow->src_if, tm_flow->dst_if);
#endif
		break;
	default:
		break;
	}

	switch (l4proto->l4proto) {
	case IPPROTO_TCP:
		tm_flow->src_port = tuple->src.u.tcp.port;
		tm_flow->dst_port = tuple->dst.u.tcp.port;
		break;
	case IPPROTO_UDP:
		tm_flow->src_port = tuple->src.u.udp.port;
		tm_flow->dst_port = tuple->dst.u.udp.port;
		break;
	default:
		break;
	}

	tm_flow->org_bytes = atomic64_read(&ct_acct->counter[IP_CT_DIR_ORIGINAL].bytes);
	tm_flow->org_pkts = atomic64_read(&ct_acct->counter[IP_CT_DIR_ORIGINAL].packets);
	tm_flow->ret_bytes = atomic64_read(&ct_acct->counter[IP_CT_DIR_REPLY].bytes);
	tm_flow->ret_pkts = atomic64_read(&ct_acct->counter[IP_CT_DIR_REPLY].packets);
	tm_flow->proto = l4proto->l4proto;
	tm_flow->flags = 0;
	fls_tm_print_tm_flow(tm_flow);
	return;

}

/*
 * fls_tm_push_stats_req_work
 *	Worker Function to Push Stats to Userspace Periodically
 */
void fls_tm_push_stats_req_work(struct work_struct *work)
{
	int i;
	struct nf_conn *ct;
	struct nf_conn_acct *ct_acct;
	struct nf_conntrack_tuple_hash *h;
	struct hlist_nulls_node *n;
	struct nf_conntrack_tuple *tuple;
	const struct nf_conntrack_l4proto *l4proto;
	struct fls_tm_flow tm_flow;
	spinlock_t *lockp;

	FLS_TRACE("FLS_TM Workqueue Called\n");

	/*
	 * Iterate through nf_conntrack entries
	 */
	for (i = 0; i < nf_conntrack_htable_size; i++) {
		lockp = &nf_conntrack_locks[i % CONNTRACK_LOCKS];
		spin_lock_bh(lockp);

		hlist_nulls_for_each_entry(h, n, &nf_conntrack_hash[i], hnnode) {
			ct = nf_ct_tuplehash_to_ctrack(h);
			if (unlikely(!ct)) {
				FLS_WARN("Unable to get nf_conn\n");
				continue;
			}

			if (nf_ct_should_gc(ct)) {
				continue;
			}

			if (NF_CT_DIRECTION(h)) {
				continue;
			}

			tuple = &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
			l4proto = nf_ct_l4proto_find(nf_ct_protonum(ct));

			if (tuple->src.l3num != NFPROTO_IPV4 && tuple->src.l3num != NFPROTO_IPV6) {
				FLS_TRACE("Connection: %p has invalid IP address", ct);
				continue;
			}
			if (l4proto->l4proto != IPPROTO_TCP && l4proto->l4proto != IPPROTO_UDP) {
				FLS_TRACE("Connection: %p isn't TCP or UDP", ct);
				continue;
			}

			ct_acct = nf_conn_acct_find(ct);
			if (unlikely(!ct_acct)) {
				FLS_WARN("Unable to get stats for connection:%p \n", ct);
				continue;
			}

			fls_tm_fill_tm_flow(ct, tuple, l4proto, ct_acct, &tm_flow);

			if (!fls_tm_chardev_enqueue(&tm_flow)) {
				FLS_INFO("FTM_MSG Dropped");
			}
		}
		spin_unlock_bh(lockp);
	}

	/*
	 * Breakpoint between seconds
	 */
	memset(&tm_flow, 0, sizeof(tm_flow));
	tm_flow.flags |= FLS_TM_FLAG_BREAK;
	if (!fls_tm_chardev_enqueue(&tm_flow)) {
		FLS_INFO("FTM_MSG Dropped");
	}

	queue_delayed_work(fls_tm_workqueue, &fls_tm_work, FLS_TM_STATS_PUSH_PERIOD);
}

/*
 * fls_tm_init();
 */
bool fls_tm_init(void)
{
	if (fls_tm_chardev_init()) {
		return false;
	}
	/*
	 * Create Workqueues
	 */
	fls_tm_workqueue = create_singlethread_workqueue("fls_tm_workqueue");
	if(!fls_tm_workqueue) {
		FLS_WARN("Failed to initialize FLS TM workqueue\n");
		return false;
	}
	INIT_DELAYED_WORK(&fls_tm_work, fls_tm_push_stats_req_work);
	queue_delayed_work(fls_tm_workqueue, &fls_tm_work, FLS_TM_STATS_PUSH_PERIOD);
	FLS_TRACE("FLS TM Init Success\n");

	return true;
}

/*
 * fls_tm_deinit()
 */
void fls_tm_deinit(void)
{
	fls_tm_chardev_shutdown();
	/*
	 * Cancel the push stats req work and destroy workqueues
	 */
	cancel_delayed_work_sync(&fls_tm_work);
	destroy_workqueue(fls_tm_workqueue);
}
