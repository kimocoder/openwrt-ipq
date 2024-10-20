/*
 * Copyright (c) 2022-2024, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/debugfs.h>
#include "edma.h"
#include "edma_debug.h"
#include "edma_debugfs.h"

/*
 * edma_debugfs_ring_usage_dump
 * 	Format to print the EDMA ring utilization (full)
 */
const char *edma_debugfs_ring_usage_dump[EDMA_RING_USAGE_MAX_FULL] = {
	"100 percentage full",
	"90 to 100 percentage full",
	"70 to 90 percentage full",
	"50 to 70 percentage full",
	"Less than 50 percentage full"
};

/*
 * edma_debugfs_ring_usage_rx_fill_dump
 * 	Format to print EDMA Rx fill ring empty
 */
const char *edma_debugfs_ring_usage_rx_fill_dump[EDMA_RING_USAGE_MAX_FULL] = {
	"100 percentage empty",
	"90 to 100 percentage empty",
	"70 to 90 percentage empty",
	"50 to 70 percentage empty",
	"Less than 50 percentage empty"
};

/*
 * edma_txcmpl_err_string
 *	EDMA Tx complete error string.
 */
const char *edma_txcmpl_err_string[EDMA_TX_CMPL_ERR_MAX] = {
	"IP header length error",
	"TSO error",
	"IPv6 data length error",
	"TCP header error",
	"TCP header offset error",
	"TCP data offset error",
	"UDP header error",
	"UDP header offset error",
	"UDP data offset error",
	"UDPLite header error",
	"UDPLite header offset error",
	"UDPLite csum cov error",
	"IP version error",
	"L4 offset < L3 offset error",
	"L4 offset out of bounds error",
	"L3 offset out of bounds error",
	"Payload offset out of bounds error",
	"Custom Checksum offset out of bounds error",
	"Reserved",
	"Reserved",
	"Reserved",
	"TSO MSS error",
	"TSO TCP packet error"
};

/*
 * edma_debugfs_print_banner()
 *	API to print the banner for a node
 */
static void edma_debugfs_print_banner(struct seq_file *m, char *node)
{
	uint32_t banner_char_len, i;

	for (i = 0; i < EDMA_STATS_BANNER_MAX_LEN; i++) {
		seq_printf(m, "_");
	}

	banner_char_len = (EDMA_STATS_BANNER_MAX_LEN - (strlen(node) + 2)) / 2;

	seq_printf(m, "\n\n");

	for (i = 0; i < banner_char_len; i++) {
		seq_printf(m, "<");
	}

	seq_printf(m, " %s ", node);

	for (i = 0; i < banner_char_len; i++) {
		seq_printf(m, ">");
	}
	seq_printf(m, "\n");

	for (i = 0; i < EDMA_STATS_BANNER_MAX_LEN; i++) {
		seq_printf(m, "_");
	}

	seq_printf(m, "\n\n");
}

/*
 * edma_debugfs_rx_rings_stats_show()
 *	EDMA debugfs rx rings stats show API
 */
