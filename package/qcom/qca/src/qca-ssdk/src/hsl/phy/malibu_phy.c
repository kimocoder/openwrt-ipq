/*
 * Copyright (c) 2015-2019, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include "sw.h"
#include "fal_port_ctrl.h"
#include "hsl_api.h"
#include "hsl.h"
#include "malibu_phy.h"
#include "hsl_phy.h"
#include "qcaphy_common.h"
#include "ssdk_plat.h"

static a_uint32_t first_phy_addr = MAX_PHY_ADDR;
static a_uint32_t combo_phy_addr = MAX_PHY_ADDR;
#define COMBO_PHY_ID combo_phy_addr

/******************************************************************************
*
*  phy4 medium is fiber 100fx
*
*  get phy4 medium is 100fx
*/
static a_bool_t __medium_is_fiber_100fx(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	a_uint16_t phy_data = 0;

	phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, MALIBU_PHY_SGMII_STATUS);

	if (phy_data & MALIBU_PHY4_AUTO_FX100_SELECT) {
		return A_TRUE;
	}
	/* Link down */
	if ((!(phy_data & MALIBU_PHY4_AUTO_COPPER_SELECT)) &&
		(!(phy_data & MALIBU_PHY4_AUTO_BX1000_SELECT)) &&
		(!(phy_data & MALIBU_PHY4_AUTO_SGMII_SELECT))) {

		phy_data =
			hsl_phy_mii_reg_read(dev_id, phy_addr, MALIBU_PHY_CHIP_CONFIG);
		if ((phy_data & MALIBU_PHY4_PREFER_FIBER)
			&& (!(phy_data & MALIBU_PHY4_FIBER_MODE_1000BX))) {
			return A_TRUE;
		}
	}

	return A_FALSE;
}

/******************************************************************************
*
*  phy4 prfer medium
*
*  get phy4 prefer medum, fiber or copper;
*/
static fal_port_medium_t __phy_prefer_medium_get(a_uint32_t dev_id,
	a_uint32_t phy_addr)
{
	a_uint16_t phy_medium = 0;

	phy_medium =
		hsl_phy_mii_reg_read(dev_id, phy_addr, MALIBU_PHY_CHIP_CONFIG);

	return ((phy_medium & MALIBU_PHY4_PREFER_FIBER) ?
		PHY_MEDIUM_FIBER : PHY_MEDIUM_COPPER);
}

/******************************************************************************
*
*  phy4 activer medium
*
*  get phy4 current active medium, fiber or copper;
*/
static fal_port_medium_t __phy_active_medium_get(a_uint32_t dev_id,
	a_uint32_t phy_addr)
{
	a_uint16_t phy_data = 0;
	a_uint16_t phy_mode = 0;

	phy_mode = hsl_phy_mii_reg_read(dev_id, phy_addr, MALIBU_PHY_CHIP_CONFIG);
	phy_mode &= 0x000f;

	if (phy_mode == MALIBU_PHY_PSGMII_AMDET) {
		phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, MALIBU_PHY_SGMII_STATUS);
		if ((phy_data & MALIBU_PHY4_AUTO_COPPER_SELECT)) {
			return PHY_MEDIUM_COPPER;
		} else if ((phy_data & MALIBU_PHY4_AUTO_BX1000_SELECT)) {
			return PHY_MEDIUM_FIBER;	/*PHY_MEDIUM_FIBER_BX1000 */
		} else if ((phy_data & MALIBU_PHY4_AUTO_FX100_SELECT)) {
			return PHY_MEDIUM_FIBER;	/*PHY_MEDIUM_FIBER_FX100 */
		}
		/* link down */
		return __phy_prefer_medium_get(dev_id, phy_addr);
	} else if ((phy_mode == MALIBU_PHY_PSGMII_BASET) ||(phy_mode == MALIBU_PHY_SGMII_BASET) ) {
		return PHY_MEDIUM_COPPER;
	} else if ((phy_mode == MALIBU_PHY_PSGMII_BX1000) ||(phy_mode == MALIBU_PHY_PSGMII_FX100)) {
		return PHY_MEDIUM_FIBER;
	} else {
		return PHY_MEDIUM_COPPER;
	}
}

/******************************************************************************
*
*  phy4 copper page or fiber page select
*
*  set phy4 copper or fiber page
*/

static sw_error_t __phy_reg_pages_sel(a_uint32_t dev_id, a_uint32_t phy_addr,
	malibu_phy_reg_pages_t phy_reg_pages)
{
	a_uint16_t reg_pages = 0;
	reg_pages = hsl_phy_mii_reg_read(dev_id, phy_addr, MALIBU_PHY_CHIP_CONFIG);

	if (phy_reg_pages == MALIBU_PHY_COPPER_PAGES) {
		reg_pages |= 0x8000;
	} else if (phy_reg_pages == MALIBU_PHY_SGBX_PAGES) {
		reg_pages &= ~0x8000;
	} else
		return SW_BAD_PARAM;

	return hsl_phy_mii_reg_write(dev_id, phy_addr, MALIBU_PHY_CHIP_CONFIG,
		reg_pages);
}

/******************************************************************************
*
*  phy4 reg pages selection by active medium
*
*  phy4 reg pages selection
*/
static sw_error_t __phy_reg_pages_sel_by_active_medium(a_uint32_t dev_id,
	a_uint32_t phy_addr)
{
	fal_port_medium_t phy_medium = 0;
	malibu_phy_reg_pages_t reg_pages = 0;

	phy_medium = __phy_active_medium_get(dev_id, phy_addr);
	if (phy_medium == PHY_MEDIUM_FIBER) {
		reg_pages = MALIBU_PHY_SGBX_PAGES;
	} else if (phy_medium == PHY_MEDIUM_COPPER) {
		reg_pages = MALIBU_PHY_COPPER_PAGES;
	} else
		return SW_BAD_VALUE;

	return __phy_reg_pages_sel(dev_id, phy_addr, reg_pages);
}

