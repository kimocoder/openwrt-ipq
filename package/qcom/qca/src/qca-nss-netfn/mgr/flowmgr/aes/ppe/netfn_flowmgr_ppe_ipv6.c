/*
 **************************************************************************
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 **************************************************************************
 */

/*
 * netfn_flowmgr_ppe_ipv6.c
 *	Netfn flow manager ppe ipv6 file
 */
#include <netfn_flowmgr.h>
#include <flowmgr/netfn_flowmgr_priv.h>
#include <flowmgr/netfn_flowmgr_stats.h>
#include "netfn_flowmgr_ppe.h"
#include "netfn_flowmgr_ppe_ipv4.h"
#include "netfn_flowmgr_ppe_ipv6.h"
#include "netfn_flowmgr_ppe_stats.h"

bool ipv6_stats_sync = false;

/*
 * netfn_flowmgr_ppe_validate_v6_tuple_info()
 *	Validate tuple info in original and reply direction
 */
static netfn_flowmgr_ret_t netfn_flowmgr_ppe_validate_v6_tuple_info(struct netfn_tuple original_tuple, struct netfn_tuple reply_tuple)
{
	struct netfn_flowmgr *f = &netfn_flowmgr_gbl;
	uint32_t org_src_ip[4];
	uint32_t org_dst_ip[4];
	uint32_t reply_src_ip[4];
	uint32_t reply_dst_ip[4];
	uint16_t org_src_ident;
	uint16_t org_dst_ident;
	uint16_t reply_src_ident;
	uint16_t reply_dst_ident;
	struct netfn_flowmgr_debug_stats *stats;
	netfn_tuple_type_t tuple_type;

	stats = &f->stats[NETFN_FLOWMGR_AE_TYPE_PPE];
	netfn_flowmgr_trace("org tuple type = %d\n", original_tuple.tuple_type);
	netfn_flowmgr_trace("reply tuple type = %d\n", reply_tuple.tuple_type);

	/*
	 * Check tuple type of original and reply direction
	 */
	if (original_tuple.tuple_type != reply_tuple.tuple_type){
		netfn_flowmgr_warn("Tuple type is not matching for original and reply direction\n");
		netfn_flowmgr_stats_inc(&stats->v6_validate_tuple_mismatch);
		return NETFN_FLOWMGR_RET_PPE_TUPLE_MISMATCH;
	}

	tuple_type = original_tuple.tuple_type;

	/*
	 * Based on tuple type validate the tuple info
	 */
	if (tuple_type == NETFN_TUPLE_3TUPLE) {
		netfn_flowmgr_stats_inc(&stats->v6_validate_tuple_type_3);
		/*
		 * IPs
		 */
		memcpy(org_src_ip, original_tuple.tuples.tuple_3.src_ip.ip6.s6_addr32, sizeof(uint32_t) * 4);
		memcpy(org_dst_ip, original_tuple.tuples.tuple_3.dest_ip.ip6.s6_addr32, sizeof(uint32_t) * 4);
		memcpy(reply_src_ip, reply_tuple.tuples.tuple_3.src_ip.ip6.s6_addr32, sizeof(uint32_t) * 4);
		memcpy(reply_dst_ip, reply_tuple.tuples.tuple_3.dest_ip.ip6.s6_addr32, sizeof(uint32_t) * 4);

		/*
		 * Original flow rule ip info should match with reply direction
		 *
		 */
		if ((memcmp(org_src_ip, reply_dst_ip, sizeof(uint32_t) * 4)) ||
				(memcmp(org_dst_ip, reply_src_ip, sizeof(uint32_t) * 4))){
			netfn_flowmgr_warn("Non-matching IP addresses in flow and return direction\n");
			netfn_flowmgr_warn("org_src_ip = %pI6, org_dst_ip = %pI6\n", &org_src_ip, &org_dst_ip);
			netfn_flowmgr_warn("reply_src_ip = %pI6, reply_dst_ip = %pI6\n", &reply_src_ip, &reply_dst_ip);
			netfn_flowmgr_stats_inc(&stats->v6_validate_tuple_ip_addr_mismatch);
			return NETFN_FLOWMGR_RET_PPE_IP_ADDR_MISMATCH;
		}

		/*
		 * Original flow rule protocol should match with reply direction
		 */
		if (original_tuple.tuples.tuple_3.protocol != reply_tuple.tuples.tuple_3.protocol) {
			netfn_flowmgr_warn("Non-matching protocol in flow and return direction, org_proto = %u, reply_proto = %u\n",
							original_tuple.tuples.tuple_3.protocol, reply_tuple.tuples.tuple_3.protocol);
			netfn_flowmgr_stats_inc(&stats->v6_validate_tuple_proto_mismatch);
			return NETFN_FLOWMGR_RET_PPE_PROTOCOL_MISMATCH;
		}
	} else if (tuple_type == NETFN_TUPLE_5TUPLE) {
		netfn_flowmgr_stats_inc(&stats->v6_validate_tuple_type_5);
		/*
		 * IPs
		 */
		memcpy(org_src_ip, original_tuple.tuples.tuple_3.src_ip.ip6.s6_addr32, sizeof(uint32_t) * 4);
		memcpy(org_dst_ip, original_tuple.tuples.tuple_3.dest_ip.ip6.s6_addr32, sizeof(uint32_t) * 4);
		memcpy(reply_src_ip, reply_tuple.tuples.tuple_3.src_ip.ip6.s6_addr32, sizeof(uint32_t) * 4);
		memcpy(reply_dst_ip, reply_tuple.tuples.tuple_3.dest_ip.ip6.s6_addr32, sizeof(uint32_t) * 4);

		/*
		 * Ports
		 */
		org_src_ident = original_tuple.tuples.tuple_5.l4_src_ident;
		org_dst_ident = original_tuple.tuples.tuple_5.l4_dest_ident;
		reply_src_ident = reply_tuple.tuples.tuple_5.l4_src_ident;
		reply_dst_ident = reply_tuple.tuples.tuple_5.l4_dest_ident;

		/*
		 * Original flow rule ip info should match with reply direction
		 *
		 */
		if ((memcmp(org_src_ip, reply_dst_ip, sizeof(uint32_t) * 4)) ||
				(memcmp(org_dst_ip, reply_src_ip, sizeof(uint32_t) * 4))){
			netfn_flowmgr_warn("Non-matching IP addresses in flow and return direction\n");
			netfn_flowmgr_warn("org_src_ip = %pI6, org_dst_ip = %pI6\n", &org_src_ip, &org_dst_ip);
			netfn_flowmgr_warn("reply_src_ip = %pI6, reply_dst_ip = %pI6\n", &reply_src_ip, &reply_dst_ip);
			netfn_flowmgr_stats_inc(&stats->v6_validate_tuple_ip_addr_mismatch);
			return NETFN_FLOWMGR_RET_PPE_IP_ADDR_MISMATCH;
		}

		/*
		 * Original flow rule ports should match with reply direction
		 */
		if (org_src_ident != reply_dst_ident || org_dst_ident != reply_src_ident) {
			netfn_flowmgr_warn("Non-matching port numbers in flow and return direction\n");
			netfn_flowmgr_warn("org_src_ident = %u, org_dst_ident = %u\n", org_src_ident, org_dst_ident);
			netfn_flowmgr_warn("reply_src_ident = %u, reply_dst_ident = %u\n", reply_src_ident, reply_dst_ident);
			netfn_flowmgr_stats_inc(&stats->v6_validate_tuple_ident_mismatch);
			return  NETFN_FLOWMGR_RET_PPE_PORT_MISMATCH;
		}

		/*
		 * Original flow rule protocol should match with reply direction
		 */
		if (original_tuple.tuples.tuple_5.protocol != reply_tuple.tuples.tuple_5.protocol) {
			netfn_flowmgr_warn("Non-matching protocol in flow and return direction, org_proto = %u, reply_proto = %u\n",
						original_tuple.tuples.tuple_5.protocol, reply_tuple.tuples.tuple_5.protocol);
			netfn_flowmgr_stats_inc(&stats->v6_validate_tuple_proto_mismatch);
			return NETFN_FLOWMGR_RET_PPE_PROTOCOL_MISMATCH;
		}

	} else {
		netfn_flowmgr_warn("Unsupported tupple type in PPE acceleration mode, tuple_type = %d\n", tuple_type);
		netfn_flowmgr_stats_inc(&stats->v6_validate_tuple_type_invalid);
		return NETFN_FLOWMGR_RET_PPE_UNSUPPORTED_TUPLE_TYPE;
	}
	return NETFN_FLOWMGR_RET_SUCCESS;
}