static int edma_debugfs_rx_rings_stats_show(struct seq_file *m, void __attribute__((unused))*p)
{
	struct edma_rx_fill_stats *rx_fill_stats;
	struct edma_rx_desc_stats *rx_desc_stats;
	struct edma_gbl_ctx *egc = &edma_gbl_ctx;
	uint32_t rx_fill_start_id = egc->rxfill_ring_start;
	uint32_t rx_desc_start_id = egc->rxdesc_ring_start;
	uint32_t i, j;
	unsigned int start;
#ifdef NSS_DP_PPEDS_SUPPORT
	struct edma_rxfill_ring *rxfill_ring;
	struct edma_rxdesc_ring *rxdesc_ring;
	struct edma_ppeds_drv *drv = &edma_gbl_ctx.ppeds_drv;
	struct edma_ppeds *ppeds_node;
#endif

	rx_fill_stats = kzalloc(egc->num_rxfill_rings * sizeof(struct edma_rx_fill_stats),
				 GFP_KERNEL);
	if (!rx_fill_stats) {
		edma_err("Error in allocating the Rx fill stats buffer\n");
		return -ENOMEM;
	}

	rx_desc_stats = kzalloc(egc->num_rxdesc_rings * sizeof(struct edma_rx_desc_stats),
				 GFP_KERNEL);
	if (!rx_desc_stats) {
		edma_err("Error in allocating the Rx descriptor stats buffer\n");
		kfree(rx_fill_stats);
		return -ENOMEM;
	}

	/*
	 * Get stats for Rx fill rings
	 */
	for (i = 0; i < egc->num_rxfill_rings; i++) {
		struct edma_rxfill_ring *rxfill_ring;
		struct edma_rx_fill_stats *stats;

		rxfill_ring = &egc->rxfill_rings[i];
		stats = &rxfill_ring->rx_fill_stats;
		do {
			start = edma_dp_stats_fetch_begin(&stats->syncp);
			rx_fill_stats[i].alloc_failed = stats->alloc_failed;
			rx_fill_stats[i].page_alloc_failed = stats->page_alloc_failed;
			memcpy(&rx_fill_stats[i].ring_stats, &stats->ring_stats,
					sizeof(struct edma_ring_util_stats));
		} while (edma_dp_stats_fetch_retry(&stats->syncp, start));
	}

	/*
	 * Get stats for Rx Desc rings
	 */
	for (i = 0; i < edma_gbl_ctx.num_rxdesc_rings; i++) {
		struct edma_rxdesc_ring *rxdesc_ring;
		struct edma_rx_desc_stats *stats;

		rxdesc_ring = &egc->rxdesc_rings[i];
		stats = &rxdesc_ring->rx_desc_stats;
		do {
			start = edma_dp_stats_fetch_begin(&stats->syncp);
			rx_desc_stats[i].src_port_inval = stats->src_port_inval;
			rx_desc_stats[i].src_port_inval_type = stats->src_port_inval_type;
			rx_desc_stats[i].src_port_inval_netdev = stats->src_port_inval_netdev;
			memcpy(&rx_desc_stats[i].ring_stats, &stats->ring_stats,
					sizeof(struct edma_ring_util_stats));
		} while (edma_dp_stats_fetch_retry(&stats->syncp, start));
	}

	edma_debugfs_print_banner(m, EDMA_RX_RING_STATS_NODE_NAME);

	seq_printf(m, "\n#EDMA RX descriptor rings stats:\n\n");
	for (i = 0; i < edma_gbl_ctx.num_rxdesc_rings; i++) {
		seq_printf(m, "\t\tEDMA RX descriptor %d ring stats:\n", i + rx_desc_start_id);
		seq_printf(m, "\t\t rxdesc[%d]:src_port_inval = %llu\n",
				i + rx_desc_start_id, rx_desc_stats[i].src_port_inval);
		seq_printf(m, "\t\t rxdesc[%d]:src_port_inval_type = %llu\n",
				i + rx_desc_start_id, rx_desc_stats[i].src_port_inval_type);
		seq_printf(m, "\t\t rxdesc[%d]:src_port_inval_netdev = %llu\n\n",
				i + rx_desc_start_id,
				rx_desc_stats[i].src_port_inval_netdev);
		seq_printf(m, "\t\t Rx Descriptor ring full utilization stats\n");
		for (j = 0; j < EDMA_RING_USAGE_MAX_FULL; j++) {
			 seq_printf(m, "\t\t %s utilized %d times\n", edma_debugfs_ring_usage_dump[j],
					 rx_desc_stats[i].ring_stats.util[j]);
		}
		seq_printf(m, "\n");
	}

	seq_printf(m, "\n#EDMA RX fill rings stats:\n\n");
	for (i = 0; i < edma_gbl_ctx.num_rxfill_rings; i++) {
		seq_printf(m, "\t\tEDMA RX fill %d ring stats:\n", i + rx_fill_start_id);
		seq_printf(m, "\t\t rxfill[%d]:alloc_failed = %llu\n",
				i + rx_fill_start_id, rx_fill_stats[i].alloc_failed);
		seq_printf(m, "\t\t rxfill[%d]:page_alloc_failed = %llu\n\n",
				i + rx_fill_start_id, rx_fill_stats[i].page_alloc_failed);
		seq_printf(m, "\t\t Rx fill ring empty stats\n");
		for (j = 0; j < EDMA_RING_USAGE_MAX_FULL; j++) {
			seq_printf(m, "\t\t %s occurred %d times\n", edma_debugfs_ring_usage_rx_fill_dump[j],
					rx_fill_stats[i].ring_stats.util[j]);
		}
		seq_printf(m, "\n");
	}

#ifdef NSS_DP_PPEDS_SUPPORT
	edma_debugfs_print_banner(m, EDMA_RX_RING_PPEDS_STATS_NODE_NAME);

	for (i = 0; i < drv->num_nodes; i++) {
		ppeds_node = drv->ppeds_node_cfg[i].ppeds_db;
		if (!ppeds_node) {
			continue;
		}

		rxfill_ring = &ppeds_node->rxfill_ring;
		seq_printf(m, "\t\t PPE-DS Rx fill ring empty stats & Ring id %d\n", rxfill_ring->ring_id);

		for (j = 0; j < EDMA_RING_USAGE_MAX_FULL; j++) {
			seq_printf(m, "\t\t %s occurred %d times\n", edma_debugfs_ring_usage_rx_fill_dump[j],
					rxfill_ring->rx_fill_stats.ring_stats.util[j]);
		}

		seq_printf(m, "\n");
	}

	for (i = 0; i < drv->num_nodes; i++) {
		ppeds_node = drv->ppeds_node_cfg[i].ppeds_db;
		if (!ppeds_node) {
			continue;
		}

		rxdesc_ring = &ppeds_node->rx_ring;
		seq_printf(m, "\t\t PPE-DS Rx desc ring full utilization stats & Ring id %d\n", rxdesc_ring->ring_id);

		for (j = 0; j < EDMA_RING_USAGE_MAX_FULL; j++) {
			seq_printf(m, "\t\t %s utilized %d times\n", edma_debugfs_ring_usage_dump[j],
					rxdesc_ring->rx_desc_stats.ring_stats.util[j]);
		}

		seq_printf(m, "\n");
	}
#endif
	kfree(rx_fill_stats);
	kfree(rx_desc_stats);
	return 0;
}

