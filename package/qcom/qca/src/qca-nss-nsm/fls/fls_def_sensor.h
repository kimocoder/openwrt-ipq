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

#ifndef __FLS_DEF_SENSOR_H
#define __FLS_DEF_SENSOR_H

#include <linux/types.h>
#include <linux/ktime.h>
#include <linux/spinlock.h>
#include "fls_sensor_manager.h"

#define FLS_DEF_SENSOR_MAX_SAMPLE_COUNT 10
#define FLS_DEF_SENSOR_WINDOWS 3
#define FLS_DEF_SENSOR_WINDOW_LG (FLS_DEF_SENSOR_WINDOWS - 1)
#define FLS_DEF_SENSOR_TOTAL_TIME (fls_def_sensor_window_sz[FLS_DEF_SENSOR_WINDOW_LG] * fls_def_sensor_sample_count)

extern uint32_t fls_def_sensor_window_sz[FLS_DEF_SENSOR_WINDOWS];
extern uint32_t fls_def_sensor_delay;
extern int32_t fls_def_sensor_max_events;
extern uint32_t fls_def_sensor_sample_count;
extern uint32_t fls_def_sensor_bytes;
extern uint32_t fls_def_sensor_ipat;
extern uint32_t fls_def_sensor_stop_forever;
extern uint32_t fls_def_sensor_pkts_hwm;
extern uint32_t fls_def_sensor_bytes_hwm;
extern uint32_t fls_def_sensor_burst;
extern uint32_t fls_def_sensor_burst_threshold[FLS_DEF_SENSOR_WINDOWS];
extern uint32_t fls_def_sensor_burst_short_intvl[FLS_DEF_SENSOR_WINDOWS];
extern uint32_t fls_def_sensor_burst_long_intvl[FLS_DEF_SENSOR_WINDOWS];
extern uint32_t fls_def_sensor_xl_sz_threshold;
extern uint32_t fls_def_sensor_xl_short;
extern uint32_t fls_def_sensor_xl_long;
extern uint32_t fls_def_sensor_xl_window;

struct fls_def_sensor_burst {
	bool active;
	uint32_t sz;
	ktime_t start;
	ktime_t last;
};

struct fls_def_sensor_window {
	bool open;
	uint32_t packets;
	uint32_t bytes;
	uint32_t bytes_min;
	uint32_t bytes_max;
	uint64_t delta_sum;
	uint64_t delta_min;
	uint64_t delta_max;
	uint32_t bursts;
	uint32_t burst_sz_sum;
	uint32_t burst_sz_min;
	uint32_t burst_sz_max;
	uint64_t burst_dur_sum;
	uint64_t burst_dur_min;
	uint64_t burst_dur_max;
	struct fls_def_sensor_burst burst_data;
};

struct fls_def_sensor_sample {
	ktime_t last_packet_time;
	ktime_t sample_start_time;
	struct fls_def_sensor_window window[FLS_DEF_SENSOR_WINDOWS];
};

struct fls_def_sensor_data {
	struct fls_def_sensor_sample samples[FLS_DEF_SENSOR_MAX_SAMPLE_COUNT];
	ktime_t first_packet_time;
	ktime_t event_start_time;
	struct fls_def_sensor_sample xl_sample;

	/* sendevent:
	 * true - send event when it is generated.
	 * false - not send the event to IFLI when it is generated.
	 * sendevent will be set to false
	 * when receive stop_cmd and if either below is true:
	 *	stop_forever == false
	 *	stop_forever == true and max_events == -1.
	 * sendevent will be reset to true when next XL window event generates.
	 */
	bool sendevent;

	uint32_t sample_index;
	uint32_t events;
};

bool fls_def_sensor_init(struct fls_sensor_manager *fsm);
uint8_t fls_def_sensor_packet_cb(void *app_data, struct fls_conn *conn, struct sk_buff *skb);
#endif