/******************************************************************************
*
* malibu_phy_get_speed - Determines the speed of phy ports associated with the
* specified device.
*/

sw_error_t
malibu_phy_get_speed(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_speed_t * speed)
{
	if (phy_addr == COMBO_PHY_ID) {
		__phy_reg_pages_sel_by_active_medium(dev_id, phy_addr);
	}
	return qcaphy_get_speed(dev_id, phy_addr, speed);
}

/******************************************************************************
*
* malibu_phy_get_duplex - Determines the speed of phy ports associated with the
* specified device.
*/
sw_error_t
malibu_phy_get_duplex(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_duplex_t * duplex)
{
	if (phy_addr == COMBO_PHY_ID) {
		__phy_reg_pages_sel_by_active_medium(dev_id, phy_addr);
	}
	return qcaphy_get_duplex(dev_id, phy_addr, duplex);
}

#ifndef IN_PORTCONTROL_MINI
/******************************************************************************
*
* malibu_phy_reset - reset the phy
*
* reset the phy
*/
sw_error_t malibu_phy_reset(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	if (phy_addr == COMBO_PHY_ID)
		__phy_reg_pages_sel_by_active_medium(dev_id, phy_addr);
	return qcaphy_sw_reset(dev_id, phy_addr);
}
#endif
/******************************************************************************
*
* malibu_phy_set combo medium type
*
* set combo medium fiber or copper
*/
sw_error_t
malibu_phy_set_combo_prefer_medium(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_medium_t phy_medium)
{
	a_uint16_t phy_data = 0;

	if (phy_addr != COMBO_PHY_ID)
		return SW_NOT_SUPPORTED;

	if (phy_medium == PHY_MEDIUM_FIBER) {
		phy_data |= MALIBU_PHY4_PREFER_FIBER;
	} else if (phy_medium == PHY_MEDIUM_COPPER) {
		phy_data |= MALIBU_PHY4_PREFER_COPPER;
	} else {
		return SW_BAD_PARAM;
	}
	return hsl_phy_modify_mii(dev_id, phy_addr, MALIBU_PHY_CHIP_CONFIG,
		MALIBU_PHY4_PREFER_FIBER | MALIBU_PHY4_PREFER_COPPER, phy_data);
}

/******************************************************************************
*
* malibu_phy_get combo medium type
*
* get combo medium fiber or copper
*/
sw_error_t
malibu_phy_get_combo_prefer_medium(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_medium_t * phy_medium)
{
	a_uint16_t phy_data = 0;

	if (phy_addr != COMBO_PHY_ID)
		return SW_NOT_SUPPORTED;

	phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, MALIBU_PHY_CHIP_CONFIG);

	*phy_medium =
		(phy_data & MALIBU_PHY4_PREFER_FIBER) ? PHY_MEDIUM_FIBER :
		PHY_MEDIUM_COPPER;

	return SW_OK;
}

/******************************************************************************
*
* malibu_phy_get current combo medium type copper or fiber
*
* get current combo medium type
*/
sw_error_t
malibu_phy_get_combo_current_medium_type(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_medium_t * phy_medium)
{

	if (phy_addr != COMBO_PHY_ID)
		return SW_NOT_SUPPORTED;

	*phy_medium = __phy_active_medium_get(dev_id, phy_addr);

	return SW_OK;
}

/******************************************************************************
*
* malibu_phy_set fiber mode 1000bx or 100fx
*
* set combo fbier mode
*/
sw_error_t
malibu_phy_set_combo_fiber_mode(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_fiber_mode_t fiber_mode)
{
	a_uint16_t phy_data = 0;

	if (phy_addr != COMBO_PHY_ID)
		return SW_NOT_SUPPORTED;

	if (fiber_mode == PHY_FIBER_1000BX) {
		phy_data |= MALIBU_PHY4_FIBER_MODE_1000BX;
	} else if (fiber_mode == PHY_FIBER_100FX) {
		phy_data &= ~MALIBU_PHY4_FIBER_MODE_1000BX;
	} else {
		return SW_BAD_PARAM;
	}

	return hsl_phy_modify_mii(dev_id, phy_addr, MALIBU_PHY_CHIP_CONFIG,
		MALIBU_PHY4_FIBER_MODE_1000BX, phy_data);
}

/******************************************************************************
*
* malibu_phy_get fiber mode 1000bx or 100fx
*
* get combo fbier mode
*/
sw_error_t
malibu_phy_get_combo_fiber_mode(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_fiber_mode_t * fiber_mode)
{
	a_uint16_t phy_data;
	if (phy_addr != COMBO_PHY_ID)
		return SW_NOT_SUPPORTED;

	phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, MALIBU_PHY_CHIP_CONFIG);

	*fiber_mode =
		(phy_data & MALIBU_PHY4_FIBER_MODE_1000BX) ? PHY_FIBER_1000BX :
		PHY_FIBER_100FX;

	return SW_OK;
}
#if 0
/******************************************************************************
*
* malibu_phy_reset_done - reset the phy
*
* reset the phy
*/
a_bool_t malibu_phy_reset_done(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	a_uint16_t phy_data;
	a_uint16_t ii = 200;

	if (phy_addr == COMBO_PHY_ID)
		__phy_reg_pages_sel_by_active_medium(dev_id, phy_addr);

	do {
		phy_data =
			hsl_phy_mii_reg_read(dev_id, phy_addr, MALIBU_PHY_CONTROL);
		aos_mdelay(10);
	}
	while ((!MALIBU_RESET_DONE(phy_data)) && --ii);

	if (ii == 0)
		return A_FALSE;

	return A_TRUE;
}