/*
 * edma_debugfs_tx_rings_stats_show()
 *	EDMA debugfs Tx rings stats show API
 */
static int edma_debugfs_tx_rings_stats_show(struct seq_file *m, void __attribute__((unused))*p)
{
	struct edma_tx_cmpl_stats *tx_cmpl_stats;
	struct edma_tx_desc_stats *tx_desc_stats;
	struct edma_gbl_ctx *egc = &edma_gbl_ctx;
	uint32_t tx_cmpl_start_id = egc->txcmpl_ring_start;
	uint32_t tx_desc_start_id = egc->txdesc_ring_start;
	uint32_t i, j;
	unsigned int start;
#ifdef NSS_DP_PPEDS_SUPPORT
	struct edma_txdesc_ring *tx_ring;
	struct edma_txcmpl_ring *txcmpl_ring;
	struct edma_ppeds_drv *drv = &edma_gbl_ctx.ppeds_drv;
	struct edma_ppeds *ppeds_node;
#endif

	tx_cmpl_stats = kzalloc(egc->num_txcmpl_rings * sizeof(struct edma_tx_cmpl_stats), GFP_KERNEL);
	if (!tx_cmpl_stats) {
		edma_err("Error in allocating the Tx complete stats buffer\n");
		return -ENOMEM;
	}

	tx_desc_stats = kzalloc(egc->num_txdesc_rings * sizeof(struct edma_tx_desc_stats), GFP_KERNEL);
	if (!tx_desc_stats) {
		edma_err("Error in allocating the Tx descriptor stats buffer\n");
		kfree(tx_cmpl_stats);
		return -ENOMEM;
	}

	/*
	 * Get stats for Tx desc rings
	 */
	for (i = 0; i < egc->num_txdesc_rings; i++) {
		struct edma_txdesc_ring *txdesc_ring;
		struct edma_tx_desc_stats *stats;

		txdesc_ring = &egc->txdesc_rings[i];
		stats = &txdesc_ring->tx_desc_stats;
		do {
			start = edma_dp_stats_fetch_begin(&stats->syncp);
			tx_desc_stats[i].no_desc_avail = stats->no_desc_avail;
			tx_desc_stats[i].tso_max_seg_exceed = stats->tso_max_seg_exceed;
			memcpy(&tx_desc_stats[i].ring_stats, &stats->ring_stats,
			       sizeof(struct edma_ring_util_stats));
		} while (edma_dp_stats_fetch_retry(&stats->syncp, start));
	}

	/*
	 * Get stats for Tx Complete rings
	 */
	for (i = 0; i < egc->num_txcmpl_rings; i++) {
		struct edma_txcmpl_ring *txcmpl_ring;
		struct edma_tx_cmpl_stats *stats;

		txcmpl_ring = &egc->txcmpl_rings[i];
		stats = &txcmpl_ring->tx_cmpl_stats;
		do {
			start = edma_dp_stats_fetch_begin(&stats->syncp);
			tx_cmpl_stats[i].invalid_buffer = stats->invalid_buffer;
			memcpy(tx_cmpl_stats[i].errors, stats->errors, sizeof(uint64_t) * EDMA_TX_CMPL_ERR_MAX);
			tx_cmpl_stats[i].desc_with_more_bit = stats->desc_with_more_bit;
			tx_cmpl_stats[i].no_pending_desc = stats->no_pending_desc;
		} while (edma_dp_stats_fetch_retry(&stats->syncp, start));
	}

	edma_debugfs_print_banner(m, EDMA_TX_RING_STATS_NODE_NAME);

	seq_printf(m, "\n#EDMA TX complete rings stats:\n\n");
	for (i = 0; i < edma_gbl_ctx.num_txcmpl_rings; i++) {
		seq_printf(m, "\t\tEDMA TX complete %d ring stats:\n", i + tx_cmpl_start_id);
		seq_printf(m, "\t\t txcmpl[%d]:invalid_buffer = %llu\n",
				i + tx_cmpl_start_id, tx_cmpl_stats[i].invalid_buffer);
		for (j = 0; j < EDMA_TX_CMPL_ERR_MAX; j++) {
			if (!strcmp(edma_txcmpl_err_string[j], "Reserved")) {
				continue;
			}
			seq_printf(m, "\t\t txcmpl[%d]:%s = %llu\n",
					i + tx_cmpl_start_id, edma_txcmpl_err_string[j], tx_cmpl_stats[i].errors[j]);
		}
		seq_printf(m, "\t\t txcmpl[%d]:desc_with_more_bit = %llu\n",
				i + tx_cmpl_start_id, tx_cmpl_stats[i].desc_with_more_bit);
		seq_printf(m, "\t\t txcmpl[%d]:no_pending_desc = %llu\n",
				i + tx_cmpl_start_id, tx_cmpl_stats[i].no_pending_desc);
		seq_printf(m, "\n");
	}

	seq_printf(m, "\n#EDMA TX descriptor rings stats:\n\n");
	for (i = 0; i < edma_gbl_ctx.num_txdesc_rings; i++) {
		seq_printf(m, "\t\tEDMA TX descriptor %d ring stats:\n", i + tx_desc_start_id);
		seq_printf(m, "\t\t txdesc[%d]:no_desc_avail = %llu\n",
				i + tx_desc_start_id, tx_desc_stats[i].no_desc_avail);
		seq_printf(m, "\t\t txdesc[%d]:tso_max_seg_exceed = %llu\n\n",
				i + tx_desc_start_id, tx_desc_stats[i].tso_max_seg_exceed);
		seq_printf(m, "\t\t Tx descriptor ring full utilization stats\n");
		for (j = 0; j < EDMA_RING_USAGE_MAX_FULL; j++) {
			seq_printf(m, "\t\t %s utilized %d times\n", edma_debugfs_ring_usage_dump[j],
					tx_desc_stats[i].ring_stats.util[j]);
		}
		seq_printf(m, "\n");
	}

#ifdef NSS_DP_PPEDS_SUPPORT
	edma_debugfs_print_banner(m, EDMA_TX_RING_PPEDS_STATS_NODE_NAME);

	for (i = 0; i < drv->num_nodes; i++) {
		ppeds_node = drv->ppeds_node_cfg[i].ppeds_db;
		if (!ppeds_node) {
			continue;
		}

		tx_ring = &ppeds_node->tx_ring;
		seq_printf(m, "\t\t PPE-DS Tx Ring full utilization stats & Ring id %d\n", tx_ring->id);

		for (j = 0; j < EDMA_RING_USAGE_MAX_FULL; j++) {
			seq_printf(m, "\t\t %s utilized %d times\n", edma_debugfs_ring_usage_dump[j],
					tx_ring->tx_desc_stats.ring_stats.util[j]);
		}

		seq_printf(m, "\n");
	}

	for (i = 0; i < drv->num_nodes; i++) {
		ppeds_node = drv->ppeds_node_cfg[i].ppeds_db;
		if (!ppeds_node) {
			continue;
		}

		txcmpl_ring = &ppeds_node->txcmpl_ring;
		seq_printf(m, "\t\t PPE-DS Tx cmpl Ring full utilization stats & Ring id %d\n", txcmpl_ring->id);

		for (j = 0; j < EDMA_RING_USAGE_MAX_FULL; j++) {
			seq_printf(m, "\t\t %s utilized %d times\n", edma_debugfs_ring_usage_dump[j],
					txcmpl_ring->tx_cmpl_stats.ring_stats.util[j]);
		}

		seq_printf(m, "\n");
	}
#endif

	kfree(tx_cmpl_stats);
	kfree(tx_desc_stats);
	return 0;
}

