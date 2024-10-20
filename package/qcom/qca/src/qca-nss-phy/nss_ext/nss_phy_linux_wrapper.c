/*
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
 */

#include "nss_phy.h"

void nss_phy_mdelay(u32 msecs)
{
	mdelay(msecs);
}

u32 nss_phydev_id_get(struct phy_device *phydev)
{
	if (phydev->is_c45)
		return phydev->c45_ids.device_ids
			[__ffs(phydev->c45_ids.mmds_present)];

	return phydev->phy_id;
}

bool nss_phydev_id_compare(struct phy_device *phydev, u32 phy_id,
	u32 mask)
{
	return phy_id_compare(nss_phydev_id_get(phydev), phy_id, mask);
}

int __nss_phy_read(struct nss_phy_device *nss_phydev, u32 reg)
{
	return __phy_read(nss_phydev->phydev, reg);
}

int __nss_phy_write(struct nss_phy_device *nss_phydev, u32 reg, u16 val)
{
	return __phy_write(nss_phydev->phydev, reg, val);
}

int __nss_phy_modify(struct nss_phy_device *nss_phydev, u32 reg,
	u16 mask, u16 set)
{
	return __phy_modify(nss_phydev->phydev, reg, mask, set);
}

int nss_phy_read(struct nss_phy_device *nss_phydev, u32 reg)
{
	return phy_read(nss_phydev->phydev, reg);
}

int nss_phy_write(struct nss_phy_device *nss_phydev, u32 reg, u16 val)
{
	return phy_write(nss_phydev->phydev, reg, val);
}

int nss_phy_modify(struct nss_phy_device *nss_phydev, u32 reg, u16 mask,
	u16 set)
{
	return phy_modify(nss_phydev->phydev, reg, mask, set);
}

int __nss_phy_read_mmd(struct nss_phy_device *nss_phydev, int devad,
	u32 reg)
{
	return __phy_read_mmd(nss_phydev->phydev, devad, reg);
}

int __nss_phy_write_mmd(struct nss_phy_device *nss_phydev, int devad,
	u32 reg, u16 val)
{
	return __phy_write_mmd(nss_phydev->phydev, devad, reg, val);
}

int __nss_phy_modify_mmd(struct nss_phy_device *nss_phydev, int devad,
	u32 reg, u16 mask, u16 set)
{
	return __phy_modify_mmd(nss_phydev->phydev, devad, reg, mask, set);
}

int nss_phy_read_mmd(struct nss_phy_device *nss_phydev, int devad,
	u32 reg)
{
	return phy_read_mmd(nss_phydev->phydev, devad, reg);
}

int nss_phy_write_mmd(struct nss_phy_device *nss_phydev, int devad,
	u32 reg, u16 val)
{
	return phy_write_mmd(nss_phydev->phydev, devad, reg, val);
}

int nss_phy_modify_mmd(struct nss_phy_device *nss_phydev, int devad,
	u32 reg, u16 mask, u16 set)
{
	return phy_modify_mmd(nss_phydev->phydev, devad, reg, mask, set);
}

static int __nss_phy_c22_read_debug(struct nss_phy_device *nss_phydev,
	u16 reg)
{
	int ret;

	ret = __nss_phy_write(nss_phydev, 29, reg);
	if (ret < 0)
		return ret;

	return __nss_phy_read(nss_phydev, 30);
}

static int __nss_phy_c45_read_debug(struct nss_phy_device *nss_phydev,
	u16 reg)
{
	int ret;

	ret = __nss_phy_write_mmd(nss_phydev, 31, 29, reg);
	if (ret < 0)
		return ret;

	return __nss_phy_read_mmd(nss_phydev, 31, 30);
}

int __nss_phy_read_debug(struct nss_phy_device *nss_phydev, u16 reg)
{
	if (nss_phydev->phydev->is_c45)
		return __nss_phy_c45_read_debug(nss_phydev, reg);

	return __nss_phy_c22_read_debug(nss_phydev, reg);
}

static int __nss_phy_c22_write_debug(struct nss_phy_device *nss_phydev,
	u16 reg, u16 data)
{
	int ret;

	ret = __nss_phy_write(nss_phydev, 29, reg);
	if (ret < 0)
		return ret;

	return __nss_phy_write(nss_phydev, 30, data);
}

static int __nss_phy_c45_write_debug(struct nss_phy_device *nss_phydev,
	u16 reg, u16 data)
{
	int ret;

	ret = __nss_phy_write_mmd(nss_phydev, 31, 29, reg);
	if (ret < 0)
		return ret;

	return __nss_phy_write_mmd(nss_phydev, 31, 30, data);
}

int __nss_phy_write_debug(struct nss_phy_device *nss_phydev, u16 reg,
	u16 data)
{
	if (nss_phydev->phydev->is_c45)
		return __nss_phy_c45_write_debug(nss_phydev, reg, data);

	return __nss_phy_c22_write_debug(nss_phydev, reg, data);
}