/*
 * netfn_flowmgr_ppe_create_v6_rule()
 *	Create PPE v6 flow rule
 */
netfn_flowmgr_ret_t netfn_flowmgr_ppe_create_v6_rule(struct netfn_flowmgr_create_rule *original, struct netfn_flowmgr_create_rule *reply)
{
	struct netfn_flowmgr *f = &netfn_flowmgr_gbl;
	struct ppe_drv_v6_rule_create *pd6rc;
	struct net_device *in_dev;
	struct net_device *out_dev;
	struct net_device *top_indev;
	struct net_device *top_outdev;
	uint32_t org_flow_flags;
	uint32_t reply_flow_flags;
	uint32_t org_rules;
	uint32_t reply_rules;
	struct netfn_flowmgr_debug_stats *stats;
	netfn_tuple_type_t tuple_type;
	ppe_drv_ret_t ppe_status = PPE_DRV_RET_SUCCESS;
	netfn_flowmgr_ret_status_t netfn_status = NETFN_FLOWMGR_RET_SUCCESS;

	stats = &f->stats[NETFN_FLOWMGR_AE_TYPE_PPE];

	/*
         * Update stats
         */
	netfn_flowmgr_stats_inc(&stats->v6_create_ppe_rule_req);

	/*
         * Validate PPE v6 tuple information
         */
        netfn_status = netfn_flowmgr_ppe_validate_v6_tuple_info(original->tuple, reply->tuple);
        if (netfn_status != NETFN_FLOWMGR_RET_SUCCESS) {
                netfn_flowmgr_warn("Invalid v6 tuple info, PPE rule creation failed\n");
                return NETFN_FLOWMGR_SET_NETFN_STATUS_CODE(netfn_status, ppe_status);
        }

	/*
	 * Allocate memory for ppe ipv6 rule create structure
	 */
	pd6rc = (struct ppe_drv_v6_rule_create *)kzalloc(sizeof(struct ppe_drv_v6_rule_create), GFP_ATOMIC);
	if (!pd6rc) {
		netfn_flowmgr_warn("No memory to allocate ppe ipv6 rule create structure\n");
		netfn_flowmgr_stats_inc(&stats->v6_create_rule_req_fail_no_mem);
		return NETFN_FLOWMGR_SET_NETFN_STATUS_CODE(NETFN_FLOWMGR_RET_NO_MEM, ppe_status);
	}

	tuple_type = original->tuple.tuple_type;

	/*
	 * Based on tuple type fill the PPE tuple info
	 */
	if (tuple_type == NETFN_TUPLE_3TUPLE) {
		/*
		 * Fill PPE three tuple info
		 */
		pd6rc->tuple.protocol = original->tuple.tuples.tuple_3.protocol;
		memcpy(pd6rc->tuple.flow_ip, original->tuple.tuples.tuple_3.src_ip.ip6.s6_addr32, sizeof(uint32_t) * 4);
		netfn_flowmgr_ppe_endianess_be_to_le(pd6rc->tuple.flow_ip, NETFN_FLOWMGR_TUPLE_IP_VERSION_V6);
		memcpy(pd6rc->tuple.return_ip, original->tuple.tuples.tuple_3.dest_ip.ip6.s6_addr32, sizeof(uint32_t) * 4);
		netfn_flowmgr_ppe_endianess_be_to_le(pd6rc->tuple.return_ip, NETFN_FLOWMGR_TUPLE_IP_VERSION_V6);
	} else if (tuple_type == NETFN_TUPLE_5TUPLE) {
		/*
		 * Fill PPE five tuple info
		 */
		pd6rc->tuple.protocol = original->tuple.tuples.tuple_5.protocol;
		memcpy(pd6rc->tuple.flow_ip, original->tuple.tuples.tuple_3.src_ip.ip6.s6_addr32, sizeof(uint32_t) * 4);
		netfn_flowmgr_ppe_endianess_be_to_le(pd6rc->tuple.flow_ip, NETFN_FLOWMGR_TUPLE_IP_VERSION_V6);
		memcpy(pd6rc->tuple.return_ip, original->tuple.tuples.tuple_3.dest_ip.ip6.s6_addr32, sizeof(uint32_t) * 4);
		netfn_flowmgr_ppe_endianess_be_to_le(pd6rc->tuple.return_ip, NETFN_FLOWMGR_TUPLE_IP_VERSION_V6);
		pd6rc->tuple.flow_ident = (uint32_t)ntohs(original->tuple.tuples.tuple_5.l4_src_ident);
		pd6rc->tuple.return_ident = (uint32_t)ntohs(original->tuple.tuples.tuple_5.l4_dest_ident);
	}

	/*
	 * Fill connection specific data
	 */
	if (original->flow_info.in_dev && original->flow_info.out_dev) {
		in_dev = original->flow_info.in_dev;
		out_dev = original->flow_info.out_dev;
		dev_hold(in_dev);
		dev_hold(out_dev);

		/* PPE flow mac and dest mac are actual source and destination mac addresses */
		memcpy(pd6rc->conn_rule.flow_mac, original->flow_info.flow_src_mac, ETH_ALEN);
		memcpy(pd6rc->conn_rule.return_mac, original->flow_info.flow_dest_mac, ETH_ALEN);

		/* Get ppe interface indexes */
		pd6rc->conn_rule.rx_if = ppe_drv_iface_idx_get_by_dev(in_dev);
		if (pd6rc->conn_rule.rx_if == -1) {
			netfn_flowmgr_warn("Invalid PPE rx interface index\n");
			netfn_status = NETFN_FLOWMGR_RET_PPE_INVALID_INTERFACE_INDEX;
			netfn_flowmgr_stats_inc(&stats->v6_create_rule_invalid_ppe_rx_if);
			goto done;
		}
		pd6rc->conn_rule.tx_if = ppe_drv_iface_idx_get_by_dev(out_dev);
		if (pd6rc->conn_rule.tx_if == -1) {
			netfn_flowmgr_warn("Invalid PPE tx interface index\n");
			netfn_status = NETFN_FLOWMGR_RET_PPE_INVALID_INTERFACE_INDEX;
			netfn_flowmgr_stats_inc(&stats->v6_create_rule_invalid_ppe_tx_if);
			goto done;
		}
		pd6rc->top_rule.rx_if = ppe_drv_iface_idx_get_by_dev(in_dev);
		if (pd6rc->top_rule.rx_if == -1) {
			netfn_flowmgr_warn("Invalid PPE top rx interface index\n");
			netfn_status = NETFN_FLOWMGR_RET_PPE_INVALID_INTERFACE_INDEX;
			netfn_flowmgr_stats_inc(&stats->v6_create_rule_invalid_ppe_top_rx_if);
			goto done;
		}
		pd6rc->top_rule.tx_if = ppe_drv_iface_idx_get_by_dev(out_dev);
		if (pd6rc->top_rule.tx_if == -1) {
			netfn_flowmgr_warn("Invalid PPE top tx interface index\n");
			netfn_status = NETFN_FLOWMGR_RET_PPE_INVALID_INTERFACE_INDEX;
			netfn_flowmgr_stats_inc(&stats->v6_create_rule_invalid_ppe_top_tx_if);
			goto done;
		}

		/*
		 * MTU info
		 */
		pd6rc->conn_rule.flow_mtu = original->flow_info.flow_mtu;
		pd6rc->conn_rule.return_mtu = reply->flow_info.flow_mtu;

		dev_put(in_dev);
		dev_put(out_dev);
	} else {
		netfn_flowmgr_warn("Indev and outdev is invalid in flow rule \n");
		netfn_status = NETFN_FLOWMGR_RET_PPE_INVALID_DEV_IN_FLOW_RULE;
		goto done;
	}

	/*
	 * Fill PPE specific rule info from flow flags in netfn create rule structure
	 */
	org_flow_flags = original->flow_flags;
	reply_flow_flags = reply->flow_flags;

	if (org_flow_flags != reply_flow_flags) {
		netfn_flowmgr_warn("Non-matching flow flags in flow and return direction\n");
		netfn_status = NETFN_FLOWMGR_RET_FLOW_FLAGS_MISMATCH;
		netfn_flowmgr_stats_inc(&stats->v6_create_rule_flow_flag_mismatch);
		goto done;
	}

	/*
	 * Currently PPE supports rule to be pushed in both flow and return direction
	 */
	pd6rc->rule_flags |= PPE_DRV_V6_RULE_FLAG_RETURN_VALID | PPE_DRV_V6_RULE_FLAG_FLOW_VALID;

	if (org_flow_flags & NETFN_FLOWMGR_FLOW_FLAG_BRIDGE_FLOW) {
		/*
		 * Bridged flow
		 */
		pd6rc->rule_flags |= PPE_DRV_V6_RULE_FLAG_BRIDGE_FLOW;
		netfn_flowmgr_stats_inc(&stats->v6_create_rule_bridge_flow);
		netfn_flowmgr_warn("Flow is bridged flow\n");
	} else {
		/*
		 * Routed flow
		 */
		pd6rc->rule_flags |= PPE_DRV_V6_RULE_FLAG_ROUTED_FLOW;
		netfn_flowmgr_stats_inc(&stats->v6_create_rule_routed_flow);
		netfn_flowmgr_warn("Flow is routed flow\n");
	}

	if (org_flow_flags & NETFN_FLOWMGR_FLOW_FLAG_DS_FLOW) {
		/*
		 * DS flow
		 */
		pd6rc->rule_flags |= PPE_DRV_V4_RULE_FLAG_DS_FLOW;
		netfn_flowmgr_warn("Flow is DS flow\n");
	}

	if (org_flow_flags & NETFN_FLOWMGR_FLOW_FLAG_VP_FLOW) {
		/*
		 * VP flow
		 */
		pd6rc->rule_flags |= PPE_DRV_V4_RULE_FLAG_VP_FLOW;
		netfn_flowmgr_warn("Flow is VP flow\n");
	}

	/*
	 * Fill supported rule info
	 */
	org_rules = original->rule_info.rule_valid_flags;
	reply_rules = reply->rule_info.rule_valid_flags;

	netfn_flowmgr_trace("org_rules = 0x%x\n", org_rules);
	netfn_flowmgr_trace("reply_rules = 0x%x\n", reply_rules);

	/*
	 * VLAN rule
	 */
	pd6rc->vlan_rule.primary_vlan.ingress_vlan_tag = NETFN_FLOWMGR_VLAN_NOT_CONFIGURED;
	pd6rc->vlan_rule.primary_vlan.egress_vlan_tag = NETFN_FLOWMGR_VLAN_NOT_CONFIGURED;
	pd6rc->vlan_rule.secondary_vlan.ingress_vlan_tag = NETFN_FLOWMGR_VLAN_NOT_CONFIGURED;
	pd6rc->vlan_rule.secondary_vlan.egress_vlan_tag = NETFN_FLOWMGR_VLAN_NOT_CONFIGURED;
	if (org_rules & NETFN_FLOWMGR_VALID_RULE_FLAG_VLAN) {
		/*
                 * Validate VLAN info
                 */
                netfn_status = netfn_flowmgr_ppe_validate_vlan_info(original, reply);
                if (netfn_status != NETFN_FLOWMGR_RET_SUCCESS) {
                        netfn_flowmgr_warn("Invalid vlan info, PPE rule creation failed\n");
                        goto done;
                }

                pd6rc->vlan_rule.primary_vlan.ingress_vlan_tag = original->rule_info.vlan_rule.inner.ingress_vlan_tag;
                pd6rc->vlan_rule.primary_vlan.egress_vlan_tag = original->rule_info.vlan_rule.inner.egress_vlan_tag;
                pd6rc->vlan_rule.secondary_vlan.ingress_vlan_tag = original->rule_info.vlan_rule.outer.ingress_vlan_tag;
                pd6rc->vlan_rule.secondary_vlan.egress_vlan_tag = original->rule_info.vlan_rule.outer.egress_vlan_tag;
                pd6rc->valid_flags |= PPE_DRV_V6_VALID_FLAG_VLAN;
	}

	/*
         * PPPoE rule
         */
	if (org_rules & NETFN_FLOWMGR_VALID_RULE_FLAG_PPPOE) {
		pd6rc->pppoe_rule.flow_session.session_id = original->rule_info.pppoe_rule.session_id;
		memcpy(pd6rc->pppoe_rule.flow_session.server_mac, original->rule_info.pppoe_rule.server_mac, ETH_ALEN);
		pd6rc->valid_flags |= PPE_DRV_V6_VALID_FLAG_PPPOE_FLOW;
	}

	if (reply_rules & NETFN_FLOWMGR_VALID_RULE_FLAG_PPPOE) {
		pd6rc->pppoe_rule.return_session.session_id = reply->rule_info.pppoe_rule.session_id;
		memcpy(pd6rc->pppoe_rule.return_session.server_mac, reply->rule_info.pppoe_rule.server_mac, ETH_ALEN);
		pd6rc->valid_flags |= PPE_DRV_V6_VALID_FLAG_PPPOE_RETURN;
	}

	/*
	 * DSCP rule
	 * TODO: Check the DSCP functionality again.
	 *	 Currently this is as per the PPE requirement but
	 *	 it seems PPE code needs revisit.
	 */
	if (org_rules & NETFN_FLOWMGR_VALID_RULE_FLAG_DSCP_MARKING) {
		pd6rc->dscp_rule.flow_dscp = original->rule_info.dscp_rule.dscp_val;
		pd6rc->dscp_rule.return_dscp = reply->rule_info.dscp_rule.dscp_val;
		pd6rc->valid_flags |= PPE_DRV_V6_VALID_FLAG_DSCP_MARKING;
	}

	/*
	 * QoS rule
	 * TODO: Check the QOS functionality again.
	 *	 Currently this is as per the PPE requirement but
	 *	 it seems PPE code needs revisit.
	 */
	if (org_rules & NETFN_FLOWMGR_VALID_RULE_FLAG_QOS) {
		pd6rc->qos_rule.flow_qos_tag = original->rule_info.qos_rule.qos_tag;
		pd6rc->qos_rule.return_qos_tag = original->rule_info.qos_rule.qos_tag;
		pd6rc->qos_rule.flow_int_pri = original->rule_info.qos_rule.priority;
		pd6rc->qos_rule.qos_valid_flags |= PPE_DRV_VALID_FLAG_FLOW_PPE_QOS;
		pd6rc->valid_flags |= PPE_DRV_V4_VALID_FLAG_QOS;
	}

	if (reply_rules & NETFN_FLOWMGR_VALID_RULE_FLAG_QOS) {
		pd6rc->qos_rule.return_qos_tag = reply->rule_info.qos_rule.qos_tag;
		pd6rc->qos_rule.flow_qos_tag = reply->rule_info.qos_rule.qos_tag;
		pd6rc->qos_rule.return_int_pri = reply->rule_info.qos_rule.priority;
		pd6rc->qos_rule.qos_valid_flags |= PPE_DRV_VALID_FLAG_RETURN_PPE_QOS;
		pd6rc->valid_flags |= PPE_DRV_V4_VALID_FLAG_QOS;
	}

	/*
	 * Noedit rule
	 * This rule enables PPE to bypass packet header editing and forward
	 * them unmodified.
	 */
	if (org_rules & NETFN_FLOWMGR_VALID_RULE_FLAG_NOEDIT_RULE) {
		netfn_flowmgr_info("Noedit rule is confifured in flow direction\n");
		pd6rc->rule_flags |= PPE_DRV_V6_RULE_NOEDIT_FLOW_RULE;
	}

	if (reply_rules & NETFN_FLOWMGR_VALID_RULE_FLAG_NOEDIT_RULE) {
		netfn_flowmgr_info("Noedit rule is confifured in return direction\n");
		pd6rc->rule_flags |= PPE_DRV_V6_RULE_NOEDIT_RETURN_RULE;
	}

	/*
	 * Top if rule
	 */
	if (original->flow_info.top_indev && original->flow_info.top_outdev) {
		top_indev = original->flow_info.top_indev;
		top_outdev = original->flow_info.top_outdev;

		dev_hold(top_indev);
		dev_hold(top_outdev);
		pd6rc->top_rule.rx_if = ppe_drv_iface_idx_get_by_dev(top_indev);
		if (pd6rc->top_rule.rx_if == -1) {
			netfn_flowmgr_warn("Invalid PPE top rx interface index\n");
			netfn_status = NETFN_FLOWMGR_RET_PPE_INVALID_INTERFACE_INDEX;
			netfn_flowmgr_stats_inc(&stats->v6_create_rule_invalid_ppe_rx_if);
			goto done;
		}
		pd6rc->top_rule.tx_if = ppe_drv_iface_idx_get_by_dev(top_outdev);
		if (pd6rc->top_rule.tx_if == -1) {
			netfn_flowmgr_warn("Invalid PPE top tx interface index\n");
			netfn_status = NETFN_FLOWMGR_RET_PPE_INVALID_INTERFACE_INDEX;
			netfn_flowmgr_stats_inc(&stats->v6_create_rule_invalid_ppe_tx_if);
			goto done;
		}

		dev_put(top_indev);
		dev_put(top_outdev);
	} else {
		netfn_flowmgr_warn("Invalid top in and out dev\n");
		netfn_status = NETFN_FLOWMGR_RET_PPE_INVALID_TOP_DEV_IN_FLOW_RULE;
		goto done;
	}

	/*
         * Dump the information passed to PPE acceleratio engine
         */
        netfn_flowmgr_trace("Accelerate v6 flow to PPE:\n"
                                "flow_ip: %pI6\n"
                                "return_ip: %pI6\n"
                                "flow_ident: %u\n"
                                "return_ident: %u\n"
                                "protocol: %u\n"
                                "valid_flags: 0x%x\n"
                                "rule_flags: 0x%x\n"
                                "rx_if: %u\n"
                                "tx_if: %u\n"
                                "flow_mac: %pM\n"
                                "return_mac: %pM\n"
                                "flow_mtu: %u\n"
                                "return_mtu: %u\n"
                                "ingress_inner_vlan_tag: 0x%x\n"
                                "egress_inner_vlan_tag: 0x%x\n"
                                "ingress_outer_vlan_tag: 0x%x\n"
                                "egress_outer_vlan_tag: 0x%x\n"
                                "flow_session.session_id: %u\n"
                                "flow_session.server_mac: %pM\n"
                                "return_session.session_id: %u\n"
                                "return_session.server_mac: %pM\n",
                                &pd6rc->tuple.flow_ip,
                                &pd6rc->tuple.return_ip,
                                pd6rc->tuple.flow_ident,
                                pd6rc->tuple.return_ident,
                                pd6rc->tuple.protocol,
                                pd6rc->valid_flags,
                                pd6rc->rule_flags,
                                pd6rc->conn_rule.rx_if,
				pd6rc->conn_rule.tx_if,
                                pd6rc->conn_rule.flow_mac,
                                pd6rc->conn_rule.return_mac,
                                pd6rc->conn_rule.flow_mtu,
                                pd6rc->conn_rule.return_mtu,
                                pd6rc->vlan_rule.primary_vlan.ingress_vlan_tag,
                                pd6rc->vlan_rule.primary_vlan.egress_vlan_tag,
                                pd6rc->vlan_rule.secondary_vlan.ingress_vlan_tag,
                                pd6rc->vlan_rule.secondary_vlan.egress_vlan_tag,
                                pd6rc->pppoe_rule.flow_session.session_id,
                                pd6rc->pppoe_rule.flow_session.server_mac,
                                pd6rc->pppoe_rule.return_session.session_id,
                                pd6rc->pppoe_rule.return_session.server_mac);

	/*
	 * Call PPE exported v6 create rule API to create flow rule
	 */
	ppe_status = ppe_drv_v6_create(pd6rc);
	if (ppe_status != PPE_DRV_RET_SUCCESS) {
		netfn_flowmgr_warn("PPE v6 rule create failed, ppe_status = %d\n", ppe_status);
		netfn_status = NETFN_FLOWMGR_RET_CREATE_RULE_FAILED;
		netfn_flowmgr_stats_inc(&stats->v6_create_rule_req_fail_ppe_fail);
		goto done;
	}

	/*
	 * Update stats
	 */
	netfn_flowmgr_stats_inc(&stats->v6_create_rule_ppe_success);
	netfn_flowmgr_info("PPE v6 rule create success\n");
done:
	kfree(pd6rc);
	/*
	 * Setting both netfn and ppe specific error code
	 */
	return NETFN_FLOWMGR_SET_NETFN_STATUS_CODE(netfn_status, ppe_status);
}