/*
 * edma_debugfs_misc_stats_show()
 *	EDMA debugfs miscellaneous stats show API
 */
static int edma_debugfs_misc_stats_show(struct seq_file *m, void __attribute__((unused))*p)
{
	struct edma_misc_stats *misc_stats, *pcpu_misc_stats;
	uint32_t cpu;
	unsigned int start;

	misc_stats = kzalloc(sizeof(struct edma_misc_stats), GFP_KERNEL);
	if (!misc_stats) {
		edma_err("Error in allocating the miscellaneous stats buffer\n");
		return -ENOMEM;
	}

	/*
	 * Get percpu EDMA miscellaneous stats
	 */
	for_each_possible_cpu(cpu) {
		pcpu_misc_stats = per_cpu_ptr(edma_gbl_ctx.misc_stats, cpu);
		do {
			start = edma_dp_stats_fetch_begin(&pcpu_misc_stats->syncp);
			misc_stats->edma_misc_axi_read_err +=
				pcpu_misc_stats->edma_misc_axi_read_err;
			misc_stats->edma_misc_axi_write_err +=
				pcpu_misc_stats->edma_misc_axi_write_err;
			misc_stats->edma_misc_rx_desc_fifo_full +=
				pcpu_misc_stats->edma_misc_rx_desc_fifo_full;
			misc_stats->edma_misc_rx_buf_size_err +=
				pcpu_misc_stats->edma_misc_rx_buf_size_err;
			misc_stats->edma_misc_tx_sram_full +=
				pcpu_misc_stats->edma_misc_tx_sram_full;
			misc_stats->edma_misc_tx_data_len_err +=
				pcpu_misc_stats->edma_misc_tx_data_len_err;
			misc_stats->edma_misc_tx_timeout +=
				pcpu_misc_stats->edma_misc_tx_timeout;
			misc_stats->edma_misc_tx_cmpl_buf_full +=
				pcpu_misc_stats->edma_misc_tx_cmpl_buf_full;
		} while (edma_dp_stats_fetch_retry(&pcpu_misc_stats->syncp, start));
	}

	edma_debugfs_print_banner(m, EDMA_MISC_STATS_NODE_NAME);

	seq_printf(m, "\n#EDMA miscellaneous stats:\n\n");
	seq_printf(m, "\t\t miscellaneous axi read error = %llu\n",
			misc_stats->edma_misc_axi_read_err);
	seq_printf(m, "\t\t miscellaneous axi write error = %llu\n",
			misc_stats->edma_misc_axi_write_err);
	seq_printf(m, "\t\t miscellaneous Rx descriptor fifo full = %llu\n",
			misc_stats->edma_misc_rx_desc_fifo_full);
	seq_printf(m, "\t\t miscellaneous Rx buffer size error = %llu\n",
			misc_stats->edma_misc_rx_buf_size_err);
	seq_printf(m, "\t\t miscellaneous Tx SRAM full = %llu\n",
			misc_stats->edma_misc_tx_sram_full);
	seq_printf(m, "\t\t miscellaneous Tx data length error = %llu\n",
			misc_stats->edma_misc_tx_data_len_err);
	seq_printf(m, "\t\t miscellaneous Tx timeout = %llu\n",
			misc_stats->edma_misc_tx_timeout);
	seq_printf(m, "\t\t miscellaneous Tx completion buffer full = %llu\n",
			misc_stats->edma_misc_tx_cmpl_buf_full);

	kfree(misc_stats);
	return 0;
}

