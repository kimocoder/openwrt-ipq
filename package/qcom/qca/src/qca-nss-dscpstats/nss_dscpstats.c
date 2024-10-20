/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/debugfs.h>

#include "nss_dscpstats.h"
#include "exports/nss_dscpstats_nl_public.h"
#include "nss_dscpstats_nl.h"

/*
 * nss_dscpstats_stop_ops()
 *	dscpstats stop handler
 */
static ssize_t nss_dscpstats_stop_ops(struct file *f, const char *buffer, size_t len, loff_t *offset)
{
	ssize_t size;
	char data[16];
	bool res;
	int status;

	size = simple_write_to_buffer(data, sizeof(data), offset, buffer, len);
	if (size < 0) {
		nss_dscpstats_warn("Error reading the input for l2tp configuration\n");
		return size;
	}

	status = kstrtobool(data, &res);
	if (status) {
		nss_dscpstats_warn("Error reading the input for l2tp configuration\n");
		return status;
	}

	nss_dscpstats_db_stop();

	return len;
}

/*
 * nss_dscpstats_stop_file_ops
 *	File operations for stopping the statistics update.
 */
const struct file_operations nss_dscpstats_stop_file_ops = {
	.owner = THIS_MODULE,
	.write = nss_dscpstats_stop_ops,
};

/*
 * nss_dscpstats_create_debugfs
 *	create debugfs entries.
 */
static bool nss_dscpstats_create_debugfs(void) {
	nss_dscpstats_ctx.dentry = debugfs_create_dir("qca-nss-dscpstats", NULL);
	if (nss_dscpstats_ctx.dentry == NULL) {
		nss_dscpstats_warn("Unable to create qca-nss-dscpstats directory in debugfs\n");
		return false;
	}

	nss_dscpstats_ctx.conf_dentry = debugfs_create_dir("conf", nss_dscpstats_ctx.dentry);
	if (nss_dscpstats_ctx.conf_dentry == NULL) {
		nss_dscpstats_warn("Unable to create conf directory in debugfs\n");
		goto debugfs_dir_failed;
	}

	if (!debugfs_create_file("stop", 0200, nss_dscpstats_ctx.conf_dentry,
					NULL, &nss_dscpstats_stop_file_ops)) {
		nss_dscpstats_warn("Unable to create flush file in debugfs\n");
		goto debugfs_dir_failed;
	}

	return 0;

debugfs_dir_failed:
	debugfs_remove_recursive(nss_dscpstats_ctx.dentry);
	nss_dscpstats_ctx.dentry = NULL;
	return 1;
}

/*
 * nss_dscpstats_remove_debugfs
 *	remove debugfs entries.
 */
static void nss_dscpstats_remove_debugfs(void) {
	debugfs_remove_recursive(nss_dscpstats_ctx.dentry);
}

/*
 * qca_nss_dscp_stat_init()
 *	register netlink ops and initialize hash table
 *
 * returns 0 on successful initialization else return -1
 */
int __init nss_dscpstats_init(void)
{
	int error = 0;

	nss_dscpstats_info("Init NSS client dscp stats handler\n");

	if (num_clients < NSS_DSCPSTATS_CLIENT_HASH_RATIO) {
		num_clients = NSS_DSCPSTATS_CLIENT_HASH_RATIO;
	}

	error = nss_dscpstats_db_init_table();
	if (error != 0) {
		return error;
	}

	error = nss_dscpstats_nl_init();
	if (error != 0) {
		nss_dscpstats_db_free_table();
		return error;
	}

	error = nss_dscpstats_create_debugfs();
	if (error != 0) {
		nss_dscpstats_nl_exit();
		nss_dscpstats_db_free_table();
		return error;
	}

	nss_dscpstats_info("Init dscpstats complete\n");
	return 0;
}

/*
 * qca_nss_dscp_stat_init()
 *	unregister netlink ops and free the hash table
 *
 */
void __exit nss_dscpstats_exit(void)
{
	nss_dscpstats_info("Exit dscpstats complete\n");

	nss_dscpstats_remove_debugfs();

	nss_dscpstats_nl_exit();

	/*
	 * free memory assigned to hashtable
	 */
	nss_dscpstats_db_free_table();
}

module_init(nss_dscpstats_init);
module_exit(nss_dscpstats_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("DSCP STAT");

module_param(num_clients, int, 0644);
MODULE_PARM_DESC(num_clients, "Number of clients supported");
