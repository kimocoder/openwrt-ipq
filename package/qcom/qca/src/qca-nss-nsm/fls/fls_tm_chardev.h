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

#include <linux/spinlock.h>

#define FLS_TM_CHARDEV_NAME "fls_tm"
#define FLS_TM_CHARDEV_MSG_MAX 256
#define FLS_TM_CHARDEV_MSG_MASK (FLS_TM_CHARDEV_MSG_MAX - 1)

struct fls_msg_log {
	uint32_t write_index;
	struct fls_tm_flow flow_ring_buf[FLS_TM_CHARDEV_MSG_MAX];
	spinlock_t lock;
};

bool fls_tm_chardev_enqueue(struct fls_tm_flow *tm_flow);
void fls_tm_chardev_shutdown(void);
int fls_tm_chardev_init(void);