/*
 * edma_debugfs_clear_ring_stats()
 *      EDMA debugfs clearing the ring stats
 */
static int edma_debugfs_clear_ring_stats(struct seq_file *m, void __attribute__((unused))*p)
{
	struct edma_gbl_ctx *egc = &edma_gbl_ctx;
	uint32_t i;
#ifdef NSS_DP_PPEDS_SUPPORT
	struct edma_ppeds_drv *drv = &edma_gbl_ctx.ppeds_drv;
	struct edma_ppeds *ppeds_node;
#endif

	for (i = 0; i < egc->num_rxfill_rings; i++) {
		memset(&egc->rxfill_rings[i].rx_fill_stats, 0, sizeof(struct edma_rx_fill_stats));
	}

	for (i = 0; i < edma_gbl_ctx.num_rxdesc_rings; i++) {
		memset(&egc->rxdesc_rings[i].rx_desc_stats, 0, sizeof(struct edma_rx_desc_stats));
	}


	for (i = 0; i < egc->num_txdesc_rings; i++) {
		memset(&egc->txdesc_rings[i].tx_desc_stats, 0, sizeof(struct edma_tx_desc_stats));
	}

#ifdef NSS_DP_PPEDS_SUPPORT
	for (i = 0; i < drv->num_nodes; i++) {
		ppeds_node = drv->ppeds_node_cfg[i].ppeds_db;
		if (!ppeds_node) {
			continue;
		}

		memset(&ppeds_node->rxfill_ring.rx_fill_stats, 0, sizeof(struct edma_rx_fill_stats));
		memset(&ppeds_node->rx_ring.rx_desc_stats, 0, sizeof(struct edma_rx_desc_stats));
		memset(&ppeds_node->tx_ring.tx_desc_stats, 0, sizeof(struct edma_tx_desc_stats));
		memset(&ppeds_node->txcmpl_ring.tx_cmpl_stats, 0, sizeof(struct edma_tx_cmpl_stats));
	}
#endif

	seq_printf(m, "Resetting the EDMA Ring stats\n");
	return 0;
}