/******************************************************************************
*
* malibu_autoneg_done
*
* malibu_autoneg_done
*/
a_bool_t malibu_autoneg_done(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	a_uint16_t phy_data;
	a_uint16_t ii = 200;

	if (phy_addr == COMBO_PHY_ID)
		__phy_reg_pages_sel_by_active_medium(dev_id, phy_addr);

	do {
		phy_data =
			hsl_phy_mii_reg_read(dev_id, phy_addr, MALIBU_PHY_STATUS);
		aos_mdelay(10);
	}
	while ((!MALIBU_AUTONEG_DONE(phy_data)) && --ii);

	if (ii == 0)
		return A_FALSE;

	return A_TRUE;
}

/******************************************************************************
*
* malibu_phy_Speed_Duplex_Resolved
 - reset the phy
*
* reset the phy
*/
a_bool_t malibu_phy_speed_duplex_resolved(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	a_uint16_t phy_data;
	a_uint16_t ii = 200;

	if (phy_addr == COMBO_PHY_ID)
		__phy_reg_pages_sel_by_active_medium(dev_id, phy_addr);

	do {
		phy_data =
			hsl_phy_mii_reg_read(dev_id, phy_addr, MALIBU_PHY_SPEC_STATUS);
		aos_mdelay(10);
	}
	while ((!MALIBU_SPEED_DUPLEX_RESOVLED(phy_data)) && --ii);

	if (ii == 0)
		return A_FALSE;

	return A_TRUE;
}
#endif
/******************************************************************************
*
* malibu_phy_off - power off the phy 
*
* Power off the phy
*/
sw_error_t malibu_phy_poweroff(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	sw_error_t rv = SW_OK;

	if (phy_addr == COMBO_PHY_ID) {
		__phy_reg_pages_sel(dev_id, phy_addr, MALIBU_PHY_SGBX_PAGES);
		rv = qcaphy_poweroff(dev_id, phy_addr);
		PHY_RTN_ON_ERROR(rv);
		__phy_reg_pages_sel(dev_id, phy_addr, MALIBU_PHY_COPPER_PAGES);
		rv = qcaphy_poweroff(dev_id, phy_addr);
		PHY_RTN_ON_ERROR(rv);
	} else {
		rv = qcaphy_poweroff(dev_id, phy_addr);
		PHY_RTN_ON_ERROR(rv);
	}

	return SW_OK;
}

/******************************************************************************
*
* malibu_phy_on - power on the phy 
*
* Power on the phy
*/
sw_error_t malibu_phy_poweron(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	sw_error_t rv = SW_OK;

	if (phy_addr == COMBO_PHY_ID) {
		__phy_reg_pages_sel(dev_id, phy_addr, MALIBU_PHY_SGBX_PAGES);
		rv = qcaphy_poweron(dev_id, phy_addr);
		PHY_RTN_ON_ERROR(rv);
		__phy_reg_pages_sel(dev_id, phy_addr, MALIBU_PHY_COPPER_PAGES);
		rv = qcaphy_poweron(dev_id, phy_addr);
		PHY_RTN_ON_ERROR(rv);
	} else {
		rv = qcaphy_poweron(dev_id, phy_addr);
		PHY_RTN_ON_ERROR(rv);
	}

	return SW_OK;
}
/******************************************************************************
*
* malibu_phy_status - test to see if the specified phy link is alive
*
* RETURNS:
*    A_TRUE  --> link is alive
*    A_FALSE --> link is down
*/
a_bool_t malibu_phy_get_link_status(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	if (phy_addr == COMBO_PHY_ID)
		__phy_reg_pages_sel_by_active_medium(dev_id, phy_addr);

	return qcaphy_get_link_status(dev_id, phy_addr);
}

/******************************************************************************
*
* malibu_set_autoneg_adv - set the phy autoneg Advertisement
*
*/
sw_error_t
malibu_phy_set_autoneg_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t autoneg)
{
	a_uint16_t phy_data = 0;

	if (phy_addr == COMBO_PHY_ID) {
		if (__medium_is_fiber_100fx(dev_id, phy_addr))
			return SW_NOT_SUPPORTED;

		if (PHY_MEDIUM_COPPER ==
			__phy_active_medium_get(dev_id, phy_addr)) {
				__phy_reg_pages_sel(dev_id, phy_addr, MALIBU_PHY_COPPER_PAGES);
		} else {
			__phy_reg_pages_sel(dev_id, phy_addr, MALIBU_PHY_SGBX_PAGES);
			phy_data &= ~MALIBU_BX_ADVERTISE_ALL;

			if (autoneg & FAL_PHY_ADV_1000BX_FD) {
				phy_data |= MALIBU_BX_ADVERTISE_1000FULL;
			}
			if (autoneg & FAL_PHY_ADV_1000BX_HD) {
				phy_data |= MALIBU_BX_ADVERTISE_1000HALF;
			}
			if (autoneg & FAL_PHY_ADV_PAUSE) {
				phy_data |= MALIBU_BX_ADVERTISE_PAUSE;
			}
			if (autoneg & FAL_PHY_ADV_ASY_PAUSE) {
				phy_data |= MALIBU_BX_ADVERTISE_ASYM_PAUSE;
			}
			return hsl_phy_modify_mii(dev_id, phy_addr, MALIBU_AUTONEG_ADVERT,
				MALIBU_BX_ADVERTISE_ALL, phy_data);
		}
	}

	return qcaphy_set_autoneg_adv(dev_id, phy_addr, autoneg);
}