int __nss_phy_modify_debug(struct nss_phy_device *nss_phydev, u16 reg,
	u16 mask, u16 set)
{
	u16 new;
	int ret;

	ret = __nss_phy_read_debug(nss_phydev, reg);
	if (ret < 0)
		return ret;

	new = (ret & ~mask) | set;

	return __nss_phy_write_debug(nss_phydev, reg, new);
}

int nss_phy_read_debug(struct nss_phy_device *nss_phydev,
	u16 reg)
{
	int ret;

	phy_lock_mdio_bus(nss_phydev->phydev);
	ret = __nss_phy_read_debug(nss_phydev, reg);
	phy_unlock_mdio_bus(nss_phydev->phydev);

	return ret;
}

int nss_phy_write_debug(struct nss_phy_device *nss_phydev, u16 reg,
	u16 data)
{
	int ret;

	phy_lock_mdio_bus(nss_phydev->phydev);
	ret = __nss_phy_write_debug(nss_phydev, reg, data);
	phy_unlock_mdio_bus(nss_phydev->phydev);

	return ret;
}

int nss_phy_modify_debug(struct nss_phy_device *nss_phydev, u16 reg,
	u16 mask, u16 set)
{
	int ret;

	phy_lock_mdio_bus(nss_phydev->phydev);
	ret = __nss_phy_modify_debug(nss_phydev, reg, mask, set);
	phy_unlock_mdio_bus(nss_phydev->phydev);

	return ret;
}

bool nss_phy_id_check(struct nss_phy_device *nss_phydev, u32 phy_id,
	u32 mask)
{
	return nss_phydev_id_compare(nss_phydev->phydev, phy_id, mask);
}

int nss_phy_addr_get(struct nss_phy_device *nss_phydev)
{
	return nss_phydev->phydev->mdio.addr;
}

int nss_phy_share_addr_get(struct nss_phy_device *nss_phydev)
{
	if (nss_phydev->phydev->shared)
		return nss_phydev->phydev->shared->addr;

	return 0;
}

int __nss_phy_read_package(struct nss_phy_device *nss_phydev,
	int addr_offset, u16 reg)
{
	int addr = 0;

	addr = nss_phy_share_addr_get(nss_phydev) + addr_offset;
	return __mdiobus_read(nss_phydev->phydev->mdio.bus, addr, reg);
}

int __nss_phy_write_package(struct nss_phy_device *nss_phydev,
	int addr_offset, u16 reg, u16 data)
{
	int addr = 0;

	addr = nss_phy_share_addr_get(nss_phydev) + addr_offset;
	return __mdiobus_write(nss_phydev->phydev->mdio.bus, addr,
		reg, data);
}

int __nss_phy_modify_package(struct nss_phy_device *nss_phydev,
	int addr_offset, u16 reg, u16 mask, u16 set)
{
	int addr = 0;

	addr = nss_phy_share_addr_get(nss_phydev) + addr_offset;
	return __mdiobus_modify(nss_phydev->phydev->mdio.bus, addr,
		reg, mask, set);
}

int nss_phy_read_package(struct nss_phy_device *nss_phydev,
	int addr_offset, u16 reg)
{
	int ret;

	phy_lock_mdio_bus(nss_phydev->phydev);
	ret = __nss_phy_read_package(nss_phydev, addr_offset, reg);
	phy_unlock_mdio_bus(nss_phydev->phydev);

	return ret;
}

int nss_phy_write_package(struct nss_phy_device *nss_phydev,
	int addr_offset, u16 reg, u16 data)
{
	int ret;

	phy_lock_mdio_bus(nss_phydev->phydev);
	ret = __nss_phy_write_package(nss_phydev, addr_offset,
		reg, data);
	phy_unlock_mdio_bus(nss_phydev->phydev);

	return ret;
}

int nss_phy_modify_package(struct nss_phy_device *nss_phydev,
	int addr_offset, u16 reg, u16 mask, u16 set)
{
	int ret;

	phy_lock_mdio_bus(nss_phydev->phydev);
	ret = __nss_phy_modify_package(nss_phydev, addr_offset, reg,
		mask, set);
	phy_unlock_mdio_bus(nss_phydev->phydev);

	return ret;
}

void *nss_phy_kzalloc(unsigned size)
{
	return kzalloc(sizeof(struct nss_phy_ops), GFP_KERNEL);
}

int nss_phydev_speed_get(struct nss_phy_device *nss_phydev)
{
	return nss_phydev->phydev->speed;
}

int nss_phydev_link_get(struct nss_phy_device *nss_phydev)
{
	return nss_phydev->phydev->link;
}