#if defined(NSS_DP_EDMA_LOOPBACK_SUPPORT)
/*
 * edma_debugfs_loopback_stats_show()
 *	EDMA debugfs loopback stats show API
 */
static int edma_debugfs_loopback_stats_show(struct seq_file *m, void __attribute__((unused))*p)
{
	struct edma_gbl_ctx *egc = &edma_gbl_ctx;
	int i = 0;

	seq_printf(m, "\n#EDMA loopback configuration stats:\n\n");
	seq_printf(m, "\t\t Loopback enabled = %d\n", egc->loopback_en);
	seq_printf(m, "\t\t Loopback ring size = %d\n", egc->loopback_ring_size);
	seq_printf(m, "\t\t Loopback buffer size = %d\n",egc->loopback_buf_size);
	seq_printf(m, "\t\t Number of loopback rings = %d\n", egc->num_loopback_rings);
	seq_printf(m, "\t\t Loopback queue base = %d\n", egc->loopback_queue_base);
	seq_printf(m, "\t\t Number of loopback queues = %d\n", egc->loopback_num_queues);

	for (i = 0; i < egc->num_loopback_rings; i++) {
		seq_printf(m, "\t\t#EDMA loopback ring info: %d\n\n", i);
		seq_printf(m, "\t\t TX descriptor loopback_ring_id = %d\n", egc->txdesc_loopback_ring_id_arr[i]);
		seq_printf(m, "\t\t TX completion loopback_ring_id = %d\n", egc->txcmpl_loopback_ring_id_arr[i]);
		seq_printf(m, "\t\t RX descriptor loopback_ring_id = %d\n", egc->rxdesc_loopback_ring_id_arr[i]);
		seq_printf(m, "\t\t RX fill loopback_ring_id = %d\n", egc->rxfill_loopback_ring_id_arr[i]);
	}

	return 0;
}