/******************************************************************************
*
* malibu_get_autoneg_adv - get the phy autoneg Advertisement
*
*/
sw_error_t
malibu_phy_get_autoneg_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t * autoneg)
{
	a_uint16_t phy_data = 0;

	*autoneg = 0;
	if (phy_addr == COMBO_PHY_ID) {
		if (__medium_is_fiber_100fx(dev_id, phy_addr))
			return SW_NOT_SUPPORTED;

		if (PHY_MEDIUM_COPPER ==
			__phy_active_medium_get(dev_id, phy_addr)) {
			__phy_reg_pages_sel(dev_id, phy_addr, MALIBU_PHY_COPPER_PAGES);
		} else {
			__phy_reg_pages_sel(dev_id, phy_addr, MALIBU_PHY_SGBX_PAGES);
			phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr,
				MALIBU_AUTONEG_ADVERT);
			if (phy_data & MALIBU_BX_ADVERTISE_PAUSE)
				*autoneg |= FAL_PHY_ADV_PAUSE;

			if (phy_data & MALIBU_BX_ADVERTISE_ASYM_PAUSE)
				*autoneg |= FAL_PHY_ADV_ASY_PAUSE;

			if (phy_data & MALIBU_BX_ADVERTISE_1000HALF)
				*autoneg |= FAL_PHY_ADV_1000BX_HD;

			if (phy_data & MALIBU_BX_ADVERTISE_1000FULL)
				*autoneg |= FAL_PHY_ADV_1000BX_FD;
			return SW_OK;
		}
	}

	return qcaphy_get_autoneg_adv(dev_id, phy_addr, autoneg);
}

/******************************************************************************
*
* malibu_phy_enable_autonego 
*
* Power off the phy
*/
a_bool_t malibu_phy_autoneg_status(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	if (phy_addr == COMBO_PHY_ID) {
		if (__medium_is_fiber_100fx(dev_id, phy_addr))
			return A_FALSE;
		__phy_reg_pages_sel_by_active_medium(dev_id, phy_addr);
	}

	return qcaphy_autoneg_status(dev_id, phy_addr);
}

/******************************************************************************
*
* malibu_restart_autoneg - restart the phy autoneg
*
*/
sw_error_t malibu_phy_restart_autoneg(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	if (phy_addr == COMBO_PHY_ID) {
		if (__medium_is_fiber_100fx(dev_id, phy_addr))
			return SW_NOT_SUPPORTED;
		__phy_reg_pages_sel_by_active_medium(dev_id, phy_addr);
	}

	return qcaphy_autoneg_restart(dev_id, phy_addr);
}

/******************************************************************************
*
* malibu_phy_enable_autonego 
*
*/
sw_error_t malibu_phy_enable_autoneg(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	if (phy_addr == COMBO_PHY_ID) {
		if (__medium_is_fiber_100fx(dev_id, phy_addr))
			return SW_NOT_SUPPORTED;
		__phy_reg_pages_sel_by_active_medium(dev_id, phy_addr);
	}

	return qcaphy_autoneg_enable(dev_id, phy_addr);
}

/******************************************************************************
*
* malibu_phy_set_speed - Determines the speed of phy ports associated with the
* specified device.
*/
sw_error_t
malibu_phy_set_speed(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_speed_t speed)
{
	if (phy_addr == COMBO_PHY_ID) {
		if (PHY_MEDIUM_COPPER !=
			__phy_active_medium_get(dev_id, phy_addr))
			return SW_NOT_SUPPORTED;
		__phy_reg_pages_sel(dev_id, phy_addr, MALIBU_PHY_COPPER_PAGES);
	}

	return qcaphy_set_speed(dev_id, phy_addr, speed);
}

/******************************************************************************
*
* malibu_phy_set_duplex - Determines the speed of phy ports associated with the
* specified device.
*/
sw_error_t
malibu_phy_set_duplex(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_duplex_t duplex)
{
	a_uint16_t phy_data = 0;
	a_uint32_t mask = 0;

	if (phy_addr == COMBO_PHY_ID) {
		if (PHY_MEDIUM_COPPER !=
			__phy_active_medium_get(dev_id, phy_addr)) {
				__phy_reg_pages_sel(dev_id, phy_addr, MALIBU_PHY_SGBX_PAGES);
			mask |= MALIBU_CTRL_FULL_DUPLEX;
			if (__medium_is_fiber_100fx(dev_id, phy_addr)) {
				if (duplex == FAL_FULL_DUPLEX) {
					phy_data |= MALIBU_CTRL_FULL_DUPLEX;
				} else if (duplex == FAL_HALF_DUPLEX) {
					phy_data &= ~MALIBU_CTRL_FULL_DUPLEX;
				} else {
					return SW_BAD_PARAM;
				}
			} else {
					mask |= MALIBU_CTRL_AUTONEGOTIATION_ENABLE;
					if (duplex == FAL_FULL_DUPLEX) {
					phy_data |= MALIBU_CTRL_FULL_DUPLEX;
				} else if (duplex == FAL_HALF_DUPLEX) {
					phy_data &= ~MALIBU_CTRL_FULL_DUPLEX;
				} else {
					return SW_BAD_PARAM;
				}
			}
			return hsl_phy_modify_mii(dev_id, phy_addr, MALIBU_PHY_CONTROL,
				mask, phy_data);
		}
		__phy_reg_pages_sel(dev_id, phy_addr, MALIBU_PHY_COPPER_PAGES);
	}

	return qcaphy_set_duplex(dev_id, phy_addr, duplex);
}

