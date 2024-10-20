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

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/version.h>

#include "fls_tm.h"
#include "fls_chardev.h"
#include "fls_tm_chardev.h"
#include "fls_debug.h"

static struct fls_chardev chardev;
static struct fls_msg_log msg_log;
static struct fls_tm_flow temp[FLS_TM_CHARDEV_MSG_MAX];

static ssize_t fls_tm_chardev_fread(struct file *file, char *buffer, size_t length, loff_t *offset)
{
	unsigned long irqflags;
	uint32_t ret;

	/*
	 * Copy full msg structure into buffer.
	 */
	if (!buffer) {
		FLS_ERROR("Could not read data due to missing buffer.\n");
		return -EINVAL;
	}
	spin_lock_irqsave(&msg_log.lock, irqflags);

	if (msg_log.write_index == 0) {
		spin_unlock_irqrestore(&msg_log.lock, irqflags);
		FLS_ERROR("msg log is empty. write_index:%d\n", msg_log.write_index);
		return 0;
	}

	memcpy(&temp, &msg_log.flow_ring_buf, sizeof(msg_log.flow_ring_buf));

	memset(msg_log.flow_ring_buf, 0, sizeof(msg_log.flow_ring_buf));
	ret = msg_log.write_index;
	msg_log.write_index = 0;
	spin_unlock_irqrestore(&msg_log.lock, irqflags);

	if (copy_to_user(buffer, &temp, sizeof(temp))) {
		FLS_ERROR("Failed to write tm_msg to output buffer.\n");
		return -EIO;
	}

	return (ret);
}

static unsigned int fls_tm_chardev_poll(struct file *file, struct poll_table_struct *wait)
{
	unsigned int ret = 0;
	unsigned long irqflags;

	poll_wait(file, &chardev.readq, wait);

	spin_lock_irqsave(&msg_log.lock, irqflags);
	if (msg_log.write_index != 0) {
		ret = POLLIN | POLLRDNORM;
	}
	spin_unlock_irqrestore(&msg_log.lock, irqflags);

	return ret;
}

static const struct file_operations fls_tm_chardev_fops = {
	.owner = THIS_MODULE,
	.llseek = NULL,
	.read = fls_tm_chardev_fread,
	.poll = fls_tm_chardev_poll
};

bool fls_tm_chardev_enqueue(struct fls_tm_flow *tm_flow)
{
	unsigned long irqflags;
	uint32_t write_index;
	bool ret = true;

	FLS_TRACE("FLS_TM: enqueue tm ");

	spin_lock_irqsave(&msg_log.lock, irqflags);
	if (((msg_log.write_index + 1) & FLS_TM_CHARDEV_MSG_MASK) == 0) {
		/*
		 * Reset the ring buffer, this case should only occur if buffer is full before tm is loaded
		 * tracking old data will result in invalid heavy hitters
		 */
		memset(msg_log.flow_ring_buf, 0, sizeof(msg_log.flow_ring_buf));
		msg_log.write_index = 0;

		tm_flow->flags |= FLS_TM_FLAG_RESET;
		ret = false;
	}

	write_index = msg_log.write_index;
	msg_log.flow_ring_buf[write_index] = *tm_flow;
	msg_log.write_index = (write_index + 1) & FLS_TM_CHARDEV_MSG_MASK;
	spin_unlock_irqrestore(&msg_log.lock, irqflags);

	FLS_TRACE("Enqeued tm msg at index [%u]", write_index);

	if (waitqueue_active(&chardev.readq)) {
		wake_up_interruptible(&chardev.readq);
	}

	return ret;
}

void fls_tm_chardev_shutdown(void)
{
	cdev_del(&chardev.cdev);
	device_destroy(chardev.cl, chardev.devid);
	class_destroy(chardev.cl);
	unregister_chrdev_region(chardev.devid, 1);
}

int fls_tm_chardev_init(void)
{
	int ret;

	spin_lock_init(&msg_log.lock);
	init_waitqueue_head(&chardev.readq);

	ret = alloc_chrdev_region(&(chardev.devid), 0, 1, FLS_TM_CHARDEV_NAME);
	if (ret) {
		FLS_ERROR("Failed to allocate device id: %d\n", ret);
		return ret;
	}

	cdev_init(&chardev.cdev, &fls_tm_chardev_fops);
	chardev.cdev.owner = THIS_MODULE;

	ret = cdev_add(&chardev.cdev, chardev.devid, 1);
	if (ret) {
		FLS_ERROR("Failed to add fls device: %d\n", ret);
		unregister_chrdev_region(chardev.devid, 1);
		return ret;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0))
	chardev.cl = class_create(THIS_MODULE, FLS_TM_CHARDEV_NAME);
#else
	chardev.cl = class_create(FLS_TM_CHARDEV_NAME);
#endif
	device_create(chardev.cl, NULL, chardev.devid, NULL, FLS_TM_CHARDEV_NAME);

	memset(msg_log.flow_ring_buf, 0, sizeof(msg_log.flow_ring_buf));

	return 0;
}