/*
 * edma_debugs_loopback_stats_open()
 *	EDMA debugfs loopback stats open callback API
 */
static int edma_debugs_loopback_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, edma_debugfs_loopback_stats_show, inode->i_private);
}

/*
 * edma_debugfs_misc_file_ops
 *	File operations for EDMA miscellaneous stats
 */
const struct file_operations edma_debugfs_loopback_file_ops = {
	.open = edma_debugs_loopback_stats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};
#endif

/*
 * edma_debugs_rx_rings_stats_open()
 *	EDMA debugfs Rx rings open callback API
 */
static int edma_debugs_rx_rings_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, edma_debugfs_rx_rings_stats_show, inode->i_private);
}

/*
 * edma_debugfs_rx_rings_file_ops
 *	File operations for EDMA Rx rings stats
 */
const struct file_operations edma_debugfs_rx_rings_file_ops = {
	.open = edma_debugs_rx_rings_stats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};

/*
 * edma_debugs_tx_rings_stats_open()
 *	EDMA debugfs Tx rings open callback API
 */
static int edma_debugs_tx_rings_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, edma_debugfs_tx_rings_stats_show, inode->i_private);
}

/*
 * edma_debugfs_tx_rings_file_ops
 *	File operations for EDMA Tx rings stats
 */
const struct file_operations edma_debugfs_tx_rings_file_ops = {
	.open = edma_debugs_tx_rings_stats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};

/*
 * edma_debugs_misc_stats_open()
 *	EDMA debugfs miscellaneous stats open callback API
 */
static int edma_debugs_misc_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, edma_debugfs_misc_stats_show, inode->i_private);
}

/*
 * edma_debugfs_misc_file_ops
 *	File operations for EDMA miscellaneous stats
 */
const struct file_operations edma_debugfs_misc_file_ops = {
	.open = edma_debugs_misc_stats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};