int nss_phydev_autoneg_update(struct nss_phy_device *nss_phydev, u32 enable)
{
	nss_phydev->phydev->autoneg = enable;

	return 0;
}

int nss_phydev_eee_update(struct nss_phy_device *nss_phydev, u32 adv)
{
	nss_phydev->phydev->eee_enabled = (adv == 0) ? false:true;

	linkmode_mod_bit(ETHTOOL_LINK_MODE_100baseT_Full_BIT,
		nss_phydev->phydev->advertising_eee, adv & EEE_100BASE_T);
	linkmode_mod_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
		nss_phydev->phydev->advertising_eee, adv & EEE_1000BASE_T);
	linkmode_mod_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT,
		nss_phydev->phydev->advertising_eee, adv & EEE_2500BASE_T);
	linkmode_mod_bit(ETHTOOL_LINK_MODE_5000baseT_Full_BIT,
		nss_phydev->phydev->advertising_eee, adv & EEE_5000BASE_T);
	linkmode_mod_bit(ETHTOOL_LINK_MODE_10000baseT_Full_BIT,
		nss_phydev->phydev->advertising_eee, adv & EEE_10000BASE_T);

	return 0;
}

int nss_phy_package_read_mmd(struct nss_phy_device *nss_phydev,
	unsigned int addr_offset, int devad, u32 regnum)
{
	int addr = 0;

	addr = nss_phy_share_addr_get(nss_phydev) + addr_offset;

	if (addr < 0)
		return addr;

	return mdiobus_c45_read(nss_phydev->phydev->mdio.bus, addr,
		devad, regnum);

}

int nss_phy_package_modify_mmd(struct nss_phy_device *nss_phydev,
	unsigned int addr_offset, int devad, u32 regnum, u16 mask, u16 set)
{
	int new, ret;
	int addr = 0;

	addr = nss_phy_share_addr_get(nss_phydev) + addr_offset;

	phy_lock_mdio_bus(nss_phydev->phydev);
	ret = __mdiobus_c45_read(nss_phydev->phydev->mdio.bus, addr, devad,
		regnum);
	new = (ret & ~mask) | set;
	ret |= __mdiobus_c45_write(nss_phydev->phydev->mdio.bus, addr, devad,
		regnum, new);
	phy_unlock_mdio_bus(nss_phydev->phydev);

	return ret;
}

bool nss_phy_support_2500(struct nss_phy_device *nss_phydev)
{
	return (linkmode_test_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT,
		nss_phydev->phydev->advertising) &&
		(linkmode_test_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT,
		nss_phydev->phydev->supported)));
}

bool nss_phy_is_fiber(struct nss_phy_device *nss_phydev)
{
	return (nss_phydev->phydev->port == PORT_FIBRE);
}

u32 __nss_phy_read_soc(struct nss_phy_device *nss_phydev, u32 reg)
{
	struct nss_phy_linux_mdio_data *mdio_priv;

	mdio_priv = nss_phydev->phydev->mdio.bus->priv;

	return mdio_priv->sw_read(nss_phydev->phydev->mdio.bus, reg);
}

int __nss_phy_write_soc(struct nss_phy_device *nss_phydev, u32 reg, u32 val)
{
	struct nss_phy_linux_mdio_data *mdio_priv;

	mdio_priv = nss_phydev->phydev->mdio.bus->priv;

	mdio_priv->sw_write(nss_phydev->phydev->mdio.bus, reg, val);

	return 0;
}

int __nss_phy_modify_soc(struct nss_phy_device *nss_phydev, u32 reg,
	u32 mask, u32 set)
{
	int val, new;

	val = __nss_phy_read_soc(nss_phydev, reg);
	new = (val & ~mask) | set;
	if (new == val)
		return 0;

	return __nss_phy_write_soc(nss_phydev, reg, new);
}

u32 nss_phy_read_soc(struct nss_phy_device *nss_phydev, u32 reg)
{
	int val;

	phy_lock_mdio_bus(nss_phydev->phydev);
	val = __nss_phy_read_soc(nss_phydev, reg);
	phy_unlock_mdio_bus(nss_phydev->phydev);

	return val;
}

int nss_phy_write_soc(struct nss_phy_device *nss_phydev, u32 reg, u32 val)
{
	phy_lock_mdio_bus(nss_phydev->phydev);
	__nss_phy_write_soc(nss_phydev, reg, val);
	phy_unlock_mdio_bus(nss_phydev->phydev);

	return 0;
}

int nss_phy_modify_soc(struct nss_phy_device *nss_phydev, u32 reg,
	u32 mask, u32 set)
{
	int ret;

	phy_lock_mdio_bus(nss_phydev->phydev);
	ret = __nss_phy_modify_soc(nss_phydev, reg, mask, set);
	phy_unlock_mdio_bus(nss_phydev->phydev);

	return ret;
}
