/*
 **************************************************************************
 * Copyright (c) 2023-2024, Qualcomm Innovation Center, Inc. All rights reserved.
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

#define FLS_CHARDEV_EVENT_MAX 128
#define FLS_CHARDEV_EVENT_MASK (FLS_CHARDEV_EVENT_MAX - 1)

#include <linux/module.h>
#include <linux/ktime.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/version.h>
#include <linux/delay.h>

#include "fls_debug.h"
#include "fls_chardev.h"
#include "fls_conn.h"

DEFINE_SPINLOCK(fls_conn_lock);

struct fls_event_log {
	uint32_t read_index;
	uint32_t write_index;
	struct fls_event event_ring_buf[FLS_CHARDEV_EVENT_MAX];
	spinlock_t read_lock;
	spinlock_t write_lock;
};

static struct fls_chardev chardev;

static struct fls_event_log event_log;
static struct fls_event temp;
static struct sk_buff dummy_skb;

static int fls_chardev_fopen(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t fls_chardev_fread(struct file *file, char *buffer, size_t length, loff_t *offset)
{
	unsigned long irqflags;

	/*
	 * Copy full event structure, including all (up to 20) samples into *buffer.
	 */
	if (!buffer) {
		FLS_ERROR("Could not read data due to missing buffer.\n");
		return -EINVAL;
	}

	if (length < sizeof(struct fls_event)) {
		FLS_ERROR("Buffer too small to hold flow event data. %d < %d\n", length, sizeof(struct fls_event));
		return -EINVAL;
	}

	spin_lock_irqsave(&event_log.read_lock, irqflags);

	if (event_log.read_index == event_log.write_index) {
		spin_unlock_irqrestore(&event_log.read_lock, irqflags);
		FLS_ERROR("Event log is empty. read_index:%d, write_index:%d\n", event_log.read_index, event_log.write_index);
		return 0;
	}

	memcpy(&temp, &(event_log.event_ring_buf[event_log.read_index]), sizeof(temp));
	event_log.read_index = (event_log.read_index + 1) & FLS_CHARDEV_EVENT_MASK;

	spin_unlock_irqrestore(&event_log.read_lock, irqflags);

	if (copy_to_user(buffer, &temp, sizeof(temp))) {
		FLS_ERROR("Failed to write event to output buffer.\n");
		return -EIO;
	}

	return sizeof(struct fls_event);
}

static ssize_t fls_chardev_fwrite(struct file *file, const char *buffer, size_t length, loff_t *offset)
{
	int count;
	struct fls_cmdinfo packetinfo;
	struct fls_conn *conn;

	count = min(length, sizeof(struct fls_cmdinfo));
	if (copy_from_user((char*)&packetinfo, buffer, count)) {
		FLS_ERROR("copy from user failed.\n");
		return -EFAULT;
	}

	if(count < sizeof(packetinfo)) {
		FLS_ERROR("packet size not correct\n");
		return 0;
	}

	/* FLSP debug info */
	if (packetinfo.version == 4) {

		FLS_TRACE("Protocol: %pI4:%u-> %pI4:%u, size = %d bytes\n",
						packetinfo.src_ip,
						packetinfo.src_port,
						packetinfo.dst_ip,
						packetinfo.dst_port,
						packetinfo.data.fls_packetinfo.packet_size);
	} else {
		FLS_TRACE("Protocol: %pI6:%u-> %pI6:%u, size = %d bytes\n",
						packetinfo.src_ip,
						packetinfo.src_port,
						packetinfo.dst_ip,
						packetinfo.dst_port,
						packetinfo.data.fls_packetinfo.packet_size);

	}

	switch (packetinfo.cmd) {
		case FLS_CHARDEV_FLUSH:
			/* flush all external connections */
			FLS_TRACE("Flush external connections.\n");
			spin_lock(&fls_conn_lock);
			fls_conn_flush();
			spin_unlock(&fls_conn_lock);
			return count;

		case FLS_CHARDEV_EVENT:
			break;

		case FLS_CHARDEV_RESULT:
			FLS_TRACE("\nFLS: Receive stop command.\n");
			spin_lock(&fls_conn_lock);
			conn = fls_conn_lookup(packetinfo.version, packetinfo.protocol,
						packetinfo.src_ip,
						packetinfo.src_port,
						packetinfo.dst_ip,
						packetinfo.dst_port);
			if(conn) {
				conn->stats.isd.sendevent = false;
				if(conn->reverse)
					conn->reverse->stats.isd.sendevent = false;
				if (fls_def_sensor_max_events != -1 && fls_def_sensor_stop_forever)  {
					FLS_TRACE("Lookup succeed! Stop XL collection (FOREVER).");
					conn->flags &= ~SFE_FLS_CONNECTION_FLAG_DEF_ENABLE;
					if (conn->reverse) {
						conn->reverse->flags &= ~SFE_FLS_CONNECTION_FLAG_DEF_ENABLE;
					}
					spin_unlock(&fls_conn_lock);
					return count;
				}
				FLS_TRACE("Lookup succeed! Stop XL collection (For this epoch).");
			} else {
				FLS_TRACE("Lookup failed!\n");
			}
			spin_unlock(&fls_conn_lock);
			return count;
		default:
			FLS_ERROR("Unrecognized command %d.\n", packetinfo.cmd);
			return 0;
	}

	/*TODO: To check if IPv6 is supported in FLSP.*/
	spin_lock(&fls_conn_lock);
	conn = fls_conn_lookup(4, packetinfo.protocol,
						&packetinfo.src_ip[0],
						packetinfo.src_port,
						&packetinfo.dst_ip[0],
						packetinfo.dst_port);
	FLS_TRACE("Lookup finished\n");
	if(!conn) {
		FLS_TRACE("Creating external connection\n");
		conn = fls_conn_create_bidiflow(4, packetinfo.protocol,
						&packetinfo.src_ip[0],
						packetinfo.src_port,
						&packetinfo.dst_ip[0],
						packetinfo.dst_port,
						true, packetinfo.data.fls_packetinfo.timestamp_nsec);
		if(!conn) {
			FLS_ERROR("Cannot create new bidiflow\n");
			spin_unlock(&fls_conn_lock);
			return 0;
		}
	} else {
		FLS_TRACE("Found connection %px, updating last packetarrival..\n", conn);
		conn->last_ts = ktime_set(packetinfo.data.fls_packetinfo.timestamp_sec, packetinfo.data.fls_packetinfo.timestamp_nsec);
		conn->reverse->last_ts = conn->last_ts;
	}
	FLS_TRACE("Receive skb for conn=%px sec = %llu nsec = %llu\n", conn,  packetinfo.data.fls_packetinfo.timestamp_sec, packetinfo.data.fls_packetinfo.timestamp_nsec);

	dummy_skb.len = packetinfo.data.fls_packetinfo.packet_size;
	dummy_skb.tstamp = ktime_set(packetinfo.data.fls_packetinfo.timestamp_sec, packetinfo.data.fls_packetinfo.timestamp_nsec);
	fls_def_sensor_packet_cb(NULL, conn, &dummy_skb);
	spin_unlock(&fls_conn_lock);

	//limit the speed of creating events.	
	if (((event_log.write_index - event_log.read_index + FLS_CHARDEV_EVENT_MASK) & FLS_CHARDEV_EVENT_MASK) > FLS_CHARDEV_EVENTS_LIMIT){
		while(event_log.write_index != event_log.read_index)
				msleep(1);
	}

	return count;
}