/* edma_debugs_clear_ring_stats()
 * 	EDMA debugfs clear stats open callback API
 */
static int edma_debugs_clear_ring_stats(struct inode *inode, struct file *file)
{
	return single_open(file, edma_debugfs_clear_ring_stats, inode->i_private);
}

/*
 * edma_debugfs_clear_ring_stats_ops
 * 	File operations for clearing ring stats
 */
const struct file_operations edma_debugfs_clear_ring_stats_ops = {
	.open = edma_debugs_clear_ring_stats,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};

/*
 * edma_debugfs_init()
 *	EDMA debugfs init API
 */
int edma_debugfs_init(void)
{
	edma_gbl_ctx.root_dentry = debugfs_create_dir("qca-nss-dp", NULL);
	if (!edma_gbl_ctx.root_dentry) {
		edma_err("Unable to create debugfs qca-nss-dp directory in debugfs\n");
		return -1;
	}

	edma_gbl_ctx.stats_dentry = debugfs_create_dir("stats", edma_gbl_ctx.root_dentry);
	if (!edma_gbl_ctx.stats_dentry) {
		edma_err("Unable to create debugfs stats directory in debugfs\n");
		goto debugfs_dir_failed;
	}

	if (!debugfs_create_file("rx_ring_stats", S_IRUGO, edma_gbl_ctx.stats_dentry,
			NULL, &edma_debugfs_rx_rings_file_ops)) {
		edma_err("Unable to create Rx rings statistics file entry in debugfs\n");
		goto debugfs_dir_failed;
	}

	if (!debugfs_create_file("tx_ring_stats", S_IRUGO, edma_gbl_ctx.stats_dentry,
			NULL, &edma_debugfs_tx_rings_file_ops)) {
		edma_err("Unable to create Tx rings statistics file entry in debugfs\n");
		goto debugfs_dir_failed;
	}

	if (!debugfs_create_file("clear_ring_stats", S_IRUGO, edma_gbl_ctx.stats_dentry,
			NULL, &edma_debugfs_clear_ring_stats_ops)) {
		edma_err("Unable to create clear rings statistics file entry in debugfs\n");
		goto debugfs_dir_failed;
	}

	/*
	 * Allocate memory for EDMA miscellaneous stats
	 */
	if (edma_misc_stats_alloc() < 0) {
		edma_err("Unable to allocate miscellaneous percpu stats\n");
		goto debugfs_dir_failed;
	}

	if (!debugfs_create_file("misc_stats", S_IRUGO, edma_gbl_ctx.stats_dentry,
			NULL, &edma_debugfs_misc_file_ops)) {
		edma_err("Unable to create EDMA miscellaneous statistics file entry in debugfs\n");
		goto debugfs_dir_failed;
	}

#if defined(NSS_DP_EDMA_LOOPBACK_SUPPORT)
	if (!debugfs_create_file("loopback_stats", S_IRUGO, edma_gbl_ctx.stats_dentry,
			NULL, &edma_debugfs_loopback_file_ops)) {
		edma_err("Unable to create EDMA loopback statistics file entry in debugfs\n");
		goto debugfs_dir_failed;
	}

#endif

	return 0;

debugfs_dir_failed:
	debugfs_remove_recursive(edma_gbl_ctx.root_dentry);
	edma_gbl_ctx.root_dentry = NULL;
	edma_gbl_ctx.stats_dentry = NULL;
	return -1;
}

/*
 * edma_debugfs_exit()
 *	EDMA debugfs exit API
 */
void edma_debugfs_exit(void)
{
	/*
	 * Free EDMA miscellaneous stats memory
	 */
	edma_misc_stats_free();

	if (edma_gbl_ctx.root_dentry) {
		debugfs_remove_recursive(edma_gbl_ctx.root_dentry);
		edma_gbl_ctx.root_dentry = NULL;
		edma_gbl_ctx.stats_dentry = NULL;
	}
}
