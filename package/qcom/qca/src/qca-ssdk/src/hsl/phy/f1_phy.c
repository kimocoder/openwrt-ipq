/*
 * Copyright (c) 2012, 2015-2018, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023-2024 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "sw.h"
#include "fal_port_ctrl.h"
#include "hsl_api.h"
#include "hsl.h"
#include "f1_phy.h"
#include "hsl_phy.h"
#include "qcaphy_common.h"
#include "ssdk_plat.h"

/******************************************************************************
*
* f1_phy_reset_done - reset the phy
*
* reset the phy
*/
a_bool_t
f1_phy_reset_done(a_uint32_t dev_id, a_uint32_t phy_addr)
{
    a_uint16_t phy_data;
    a_uint16_t ii = 200;

    do
    {
        phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, F1_PHY_CONTROL);
        aos_mdelay(10);
    }
    while ((!F1_RESET_DONE(phy_data)) && --ii);

    if (ii == 0)
        return A_FALSE;

    return A_TRUE;
}

/******************************************************************************
*
* f1_autoneg_done
*
* f1_autoneg_done
*/
a_bool_t
f1_autoneg_done(a_uint32_t dev_id, a_uint32_t phy_addr)
{
    a_uint16_t phy_data = 0;
    a_uint16_t ii = 200;

    do
    {
        phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, F1_PHY_STATUS);
        aos_mdelay(10);
    }
    while ((!F1_AUTONEG_DONE(phy_data)) && --ii);

    if (ii == 0)
        return A_FALSE;

    return A_TRUE;
}
#if 0
/******************************************************************************
*
* f1_phy_Speed_Duplex_Resolved
 - reset the phy
*
* reset the phy
*/
a_bool_t
f1_phy_speed_duplex_resolved(a_uint32_t dev_id, a_uint32_t phy_addr)
{
    a_uint16_t phy_data;
    a_uint16_t ii = 200;

    do
    {
        phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, F1_PHY_SPEC_STATUS);
        aos_mdelay(10);
    }
    while ((!F1_SPEED_DUPLEX_RESOVLED(phy_data)) && --ii);

    if (ii == 0)
        return A_FALSE;

    return A_TRUE;
}
#endif
/******************************************************************************
*
* f1_phy_get_speed - Determines the speed of phy ports associated with the
* specified device.
*/

sw_error_t
f1_phy_get_speed(a_uint32_t dev_id, a_uint32_t phy_addr,
                 fal_port_speed_t * speed)
{
    a_uint16_t phy_data;
    a_bool_t auto_neg;

    auto_neg = qcaphy_autoneg_status(dev_id, phy_addr);
    if (A_TRUE == auto_neg ) {
        qcaphy_get_speed(dev_id, phy_addr, speed);
    } else {
        phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, F1_PHY_CONTROL);
        switch (phy_data & F1_CTRL_SPEED_MASK)
        {
            case F1_CTRL_SPEED_1000:
                *speed = FAL_SPEED_1000;
                break;
            case F1_CTRL_SPEED_100:
                *speed = FAL_SPEED_100;
                break;
            case F1_CTRL_SPEED_10:
                *speed = FAL_SPEED_10;
            break;
            default:
                return SW_READ_ERROR;
        }
    }
    return SW_OK;
}