/******************************************************************************
*
* malibu_serdes_reset - malibu psgmii serdes reset
*
* reset serdes
*/
sw_error_t
malibu_phy_serdes_reset(a_uint32_t dev_id)
{

	hsl_phy_mii_reg_write(dev_id, first_phy_addr + MALIBU_PHY_PSGMII_ADDR_INC,
		MALIBU_MODE_RESET_REG, MALIBU_MODE_CHANAGE_RESET);
	mdelay(100);
	hsl_phy_mii_reg_write(dev_id, first_phy_addr + MALIBU_PHY_PSGMII_ADDR_INC,
		MALIBU_MODE_RESET_REG, MALIBU_MODE_RESET_DEFAULT_VALUE);

	return SW_OK;
}

/******************************************************************************
*
* malibu_phy_interface mode set
*
* set malibu phy interface mode
*/
sw_error_t
malibu_phy_interface_set_mode(a_uint32_t dev_id, a_uint32_t phy_addr, fal_port_interface_mode_t interface_mode)
{
	a_uint16_t phy_data = 0;
	static fal_port_interface_mode_t phy_mode = PORT_INTERFACE_MODE_MAX;

	if ((phy_addr < first_phy_addr) ||
		(phy_addr > (first_phy_addr + MALIBU_PHY_MAX_ADDR_INC)))
		return SW_NOT_SUPPORTED;
	/*if interface_mode have been configured, then no need to configure again*/
	if(phy_mode == interface_mode)
		return SW_OK;

	if (interface_mode == PHY_PSGMII_BASET) {
		phy_data |= MALIBU_PHY_PSGMII_BASET;
	} else if (interface_mode == PHY_PSGMII_BX1000) {
		phy_data |= MALIBU_PHY_PSGMII_BX1000;
	} else if (interface_mode == PHY_PSGMII_FX100) {
		phy_data |= MALIBU_PHY_PSGMII_FX100;
	} else if (interface_mode == PHY_PSGMII_AMDET) {
	       phy_data |= MALIBU_PHY_PSGMII_AMDET;
	} else if (interface_mode == PHY_SGMII_BASET ||
		interface_mode == PORT_QSGMII) {
	       phy_data |= MALIBU_PHY_SGMII_BASET;
	} else if (interface_mode == PHY_PSGMII_FIBER) {
		phy_data |= MALIBU_PHY_PSGMII_AMDET;
	} else {
		return SW_BAD_PARAM;
	}

	hsl_phy_modify_mii(dev_id,
		first_phy_addr + MALIBU_PHY_MAX_ADDR_INC, MALIBU_PHY_CHIP_CONFIG,
		BITS(0, 4), phy_data);

	/* reset operation */
	malibu_phy_serdes_reset(dev_id);

	if (interface_mode == PHY_PSGMII_FIBER) {
		hsl_phy_mii_reg_write(dev_id, first_phy_addr + MALIBU_PHY_MAX_ADDR_INC,
			MALIBU_PHY_CHIP_CONFIG, MALIBU_MODECTRL_DFLT);
		hsl_phy_mii_reg_write(dev_id, first_phy_addr + MALIBU_PHY_MAX_ADDR_INC,
			MALIBU_PHY_CONTROL, MALIBU_MIICTRL_DFLT);
		hsl_phy_phydev_autoneg_update(dev_id,
			first_phy_addr + MALIBU_PHY_MAX_ADDR_INC, A_FALSE, 0);
	}
	phy_mode = interface_mode;
	SSDK_DEBUG("malibu phy is configured as phy_mode:0x%x\n", phy_mode);

	return SW_OK;
}

/******************************************************************************
*
* malibu_phy_interface mode get
*
* get malibu phy interface mode
*/
sw_error_t
malibu_phy_interface_get_mode(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_interface_mode_t *interface_mode)
{
	a_uint16_t phy_data;
	a_uint16_t copper_mode;

	if ((phy_addr < first_phy_addr) ||
		(phy_addr > (first_phy_addr + MALIBU_PHY_MAX_ADDR_INC))) {
		return SW_NOT_SUPPORTED;
	}

	phy_data = hsl_phy_mii_reg_read(dev_id,
		first_phy_addr + MALIBU_PHY_MAX_ADDR_INC, MALIBU_PHY_CHIP_CONFIG);
	copper_mode = ((phy_data & MALIBU_PHY_COPPER_MODE) >> 0xf);
	phy_data &= 0x000f;

	switch (phy_data) {
		case MALIBU_PHY_PSGMII_BASET:
			*interface_mode = PHY_PSGMII_BASET;
			break;
		case MALIBU_PHY_PSGMII_BX1000:
			if (phy_addr == first_phy_addr + MALIBU_PHY_MAX_ADDR_INC)
				*interface_mode = PHY_PSGMII_BX1000;
			else
				*interface_mode = PHY_PSGMII_BASET;
			break;
		case MALIBU_PHY_PSGMII_FX100:
			if (phy_addr == first_phy_addr + MALIBU_PHY_MAX_ADDR_INC)
				*interface_mode = PHY_PSGMII_FX100;
			else
				*interface_mode = PHY_PSGMII_BASET;
			break;
		case MALIBU_PHY_PSGMII_AMDET:
			if (copper_mode) {
				*interface_mode = PHY_PSGMII_BASET;
			 } else {
				if (phy_addr == first_phy_addr + MALIBU_PHY_MAX_ADDR_INC)
					*interface_mode = PHY_PSGMII_FIBER;
				else
					*interface_mode = PHY_PSGMII_BASET;
			 }
			break;
		case MALIBU_PHY_SGMII_BASET:
			if (phy_addr == first_phy_addr + MALIBU_PHY_MAX_ADDR_INC)
				*interface_mode = PHY_SGMII_BASET;
			else
				*interface_mode = PORT_QSGMII;
			break;
		default:
			*interface_mode = PORT_INTERFACE_MODE_MAX;
			break;
	}

	return SW_OK;
}

