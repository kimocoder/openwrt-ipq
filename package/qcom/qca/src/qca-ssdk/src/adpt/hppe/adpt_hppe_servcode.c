/*
 * Copyright (c) 2016-2017, 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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


/**
 * @defgroup
 * @{
 */
#include "sw.h"
#include "hppe_servcode_reg.h"
#include "hppe_servcode.h"
#include "hppe_fdb_reg.h"
#include "hppe_fdb.h"
#include "adpt.h"
#include "adpt_hppe.h"
#if defined(APPE)
#include "adpt_appe_servcode.h"
#endif
#if defined(MPPE)
#include "adpt_mppe_servcode.h"
#endif

#define MAX_PHYSICAL_PORT 8

sw_error_t adpt_hppe_servcode_config_set(a_uint32_t dev_id, a_uint32_t servcode_index,
					fal_servcode_config_t *entry)
{
	union in_l2_service_tbl_u in_l2_service_tbl;
	union service_tbl_u service_tbl;
	union eg_service_tbl_u eg_service_tbl;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(entry);

	if (servcode_index >= IN_L2_SERVICE_TBL_MAX_ENTRY || entry->dest_port_id >= MAX_PHYSICAL_PORT)
		return SW_OUT_OF_RANGE;

	in_l2_service_tbl.bf.dst_port_id_valid = entry->dest_port_valid;
	in_l2_service_tbl.bf.dst_port_id = entry->dest_port_id;
	in_l2_service_tbl.bf.direction = entry->direction;
	in_l2_service_tbl.bf.bypass_bitmap = entry->bypass_bitmap[1];
	in_l2_service_tbl.bf.rx_cnt_en = (entry->bypass_bitmap[2] >> 1) & 0x1;
	in_l2_service_tbl.bf.tx_cnt_en = (entry->bypass_bitmap[2] >> 3) & 0x1;
#if defined(MRPPE)
	if(adpt_chip_type_get(dev_id) == CHIP_MRPPE)
		in_l2_service_tbl.bf.bypass_bitmap_ext = (entry->bypass_bitmap[1] >> 24) & 0xff;
#endif
#ifdef IN_PORTCONTROL
	SW_RTN_ON_ERROR(adpt_ppe_port_tdm_resource_set(dev_id, A_FALSE));
#endif
	SW_RTN_ON_ERROR(hppe_in_l2_service_tbl_set(dev_id, servcode_index, &in_l2_service_tbl));
#ifdef IN_PORTCONTROL
	SW_RTN_ON_ERROR(adpt_ppe_port_tdm_resource_set(dev_id, A_TRUE));
#endif

	service_tbl.bf.bypass_bitmap = entry->bypass_bitmap[0];
	service_tbl.bf.rx_counting_en = entry->bypass_bitmap[2] & 0x1;
	SW_RTN_ON_ERROR(hppe_service_tbl_set(dev_id, servcode_index, &service_tbl));

#if defined(MPPE)
	/*do not touch the athtag configurations*/
	SW_RTN_ON_ERROR(hppe_eg_service_tbl_get(dev_id, servcode_index, &eg_service_tbl));
	eg_service_tbl.bf.field_update_action &= ATHTAG_UPDATE;
	eg_service_tbl.bf.field_update_action |= entry->field_update_bitmap;
#else
	eg_service_tbl.bf.field_update_action = entry->field_update_bitmap;
#endif
	eg_service_tbl.bf.next_service_code = entry->next_service_code;
	eg_service_tbl.bf.hw_services = entry->hw_services;
	eg_service_tbl.bf.offset_sel = entry->offset_sel;
	eg_service_tbl.bf.tx_counting_en = (entry->bypass_bitmap[2] >> 2) & 0x1;
	SW_RTN_ON_ERROR(hppe_eg_service_tbl_set(dev_id, servcode_index, &eg_service_tbl));

#if defined(APPE)
	if(adpt_chip_type_get(dev_id) == CHIP_APPE ||
		adpt_chip_type_get(dev_id) == CHIP_MRPPE)
	{
		SW_RTN_ON_ERROR(adpt_appe_servcode_tl_config_set(dev_id, servcode_index, entry));
	}
#endif
	return SW_OK;
}