/******************************************************************************
*
* f1_phy_set_speed - Determines the speed of phy ports associated with the
* specified device.
*/
sw_error_t
f1_phy_set_speed(a_uint32_t dev_id, a_uint32_t phy_addr,
    fal_port_speed_t speed)
{
    a_uint16_t phy_data = 0;
    a_uint32_t autoneg, oldneg;
    fal_port_duplex_t old_duplex;

    if (FAL_SPEED_1000 == speed)
    {
        phy_data |= F1_CTRL_SPEED_1000;
        phy_data |= F1_CTRL_AUTONEGOTIATION_ENABLE;
    }
    else if (FAL_SPEED_100 == speed)
    {
        phy_data |= F1_CTRL_SPEED_100;
        phy_data &= ~F1_CTRL_AUTONEGOTIATION_ENABLE;
    }
    else if (FAL_SPEED_10 == speed)
    {
        phy_data |= F1_CTRL_SPEED_10;
        phy_data &= ~F1_CTRL_AUTONEGOTIATION_ENABLE;
    }
    else
    {
        return SW_BAD_PARAM;
    }

    qcaphy_get_autoneg_adv(dev_id, phy_addr, &autoneg);
    oldneg = autoneg;
    autoneg &= ~FAL_PHY_ADV_GE_SPEED_ALL;

    qcaphy_get_duplex(dev_id, phy_addr, &old_duplex);

    if (old_duplex == FAL_FULL_DUPLEX)
    {
        phy_data |= F1_CTRL_FULL_DUPLEX;

        if (FAL_SPEED_1000 == speed)
        {
            autoneg |= FAL_PHY_ADV_1000T_FD;
        }
        else if (FAL_SPEED_100 == speed)
        {
            autoneg |= FAL_PHY_ADV_100TX_FD;
        }
        else
        {
            autoneg |= FAL_PHY_ADV_10T_FD;
        }
    }
    else if (old_duplex == FAL_HALF_DUPLEX)
    {
        phy_data &= ~F1_CTRL_FULL_DUPLEX;

        if (FAL_SPEED_100 == speed)
        {
            autoneg |= FAL_PHY_ADV_100TX_HD;
        }
        else
        {
            autoneg |= FAL_PHY_ADV_10T_HD;
        }
    }
    else
    {
        return SW_FAIL;
    }

    qcaphy_set_autoneg_adv(dev_id, phy_addr, autoneg);
    qcaphy_autoneg_restart(dev_id, phy_addr);
    if(qcaphy_get_link_status(dev_id, phy_addr))
    {
        f1_autoneg_done(dev_id, phy_addr);
    }

    hsl_phy_mii_reg_write(dev_id, phy_addr, F1_PHY_CONTROL, phy_data);
    qcaphy_set_autoneg_adv(dev_id, phy_addr, oldneg);

    return SW_OK;

}

/******************************************************************************
*
* f1_phy_get_duplex - Determines the speed of phy ports associated with the
* specified device.
*/
sw_error_t
f1_phy_get_duplex(a_uint32_t dev_id, a_uint32_t phy_addr,
    fal_port_duplex_t * duplex)
{
    a_uint16_t phy_data;
    a_bool_t auto_neg;

    auto_neg = qcaphy_autoneg_status(dev_id, phy_addr);
    if (A_TRUE == auto_neg ) {
        qcaphy_get_duplex(dev_id, phy_addr, duplex);
    } else {
        phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, F1_PHY_CONTROL);
        //read duplex
        if (phy_data & F1_CTRL_FULL_DUPLEX)
            *duplex = FAL_FULL_DUPLEX;
        else
            *duplex = FAL_HALF_DUPLEX;
    }
    return SW_OK;
}