/******************************************************************************
*
* malibu_phy_interface mode status get
*
* get malibu phy interface mode status
*/
sw_error_t
malibu_phy_interface_get_mode_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_interface_mode_t *interface_mode_status)
{
	a_uint16_t phy_data, phy_mode, phy_mode_status;
	a_uint16_t copper_mode;

	if ((phy_addr < first_phy_addr) ||
		(phy_addr > (first_phy_addr + MALIBU_PHY_MAX_ADDR_INC))) {
		return SW_NOT_SUPPORTED;
	}

	phy_data = hsl_phy_mii_reg_read(dev_id,
		first_phy_addr + MALIBU_PHY_MAX_ADDR_INC, MALIBU_PHY_CHIP_CONFIG);
	copper_mode = ((phy_data & MALIBU_PHY_COPPER_MODE) >> 0xf);
	phy_mode = phy_data & 0x000f;
	phy_mode_status = (phy_data & 0x00f0) >> 0x4;

	if (phy_mode == MALIBU_PHY_PSGMII_AMDET) {
		if (copper_mode) {
			*interface_mode_status = PHY_PSGMII_BASET;
		} else {
			if (phy_addr == first_phy_addr + MALIBU_PHY_MAX_ADDR_INC)
				*interface_mode_status = PHY_PSGMII_FIBER;
			else
				*interface_mode_status = PHY_PSGMII_BASET;
		}
	} else {
		switch (phy_mode_status) {
			case MALIBU_PHY_PSGMII_BASET:
				*interface_mode_status = PHY_PSGMII_BASET;
				break;
			case MALIBU_PHY_PSGMII_BX1000:
				if (phy_addr == first_phy_addr + MALIBU_PHY_MAX_ADDR_INC)
					*interface_mode_status = PHY_PSGMII_BX1000;
				else
					*interface_mode_status = PHY_PSGMII_BASET;
				break;
			case MALIBU_PHY_PSGMII_FX100:
				if (phy_addr == first_phy_addr + MALIBU_PHY_MAX_ADDR_INC)
					*interface_mode_status = PHY_PSGMII_FX100;
				else
					*interface_mode_status = PHY_PSGMII_BASET;
				break;
			case MALIBU_PHY_SGMII_BASET:
				if (phy_addr == first_phy_addr + MALIBU_PHY_MAX_ADDR_INC)
					*interface_mode_status = PHY_SGMII_BASET;
				else
					*interface_mode_status = PORT_QSGMII;
				break;
			default:
				*interface_mode_status = PORT_INTERFACE_MODE_MAX;
				break;
		}
	}

	return SW_OK;
}

/******************************************************************************
*
* malibu_phy_get status
*
* get phy status
*/
sw_error_t
malibu_phy_get_status(a_uint32_t dev_id, a_uint32_t phy_addr,
		struct port_phy_status *phy_status)
{

	if (phy_addr == COMBO_PHY_ID) {
		__phy_reg_pages_sel_by_active_medium(dev_id, phy_addr);
	}
	return qcaphy_status_get(dev_id, phy_addr, phy_status);
}

#ifdef IN_LED
a_uint32_t
malibu_phy_led_source_map_mmd_reg_get(a_uint32_t dev_id, a_uint32_t source_id)
{
	a_uint16_t mmd_reg = 0;

	switch(source_id)
	{
		case QCAPHY_LED_SOURCE0:
			mmd_reg = MALIBU_PHY_MMD7_LED_100_N_MAP_CTRL;
			break;
		case QCAPHY_LED_SOURCE1:
			mmd_reg = MALIBU_PHY_MMD7_LED_1000_N_MAP_CTRL;
			break;
		default:
			SSDK_ERROR("source %d is not support\n", source_id);
			break;
	}

	return mmd_reg;
}

a_uint32_t
malibu_phy_led_source_force_mmd_reg_get(a_uint32_t dev_id, a_uint32_t source_id)
{
	a_uint16_t mmd_reg = 0;

	switch(source_id)
	{
		case QCAPHY_LED_SOURCE0:
			mmd_reg = MALIBU_PHY_MMD7_LED_100_N_FORCE_CTRL;
			break;
		case QCAPHY_LED_SOURCE1:
			mmd_reg = MALIBU_PHY_MMD7_LED_1000_N_FORCE_CTRL;
			break;
		default:
			SSDK_ERROR("source %d is not support\n", source_id);
			break;
	}

	return mmd_reg;
}

static sw_error_t
malibu_phy_led_force_pattern_set(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t source_id, a_bool_t enable, a_uint32_t force_mode)
{
	sw_error_t rv = SW_OK;
	a_uint32_t mmd_reg = 0;
	a_uint16_t phy_data = 0;

	mmd_reg = malibu_phy_led_source_force_mmd_reg_get(dev_id, source_id);
	if(enable) {
		rv = qcaphy_led_pattern_force_to_phy(dev_id, force_mode, &phy_data);
		PHY_RTN_ON_ERROR(rv);
	}
	return hsl_phy_modify_mmd(dev_id, phy_addr, A_FALSE, MALIBU_PHY_MMD7_NUM,
		mmd_reg, QCAPHY_PHY_LED_FORCE_EN | QCAPHY_PHY_LED_FORCE_MASK, phy_data);
}