static int fls_chardev_fmmap(struct file *file, struct vm_area_struct *vma)
{
	return -EINVAL;
}

static int fls_chardev_frelease(struct inode *inode, struct file *file)
{
	return 0;
}

static unsigned int fls_chardev_poll(struct file *file, struct poll_table_struct *wait)
{
	unsigned int ret = 0;
	unsigned long irqflags;

	poll_wait(file, &chardev.readq, wait);

	spin_lock_irqsave(&event_log.read_lock, irqflags);
	if (event_log.read_index != event_log.write_index) {
		ret = POLLIN | POLLRDNORM;
	}
	spin_unlock_irqrestore(&event_log.read_lock, irqflags);

	return ret;
}

static const struct file_operations fls_chardev_fops = {
	.owner = THIS_MODULE,
	.open = fls_chardev_fopen,
	.mmap = fls_chardev_fmmap,
	.llseek = NULL,
	.read = fls_chardev_fread,
	.write = fls_chardev_fwrite,
	.release = fls_chardev_frelease,
	.poll = fls_chardev_poll
};

bool fls_chardev_enqueue(struct fls_event *event)
{
	unsigned long irqflags;
	uint32_t write_index;

	FLS_INFO("FID: enqueue flow event.");
	fls_debug_print_event_info(event);

	spin_lock_irqsave(&event_log.write_lock, irqflags);
	if (((event_log.write_index + 1) & FLS_CHARDEV_EVENT_MASK) == event_log.read_index) {
		spin_unlock_irqrestore(&event_log.write_lock, irqflags);
		return false;
	}

	write_index = event_log.write_index;
	event_log.event_ring_buf[write_index] = *event;
	event_log.write_index = (write_index + 1) & FLS_CHARDEV_EVENT_MASK;
	spin_unlock_irqrestore(&event_log.write_lock, irqflags);

	FLS_INFO("Enqeued flow event at index [%u]", write_index);

	if (waitqueue_active(&chardev.readq)) {
		wake_up_interruptible(&chardev.readq);
	}

	return true;
}

void fls_chardev_shutdown(void)
{
	cdev_del(&chardev.cdev);
	device_destroy(chardev.cl, chardev.devid);
	class_destroy(chardev.cl);
	unregister_chrdev_region(chardev.devid, 1);
}

int fls_chardev_init(void)
{
	int ret;
	spin_lock_init(&event_log.read_lock);
	spin_lock_init(&event_log.write_lock);
	init_waitqueue_head(&chardev.readq);

	ret = alloc_chrdev_region(&(chardev.devid), 0, 1, FLS_CHARDEV_NAME);
	if (ret) {
		FLS_ERROR("Failed to allocate device id: %d\n", ret);
		return ret;
	}

	cdev_init(&chardev.cdev, &fls_chardev_fops);
	chardev.cdev.owner = THIS_MODULE;

	ret = cdev_add(&chardev.cdev, chardev.devid, 1);
	if (ret) {
		FLS_ERROR("Failed to add fls device: %d\n", ret);
		unregister_chrdev_region(chardev.devid, 1);
		return ret;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0))
	chardev.cl = class_create(THIS_MODULE, FLS_CHARDEV_NAME);
#else
	chardev.cl = class_create(FLS_CHARDEV_NAME);
#endif
	device_create(chardev.cl, NULL, chardev.devid, NULL, FLS_CHARDEV_NAME);

	return 0;
}