/******************************************************************************
*
* f1_phy_set_duplex - Determines the speed of phy ports associated with the
* specified device.
*/
sw_error_t
f1_phy_set_duplex(a_uint32_t dev_id, a_uint32_t phy_addr,
    fal_port_duplex_t duplex)
{
    a_uint16_t phy_data = 0;
    fal_port_speed_t old_speed = FAL_SPEED_10;
    a_uint32_t oldneg, autoneg;

    if (A_TRUE == qcaphy_autoneg_status(dev_id, phy_addr))
        phy_data &= ~F1_CTRL_AUTONEGOTIATION_ENABLE;

    qcaphy_get_autoneg_adv(dev_id, phy_addr, &autoneg);
    oldneg = autoneg;
    autoneg &= ~FAL_PHY_ADV_GE_SPEED_ALL;
    f1_phy_get_speed(dev_id, phy_addr, &old_speed);

    if (FAL_SPEED_1000 == old_speed)
    {
        phy_data |= F1_CTRL_SPEED_1000;
        phy_data |= F1_CTRL_AUTONEGOTIATION_ENABLE;
    }
    else if (FAL_SPEED_100 == old_speed)
    {
        phy_data |= F1_CTRL_SPEED_100;
    }
    else if (FAL_SPEED_10 == old_speed)
    {
        phy_data |= F1_CTRL_SPEED_10;
    }
    else
    {
        return SW_FAIL;
    }

    if (duplex == FAL_FULL_DUPLEX)
    {
        phy_data |= F1_CTRL_FULL_DUPLEX;

        if (FAL_SPEED_1000 == old_speed)
        {
            autoneg = FAL_PHY_ADV_1000T_FD;
        }
        else if (FAL_SPEED_100 == old_speed)
        {
            autoneg = FAL_PHY_ADV_100TX_FD;
        }
        else
        {
            autoneg = FAL_PHY_ADV_10T_FD;
        }
    }
    else if (duplex == FAL_HALF_DUPLEX)
    {
        phy_data &= ~F1_CTRL_FULL_DUPLEX;

        if (FAL_SPEED_100 == old_speed)
        {
            autoneg = FAL_PHY_ADV_100TX_HD;
        }
        else
        {
            autoneg = FAL_PHY_ADV_10T_HD;
        }
    }
    else
    {
        return SW_BAD_PARAM;
    }

    qcaphy_set_autoneg_adv(dev_id, phy_addr, autoneg);
    qcaphy_autoneg_restart(dev_id, phy_addr);
    if(qcaphy_get_link_status(dev_id, phy_addr))
    {
       f1_autoneg_done(dev_id, phy_addr);
    }

    hsl_phy_mii_reg_write(dev_id, phy_addr, F1_PHY_CONTROL, phy_data);
    qcaphy_set_autoneg_adv(dev_id, phy_addr, oldneg);

    return SW_OK;
}

static int f1_phy_api_ops_init(void)
{
	int ret;
	hsl_phy_ops_t *f1_phy_api_ops = NULL;

	f1_phy_api_ops = kzalloc(sizeof(hsl_phy_ops_t), GFP_KERNEL);
	if (f1_phy_api_ops == NULL) {
		SSDK_ERROR("f1 phy ops kzalloc failed!\n");
		return -ENOMEM;
	}

	phy_api_ops_init(F1_PHY_CHIP);

	f1_phy_api_ops->phy_speed_get = f1_phy_get_speed;
	f1_phy_api_ops->phy_speed_set = f1_phy_set_speed;
	f1_phy_api_ops->phy_duplex_get = f1_phy_get_duplex;
	f1_phy_api_ops->phy_duplex_set = f1_phy_set_duplex;
	f1_phy_api_ops->phy_autoneg_enable_set = qcaphy_autoneg_enable;
	f1_phy_api_ops->phy_restart_autoneg = qcaphy_autoneg_restart;
	f1_phy_api_ops->phy_autoneg_status_get = qcaphy_autoneg_status;
	f1_phy_api_ops->phy_autoneg_adv_set = qcaphy_set_autoneg_adv;
	f1_phy_api_ops->phy_autoneg_adv_get = qcaphy_get_autoneg_adv;
	f1_phy_api_ops->phy_link_status_get = qcaphy_get_link_status;
	f1_phy_api_ops->phy_reset = qcaphy_sw_reset;
	f1_phy_api_ops->phy_power_off = qcaphy_poweroff;
	f1_phy_api_ops->phy_power_on = qcaphy_poweron;
	f1_phy_api_ops->phy_id_get = qcaphy_get_phy_id;

	ret = hsl_phy_api_ops_register(F1_PHY_CHIP, f1_phy_api_ops);

	if (ret == 0)
		SSDK_INFO("qca probe f1 phy driver succeeded!\n");
	else
		SSDK_ERROR("qca probe f1 phy driver failed! (code: %d)\n", ret);
	return ret;
}

int f1_phy_init(a_uint32_t dev_id, a_uint32_t port_bmp)
{
	static a_uint32_t phy_ops_flag = 0;

	if(phy_ops_flag == 0) {
		f1_phy_api_ops_init();
		phy_ops_flag = 1;
	}

	return 0;
}