static sw_error_t
malibu_phy_led_force_pattern_get(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t source_id, a_bool_t *enable, a_uint32_t *force_mode)
{
	sw_error_t rv = SW_OK;
	a_uint32_t mmd_reg = 0;
	a_uint16_t phy_data = 0;

	mmd_reg = malibu_phy_led_source_force_mmd_reg_get(dev_id, source_id);
	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_FALSE, MALIBU_PHY_MMD7_NUM,
		mmd_reg);
	PHY_RTN_ON_READ_ERROR(phy_data);
	if(phy_data  & QCAPHY_PHY_LED_FORCE_EN) {
		*enable = A_TRUE;
		rv = qcaphy_led_pattern_force_from_phy(dev_id, force_mode, phy_data);
		PHY_RTN_ON_ERROR(rv);
	}
	else
		*enable = A_FALSE;

	return SW_OK;
}

sw_error_t
malibu_phy_led_ctrl_source_set(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t source_id, led_ctrl_pattern_t *pattern)
{
	sw_error_t rv = SW_OK;
	a_uint32_t mmd_reg = 0;
	a_uint16_t phy_data = 0;

	if(pattern->mode == LED_PATTERN_MODE_BUTT || source_id > QCAPHY_LED_SOURCE1)
		return SW_NOT_SUPPORTED;

	rv = qcaphy_led_blink_freq_set(dev_id, phy_addr, pattern->mode, pattern->freq);
	PHY_RTN_ON_ERROR(rv);

	if(pattern->mode == LED_PATTERN_MAP_EN) {
		rv = malibu_phy_led_force_pattern_set(dev_id, phy_addr, source_id, A_FALSE,
			pattern->mode);
		PHY_RTN_ON_ERROR(rv);
		rv = qcaphy_led_pattern_map_to_phy(dev_id, pattern->map, &phy_data);
		PHY_RTN_ON_ERROR(rv);
		mmd_reg = malibu_phy_led_source_map_mmd_reg_get(dev_id, source_id);
		rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_FALSE, MALIBU_PHY_MMD7_NUM,
			mmd_reg, phy_data);
		PHY_RTN_ON_ERROR(rv);
	} else {
		rv = malibu_phy_led_force_pattern_set(dev_id, phy_addr, source_id, A_TRUE,
			pattern->mode);
		PHY_RTN_ON_ERROR(rv);
	}

	return SW_OK;
}

sw_error_t
malibu_phy_led_ctrl_source_get(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t source_id, led_ctrl_pattern_t *pattern)
{
	sw_error_t rv = SW_OK;
	a_uint32_t mmd_reg = 0;
	a_uint16_t phy_data = 0;
	a_bool_t force_enable = A_FALSE;

	if(source_id > QCAPHY_LED_SOURCE1)
		return SW_NOT_SUPPORTED;
	pattern->map = 0;
	rv = malibu_phy_led_force_pattern_get(dev_id, phy_addr, source_id, &force_enable,
		&(pattern->mode));
	PHY_RTN_ON_ERROR(rv);
	if(!force_enable) {
		pattern->mode = LED_PATTERN_MAP_EN;
		mmd_reg = malibu_phy_led_source_map_mmd_reg_get(dev_id, source_id);
		phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_FALSE,
			MALIBU_PHY_MMD7_NUM, mmd_reg);
		PHY_RTN_ON_READ_ERROR(phy_data);
		rv = qcaphy_led_pattern_map_from_phy(dev_id, &(pattern->map), phy_data);
		PHY_RTN_ON_ERROR(rv);
	}
	rv = qcaphy_led_blink_freq_get(dev_id, phy_addr, pattern->mode, &(pattern->freq));
	PHY_RTN_ON_ERROR(rv);

	return SW_OK;
}
#endif
/******************************************************************************
*
* malibu_phy_hw_register init
*
*/
sw_error_t
malibu_phy_hw_init(a_uint32_t dev_id, a_uint32_t port_bmp)
{
	a_uint32_t port_id = 0, phy_addr = 0, phy_cnt = 0;
	a_uint32_t mode = 0;

	for (port_id = 0; port_id < SW_MAX_NR_PORT; port_id ++)
	{
		if (port_bmp & (0x1 << port_id))
		{
			phy_cnt ++;
			phy_addr = qca_ssdk_port_to_phy_addr(dev_id, port_id);
			if (phy_addr < first_phy_addr)
			{
				first_phy_addr = phy_addr;
			}
			/*change malibu control_dac[2:0] of MMD7 0x801A bit[9:7] from 111 to 101*/
			hsl_phy_modify_mmd(dev_id, phy_addr, A_FALSE, MALIBU_PHY_MMD7_NUM,
				MALIBU_PHY_MMD7_DAC_CTRL, MALIBU_DAC_CTRL_MASK,
				MALIBU_DAC_CTRL_VALUE);
			/* add 10M and 100M link LED behavior for QFN board*/
			hsl_phy_modify_mmd(dev_id, phy_addr, A_FALSE, MALIBU_PHY_MMD7_NUM,
				MALIBU_PHY_MMD7_LED_1000_CTRL1, MALIBU_LED_1000_CTRL1_100_10_MASK,
				MALIBU_LED_1000_CTRL1_100_10_MASK);
			/*disable Extended next page*/
			hsl_phy_modify_mii(dev_id, phy_addr, MALIBU_AUTONEG_ADVERT,
				MALIBU_EXTENDED_NEXT_PAGE_EN, 0);
		}
	}
	/* qca 8072 two ports phy chip's firstly address to init phy chip */
	if ((phy_cnt == QCA8072_PHY_NUM) && (first_phy_addr >= 0x3)) {
		first_phy_addr = first_phy_addr - 0x3;
	}

	/*workaround to enable AZ transmitting ability*/
	hsl_phy_mmd_reg_write(dev_id, first_phy_addr + 5, A_FALSE, MALIBU_PHY_MMD1_NUM,
		MALIBU_PSGMII_MODE_CTRL, MALIBU_PHY_PSGMII_MODE_CTRL_ADJUST_VALUE);

	/* adjust psgmii serdes tx amp */
	hsl_phy_mii_reg_write(dev_id, first_phy_addr + 5,
		MALIBU_PSGMII_TX_DRIVER_1_CTRL, MALIBU_PHY_PSGMII_REDUCE_SERDES_TX_AMP);

	/* to avoid psgmii module goes into hibernation, work with psgmii self test*/
	hsl_phy_modify_mmd(dev_id, first_phy_addr + 4, A_FALSE, MALIBU_PHY_MMD3_NUM,
		MALIBU_PHY_MMD3_ADDR_REMOTE_LOOPBACK_CTRL, BIT(1), 0);

	mode = ssdk_dt_global_get_mac_mode(dev_id, 0);
	if (mode == PORT_WRAPPER_PSGMII_FIBER)
		malibu_phy_interface_set_mode(dev_id, first_phy_addr, PHY_PSGMII_FIBER);

	/*init combo phy address*/
	combo_phy_addr = first_phy_addr+4;

	return SW_OK;
}