sw_error_t adpt_hppe_servcode_config_get(a_uint32_t dev_id, a_uint32_t servcode_index,
					fal_servcode_config_t *entry)
{
	union in_l2_service_tbl_u in_l2_service_tbl;
	union service_tbl_u service_tbl;
	union eg_service_tbl_u eg_service_tbl;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(entry);

	if (servcode_index >= IN_L2_SERVICE_TBL_MAX_ENTRY)
		return SW_OUT_OF_RANGE;

	SW_RTN_ON_ERROR(hppe_in_l2_service_tbl_get(dev_id, servcode_index, &in_l2_service_tbl));
	entry->dest_port_valid = in_l2_service_tbl.bf.dst_port_id_valid;
	entry->dest_port_id = in_l2_service_tbl.bf.dst_port_id;
	entry->direction = in_l2_service_tbl.bf.direction;
	entry->bypass_bitmap[1] = in_l2_service_tbl.bf.bypass_bitmap;
#if defined(MRPPE)
	if(adpt_chip_type_get(dev_id) == CHIP_MRPPE)
		entry->bypass_bitmap[1] |= (in_l2_service_tbl.bf.bypass_bitmap_ext & 0xff) << 24;
#endif
	entry->bypass_bitmap[2] |= in_l2_service_tbl.bf.rx_cnt_en << 1;
	entry->bypass_bitmap[2] |= in_l2_service_tbl.bf.tx_cnt_en << 3;

	SW_RTN_ON_ERROR(hppe_service_tbl_get(dev_id, servcode_index, &service_tbl));
	entry->bypass_bitmap[0] = service_tbl.bf.bypass_bitmap;
	entry->bypass_bitmap[2] |= service_tbl.bf.rx_counting_en;

	SW_RTN_ON_ERROR(hppe_eg_service_tbl_get(dev_id, servcode_index, &eg_service_tbl));
	entry->field_update_bitmap = eg_service_tbl.bf.field_update_action;
	entry->next_service_code = eg_service_tbl.bf.next_service_code;
	entry->hw_services = eg_service_tbl.bf.hw_services;
	entry->offset_sel = eg_service_tbl.bf.offset_sel;
	entry->bypass_bitmap[2] |= eg_service_tbl.bf.tx_counting_en << 2;

#if defined (APPE)
	if(adpt_chip_type_get(dev_id) == CHIP_APPE ||
		adpt_chip_type_get(dev_id) == CHIP_MRPPE)
	{
		SW_RTN_ON_ERROR(adpt_appe_servcode_tl_config_get(dev_id, servcode_index, entry));
	}
#endif
	return SW_OK;
}

sw_error_t adpt_hppe_servcode_loopcheck_en(a_uint32_t dev_id, a_bool_t enable)
{
	ADPT_DEV_ID_CHECK(dev_id);
#ifndef IN_FDB_MINI
	SW_RTN_ON_ERROR(hppe_l2_global_conf_service_code_loop_set(dev_id, enable));
#endif
	return SW_OK;
}

sw_error_t adpt_hppe_servcode_loopcheck_status_get(a_uint32_t dev_id, a_bool_t *enable)
{
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(enable);
#ifndef IN_FDB_MINI
	SW_RTN_ON_ERROR(hppe_l2_global_conf_service_code_loop_get(dev_id, enable));
#endif
	return SW_OK;
}

sw_error_t adpt_hppe_servcode_init(a_uint32_t dev_id)
{
	adpt_api_t *p_adpt_api = NULL;

	p_adpt_api = adpt_api_ptr_get(dev_id);

	if(p_adpt_api == NULL)
		return SW_FAIL;

	p_adpt_api->adpt_servcode_config_set = adpt_hppe_servcode_config_set;
	p_adpt_api->adpt_servcode_config_get = adpt_hppe_servcode_config_get;
	p_adpt_api->adpt_servcode_loopcheck_en = adpt_hppe_servcode_loopcheck_en;
	p_adpt_api->adpt_servcode_loopcheck_status_get = adpt_hppe_servcode_loopcheck_status_get;
#if defined(MPPE)
	if(adpt_ppe_type_get(dev_id) == MPPE_TYPE ||
		adpt_ppe_type_get(dev_id) == MRPPE_TYPE)
	{
		p_adpt_api->adpt_port_servcode_set = adpt_mppe_port_servcode_set;
		p_adpt_api->adpt_port_servcode_get = adpt_mppe_port_servcode_get;
		p_adpt_api->adpt_servcode_athtag_set = adpt_mppe_servcode_athtag_set;
		p_adpt_api->adpt_servcode_athtag_get = adpt_mppe_servcode_athtag_get;
	}
#endif

	return SW_OK;
}

/**
 * @}
 */