/*
 * netfn_flowmgr_ppe_destroy_v6_rule()
 *	Destroy PPE v6 flow rule
 */
netfn_flowmgr_ret_t netfn_flowmgr_ppe_destroy_v6_rule(struct netfn_flowmgr_destroy_rule *original, struct netfn_flowmgr_destroy_rule *reply)
{
	struct netfn_flowmgr *f = &netfn_flowmgr_gbl;
	struct ppe_drv_v6_rule_destroy *pd6rd;
	netfn_tuple_type_t tuple_type;
	struct netfn_flowmgr_debug_stats *stats;
	ppe_drv_ret_t ppe_status = PPE_DRV_RET_SUCCESS;
	netfn_flowmgr_ret_status_t netfn_status = NETFN_FLOWMGR_RET_SUCCESS;
	stats = &f->stats[NETFN_FLOWMGR_AE_TYPE_PPE];

	/*
	 * Update stats
	 */
	netfn_flowmgr_stats_inc(&stats->v6_destroy_ppe_rule_req);

	/*
	 * Validate PPE tuple information
	 */
	netfn_status = netfn_flowmgr_ppe_validate_v6_tuple_info(original->tuple, reply->tuple);
	if (netfn_status != NETFN_FLOWMGR_RET_SUCCESS) {
		netfn_flowmgr_warn("Invalid v6 tuple info, PPE rule creation failed\n");
		netfn_flowmgr_stats_inc(&stats->v6_destroy_rule_req_fail_ppe_fail);
		return NETFN_FLOWMGR_SET_NETFN_STATUS_CODE(netfn_status, ppe_status);
	}

	/*
	 * Allocate memory for ppe ipv6 rule destroy structure
	 */
	pd6rd = (struct ppe_drv_v6_rule_destroy *)kzalloc(sizeof(struct ppe_drv_v6_rule_destroy), GFP_KERNEL);
	if (!pd6rd) {
		netfn_flowmgr_warn("No memory to allocate ppe ipv6 rule destroy structure\n");
		netfn_flowmgr_stats_inc(&stats->v6_destroy_rule_req_fail_no_mem);
		return NETFN_FLOWMGR_SET_NETFN_STATUS_CODE(NETFN_FLOWMGR_RET_NO_MEM, ppe_status);
	}

	tuple_type = original->tuple.tuple_type;

	/*
	 * Based on tuple type fill the PPE tuple info
	 */
	if (tuple_type == NETFN_TUPLE_3TUPLE) {
		/*
		 * Fill PPE three tuple info
		 */
		pd6rd->tuple.protocol = original->tuple.tuples.tuple_3.protocol;
		memcpy(pd6rd->tuple.flow_ip, original->tuple.tuples.tuple_3.src_ip.ip6.s6_addr32, sizeof(uint32_t) * 4);
		netfn_flowmgr_ppe_endianess_be_to_le(pd6rd->tuple.flow_ip, NETFN_FLOWMGR_TUPLE_IP_VERSION_V6);
		memcpy(pd6rd->tuple.return_ip, original->tuple.tuples.tuple_3.dest_ip.ip6.s6_addr32, sizeof(uint32_t) * 4);
		netfn_flowmgr_ppe_endianess_be_to_le(pd6rd->tuple.return_ip, NETFN_FLOWMGR_TUPLE_IP_VERSION_V6);
	} else if (tuple_type == NETFN_TUPLE_5TUPLE) {
		/*
		 * Fill PPE five tuple info
		 */
		pd6rd->tuple.protocol = original->tuple.tuples.tuple_5.protocol;
		memcpy(pd6rd->tuple.flow_ip, original->tuple.tuples.tuple_5.src_ip.ip6.s6_addr32, sizeof(uint32_t) * 4);
		netfn_flowmgr_ppe_endianess_be_to_le(pd6rd->tuple.flow_ip, NETFN_FLOWMGR_TUPLE_IP_VERSION_V6);
		memcpy(pd6rd->tuple.return_ip, original->tuple.tuples.tuple_5.dest_ip.ip6.s6_addr32, sizeof(uint32_t) * 4);
		netfn_flowmgr_ppe_endianess_be_to_le(pd6rd->tuple.return_ip, NETFN_FLOWMGR_TUPLE_IP_VERSION_V6);
		pd6rd->tuple.flow_ident = (uint32_t)ntohs(original->tuple.tuples.tuple_5.l4_src_ident);
		pd6rd->tuple.return_ident = (uint32_t)ntohs(original->tuple.tuples.tuple_5.l4_dest_ident);
	}

	/*
	 * Dump the information passed to PPE acceleratio engine
	 */
	netfn_flowmgr_trace("Destroy v6 flow from PPE:\n"
				"flow_ip: %pI6\n"
				"return_ip: %pI6\n"
				"flow_ident: %u\n"
				"return_ident: %u\n"
				"protocol: %u\n",
				&pd6rd->tuple.flow_ip,
				&pd6rd->tuple.return_ip,
				pd6rd->tuple.flow_ident,
				pd6rd->tuple.return_ident,
				pd6rd->tuple.protocol);
	/*
	 * Call PPE exported v6 destroy rule API to destroy flow rule
	 */
	ppe_status = ppe_drv_v6_destroy(pd6rd);
	if (ppe_status != PPE_DRV_RET_SUCCESS) {
		netfn_flowmgr_warn("PPE v6 rule destroy failed, ppe_status = %d\n", ppe_status);
		netfn_status = NETFN_FLOWMGR_RET_DESTROY_RULE_FAILED;
		netfn_flowmgr_stats_inc(&stats->v6_destroy_rule_req_fail_ppe_fail);
		goto done;
	}

	/*
	 * Update stats
	 */
	netfn_flowmgr_stats_inc(&stats->v6_destroy_rule_ppe_success);
	netfn_flowmgr_warn("PPE v6 rule destroy success\n");
done:
	kfree(pd6rd);
	/*
	 * Setting both netfn and ppe specific error code
	 */
	return NETFN_FLOWMGR_SET_NETFN_STATUS_CODE(netfn_status, ppe_status);
}

/*
 * netfn_flowmgr_ppe_ipv6_deinit()
 *	PPE IPv6 de-initialization
 */
void netfn_flowmgr_ppe_ipv6_deinit(void)
{
	netfn_flowmgr_warn("Netfn flowmgr PPE IPv6 de-initialization\n");

	if (ipv6_stats_sync) {
		netfn_flowmgr_ppe_ipv6_stats_deinit();
	}
}

/*
 * netfn_flowmgr_ppe_ipv6_init()
 *	PPE IPv6 initialization
 */
bool netfn_flowmgr_ppe_ipv6_init(void)
{
	bool status = true;

	netfn_flowmgr_warn("Netfn flowmgr PPE IPv6 initialization\n");

	if (enable_stats_sync) {
		netfn_flowmgr_trace("Stats sync is enabled, initialize PPE IPv6 stats\n");
		status = netfn_flowmgr_ppe_ipv6_stats_init();
		if (status != true) {
			netfn_flowmgr_warn("PPE IPv6 stats initialization failed\n");
			return status;
		}
		ipv6_stats_sync = true;
		return status;
	}
	return status;
}