static int malibu_phy_api_ops_init(void)
{

	int ret;
	hsl_phy_ops_t *malibu_phy_api_ops = NULL;

	malibu_phy_api_ops = kzalloc(sizeof(hsl_phy_ops_t), GFP_KERNEL);
	if (malibu_phy_api_ops == NULL) {
		SSDK_ERROR("malibu phy ops kzalloc failed!\n");
		return -ENOMEM;
	}

	phy_api_ops_init(MALIBU_PHY_CHIP);

	malibu_phy_api_ops->phy_speed_get = malibu_phy_get_speed;
	malibu_phy_api_ops->phy_speed_set = malibu_phy_set_speed;
	malibu_phy_api_ops->phy_duplex_get = malibu_phy_get_duplex;
	malibu_phy_api_ops->phy_duplex_set = malibu_phy_set_duplex;
	malibu_phy_api_ops->phy_autoneg_enable_set = malibu_phy_enable_autoneg;
	malibu_phy_api_ops->phy_restart_autoneg = malibu_phy_restart_autoneg;
	malibu_phy_api_ops->phy_autoneg_status_get = malibu_phy_autoneg_status;
	malibu_phy_api_ops->phy_autoneg_adv_set = malibu_phy_set_autoneg_adv;
	malibu_phy_api_ops->phy_autoneg_adv_get = malibu_phy_get_autoneg_adv;
	malibu_phy_api_ops->phy_link_status_get = malibu_phy_get_link_status;
#ifndef IN_PORTCONTROL_MINI
	malibu_phy_api_ops->phy_combo_prefer_medium_set = malibu_phy_set_combo_prefer_medium;
	malibu_phy_api_ops->phy_combo_prefer_medium_get = malibu_phy_get_combo_prefer_medium;
	malibu_phy_api_ops->phy_combo_medium_status_get = malibu_phy_get_combo_current_medium_type;
	malibu_phy_api_ops->phy_combo_fiber_mode_set = malibu_phy_set_combo_fiber_mode;
	malibu_phy_api_ops->phy_combo_fiber_mode_get = malibu_phy_get_combo_fiber_mode;
	malibu_phy_api_ops->phy_reset = malibu_phy_reset;
#endif
	malibu_phy_api_ops->phy_power_off = malibu_phy_poweroff;
	malibu_phy_api_ops->phy_power_on = malibu_phy_poweron;
	malibu_phy_api_ops->phy_id_get = qcaphy_get_phy_id;
	malibu_phy_api_ops->phy_interface_mode_set = malibu_phy_interface_set_mode;
	malibu_phy_api_ops->phy_interface_mode_get = malibu_phy_interface_get_mode;
	malibu_phy_api_ops->phy_interface_mode_status_get = malibu_phy_interface_get_mode_status;
	malibu_phy_api_ops->phy_serdes_reset = malibu_phy_serdes_reset;
	malibu_phy_api_ops->phy_get_status = malibu_phy_get_status;
#ifdef IN_LED
	malibu_phy_api_ops->phy_led_ctrl_source_set = malibu_phy_led_ctrl_source_set;
	malibu_phy_api_ops->phy_led_ctrl_source_get = malibu_phy_led_ctrl_source_get;
#endif
	ret = hsl_phy_api_ops_register(MALIBU_PHY_CHIP, malibu_phy_api_ops);

	if (ret == 0)
		SSDK_INFO("qca probe malibu phy driver succeeded!\n");
	else
		SSDK_ERROR("qca probe malibu phy driver failed! (code: %d)\n", ret);
	return ret;
}

/******************************************************************************
*
* malibu_phy_init -
*
*/
int malibu_phy_init(a_uint32_t dev_id, a_uint32_t port_bmp)
{
	static a_uint32_t phy_ops_flag = 0;

	if(phy_ops_flag == 0) {
		malibu_phy_api_ops_init();
		phy_ops_flag = 1;
	}
	malibu_phy_hw_init(dev_id, port_bmp);

	return 0;
}

