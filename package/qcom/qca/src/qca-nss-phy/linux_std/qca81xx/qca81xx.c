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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/phy.h>
#include <linux/bitfield.h>

#define QCA8111_PHY		0x004dd1c0
/* in QCOM MDIO bus driver, bit29~31 is for soc type, 2 is for laguna */
/* and bit24~28 is for phy address, 0~23 is for soc address */
#define TO_QCA81XX_PHY_SOC_ADDR(addr, reg)		\
	((BIT(30) | reg) | (addr << 24))

enum qca81xx_addr_offset {
	PCS_ADDR_OFFSET = 1,
	SOC_ADDR_OFFSET = 2,
};

struct qca81xx_phy_mdio_data {
	void __iomem	*membase[2];
	void __iomem *eth_ldo_rdy[3];
	int clk_div;
	bool force_c22;
	struct gpio_descs *reset_gpios;
	void (*preinit)(struct mii_bus *bus);
	u32 (*sw_read)(struct mii_bus *bus, u32 reg);
	void (*sw_write)(struct mii_bus *bus, u32 reg, u32 val);
	struct clk *clk[5];
	void *i2c;
};

/* below two registers are used to access PHY */
/* DEBUG registers indirectly */
#define QCA81XX_DEBUG_ADDR		0x1d
#define QCA81XX_DEBUG_DATA		0x1e

/*PHY DEBUG registers*/
#define QCA81XX_ANA_DEBUG_AFE_DAC8_DP		0x2f80
#define QCA81XX_ANA_DEBUG_AFE_DAC8_DP_VAL		0x5b56
#define QCA81XX_ANA_DEBUG_AFE_DAC9_DP		0x3080
#define QCA81XX_ANA_DEBUG_AFE_DAC9_DP_VAL		0x5b57
#define QCA81XX_ANA_DEBUG_AFE_DAC38_DP		0x4d80
#define QCA81XX_ANA_DEBUG_AFE_DAC38_DP_VAL		0x2a2a
#define QCA81XX_ANA_DEBUG_AFE_DAC39_DP		0x4e80
#define QCA81XX_ANA_DEBUG_AFE_DAC39_DP_VAL		0x2a2a

/*PHY MMD3 registers*/
#define QCA81XX_MMD3_CDT_THRESH_CTRL2		0x8073
#define QCA81XX_MMD3_CDT_THRESH_CTRL2_VAL		0xb03f
#define QCA81XX_MMD3_CDT_THRESH_CTRL3		0x8074
#define QCA81XX_MMD3_CDT_THRESH_CTRL3_VAL		0xc040
#define QCA81XX_MMD3_CDT_THRESH_CTRL4		0x8075
#define QCA81XX_MMD3_CDT_THRESH_CTRL4_VAL		0xa060
#define QCA81XX_MMD3_CDT_THRESH_CTRL5		0x8076
#define QCA81XX_MMD3_CDT_THRESH_CTRL5_VAL		0xc040
#define QCA81XX_MMD3_CDT_THRESH_CTRL6		0x8077
#define QCA81XX_MMD3_CDT_THRESH_CTRL6_VAL		0xa060
#define QCA81XX_MMD3_CDT_THRESH_CTRL7		0x8078
#define QCA81XX_MMD3_CDT_THRESH_CTRL7_VAL		0xae50
#define QCA81XX_MMD3_CDT_THRESH_CTRL9		0x807a
#define QCA81XX_MMD3_CDT_THRESH_CTRL9_VAL		0xc060
#define QCA81XX_MMD3_CDT_THRESH_CTRL13		0x807e
#define QCA81XX_MMD3_CDT_THRESH_CTRL13_VAL		0xb060
#define QCA81XX_MMD3_CDT_THRESH_CTRL14		0x807f
#define QCA81XX_MMD3_CDT_THRESH_CTRL14_VAL		0x9cb0
#define QCA81XX_MMD3_DEBUG5		0xa015
#define QCA81XX_MMD3_DEBUG5_VAL		0xce80

/*PHY MMD31 registers*/
#define QCA81XX_FIFO_CONTROL		0x19
#define QCA81XX_FIFO_RESET		0x3

#define QCA81XX_1000BASET_CONTROL		0x9
#define QCA81XX_ADVERTISE_1000FULL		0x200
#define QCA81XX_1000BASET_STATUS		0xa
#define QCA81XX_LP_ADVERTISE_1000FULL		0x2000

#define QCA81XX_SPEC_STATUS		0x11
#define QCA81XX_INTR_DOWNSHIFT		0x20
#define QCA81XX_SS_DUPLEX_FULL		0x2000
#define QCA81XX_SS_SPEED_MASK		0x380
#define QCA81XX_SS_SPEED_10000		0x180
#define QCA81XX_SS_SPEED_5000		0x280
#define QCA81XX_SS_SPEED_2500		0x200
#define QCA81XX_SS_SPEED_1000		0x100
#define QCA81XX_SS_SPEED_100		0x80

#define QCA81XX_INTR_MASK		0x12
#define QCA81XX_INTR_STATUS		0x13
#define QCA81XX_INTR_STATUS_DOWN		0x800
#define QCA81XX_INTR_STATUS_UP		0x400

#define QCA81XX_SMART_SPEED		0x14
#define QCA81XX_AUTO_SOFT_RESET		0x8000

/*PCS MII registers*/
#define QCA81XX_PCS_PLL_POWER_ON_AND_RESET		0
#define QCA81XX_PCS_ANA_SOFT_RESET_MASK		0x40
#define QCA81XX_PCS_ANA_SOFT_RESET		0
#define QCA81XX_PCS_ANA_SOFT_RELEASE		0x40

#define QCA81XX_PCS_MII_DIG_CTRL		0x8000
#define QCA81XX_PCS_MMD3_USXG_FIFO_RESET		0x400

/*PCS MMD1 registers*/
#define QCA81XX_PCS_MMD1_MODE_CTRL		0x11b
#define QCA81XX_PCS_MMD1_MODE_MASK		0x1f00
#define QCA81XX_PCS_MMD1_XPCS_MODE		0x1000

#define QCA81XX_PCS_MMD1_CDA_CONTROL1		0x20
#define QCA81XX_PCS_MMD1_SSCG_ENABLE		0x8

#define QCA81XX_PCS_MMD1_CALIBRATION4		0x78
#define QCA81XX_PCS_MMD1_CALIBRATION_DONE		0x80

/*PCS MMD3 registers*/
#define QCA81XX_PCS_MMD3_AN_LP_BASE_ABL2		0x14
#define QCA81XX_PCS_MMD3_XPCS_EEE_CAP		0x40

#define QCA81XX_PCS_MMD3_PCS_CTRL2		0x7
#define QCA81XX_PCS_MMD3_PCS_TYPE_MASK		0xf
#define QCA81XX_PCS_MMD3_PCS_TYPE_10GBASE_R		0

#define QCA81XX_PCS_MMD3_10GBASE_R_PCS_STATUS1		0x20
#define QCA81XX_PCS_MMD3_10GBASE_R_UP		0x1000

#define QCA81XX_PCS_MMD3_DIG_CTRL1		0x8000
#define QCA81XX_PCS_MMD3_USXGMII_EN		0x200
#define QCA81XX_PCS_MMD3_XPCS_SOFT_RESET		0x8000

#define QCA81XX_PCS_MMD3_AN_LP_BASE_ABL2		0x14

#define QCA81XX_PCS_MMD3_EEE_MODE_CTRL		0x8006
#define QCA81XX_PCS_MMD3_EEE_RES_REGS		0x100
#define QCA81XX_PCS_MMD3_EEE_SIGN_BIT_REGS		0x40
#define QCA81XX_PCS_MMD3_EEE_EN		0x3

#define QCA81XX_PCS_MMD3_EEE_TX_TIMER		0x8008
#define QCA81XX_PCS_MMD3_EEE_TSL_REGS		0xa
#define QCA81XX_PCS_MMD3_EEE_TLU_REGS		0xc0
#define QCA81XX_PCS_MMD3_EEE_TWL_REGS		0x1600

#define QCA81XX_PCS_MMD3_EEE_MODE_CTRL1		0x800b
#define QCA81XX_PCS_MMD3_EEE_TRANS_LPI_MODE		0x1
#define QCA81XX_PCS_MMD3_EEE_TRANS_RX_LPI_MODE		0x100

#define QCA81XX_PCS_MMD3_EEE_RX_TIMER		0x8009
#define QCA81XX_PCS_MMD3_EEE_100US_REG_REGS		0xc8
#define QCA81XX_PCS_MMD3_EEE_RWR_REG_REGS		0x1c00

#define QCA81XX_PCS_MMD3_USXG_FIFO_RESET		0x400

/*PCS MMD31 register*/
#define QCA81XX_PCS_MMD31_MII_DIG_CTRL	0x8000
#define QCA81XX_PCS_MMD31_PHY_MODE_CTRL_EN		0x1

#define QCA81XX_PCS_MMD31_MII_AN_INT_MSK		0x8001
#define QCA81XX_PCS_MMD31_AN_COMPLETE_INT		0x1
#define QCA81XX_PCS_MMD31_MII_4BITS_CTRL		0
#define QCA81XX_PCS_MMD31_TX_CONFIG_CTRL		0x8

#define QCA81XX_PCS_MMD31_MII_ERR_SEL		0x8002
#define QCA81XX_PCS_MMD31_MII_XAUI_MODE_CTRL		0x8004
#define QCA81XX_PCS_MMD31_MII_CTRL		0
#define QCA81XX_PCS_MMD31_MII_AN_ENABLE		0x1000

#define QCA81XX_PCS_MMD31_MII_ERR_SEL		0x8002
#define QCA81XX_PCS_MMD31_XPCS_SPEED_MASK		0x2060
#define QCA81XX_PCS_MMD31_XPCS_SPEED_10000	0x2040
#define QCA81XX_PCS_MMD31_XPCS_SPEED_5000		0x2020
#define QCA81XX_PCS_MMD31_XPCS_SPEED_2500		0x20
#define QCA81XX_PCS_MMD31_XPCS_SPEED_1000		0x40
#define QCA81XX_PCS_MMD31_XPCS_SPEED_100		0x2000
#define QCA81XX_PCS_MMD31_AN_RESTART		0x200
#define QCA81XX_PCS_MMD31_MII_AN_COMPLETE_INT		0x1

/*SOC GCC registers*/
#define GCC_E2S_TX_CMD_RCGR		0x800000
#define GCC_E2S_TX_CFG_RCGR		0x800004
#define GCC_E2S_TX_DIV_CDIVR		0x800008
#define GCC_E2S_SRDS_CH0_RX_CBCR		0x800010
#define GCC_E2S_GEPHY_TX_CBCR		0x800014
#define GCC_E2S_RX_CMD_RCGR		0x800018
#define GCC_E2S_RX_CFG_RCGR		0x80001c
#define GCC_E2S_RX_DIV_CDIVR		0x800020
#define GCC_E2S_SRDS_CH0_TX_CBCR		0x800028
#define GCC_E2S_GEPHY_RX_CBCR		0x80002c
#define GCC_AHB_CMD_RCGR		0x80003c
#define GCC_AHB_CFG_RCGR		0x800040
#define GCC_SRDS_SYS_CBCR		0x80007c
#define GCC_GEPHY_SYS_CBCR		0x800080
#define GCC_SEC_CTRL_CMD_RCGR		0x800088
#define GCC_SEC_CTRL_CFG_RCGR		0x80008c
#define GCC_SERDES_CTL		0x80030C

#define GCC_CLK_ENABLE		0x1
#define GCC_CLK_ARES		0x4
#define XPCS_PWR_ARES		0x1
#define GCC_E2S_SRC_MASK		GENMASK(10, 8)
#define GCC_E2S_SRC0_REF_50MCLK		0
#define GCC_E2S_SRC1_EPHY_TXCLK		1
#define GCC_E2S_SRC2_EPHY_RXCLK		2
#define GCC_E2S_SRC3_SRDS_TXCLK		3
#define GCC_E2S_SRC4_SRDS_RXCLK		4

#define SRC_DIV_MASK		GENMASK(4, 0)
#define CLK_DIV_MASK		GENMASK(3, 0)
#define CLK_CMD_UPDATE		BIT(0)

/*SOC SEC_TCSR registers*/
#define EPHY_CFG		0x90F018
#define EPHY_LDO_CTRL		BIT(20)

static int __qca81xx_phy_debug_write(struct phy_device *phydev,
	unsigned int reg, u16 val)
{
	int ret;

	ret = __phy_write_mmd(phydev, MDIO_MMD_VEND2,
		QCA81XX_DEBUG_ADDR, reg);
	if (ret < 0)
		return ret;
	ret = __phy_write_mmd(phydev, MDIO_MMD_VEND2,
		QCA81XX_DEBUG_DATA, val);

	return ret;
}

static int qca81xx_phy_debug_write(struct phy_device *phydev,
	unsigned int reg, u16 val)
{
	int ret;

	phy_lock_mdio_bus(phydev);
	ret = __qca81xx_phy_debug_write(phydev, reg, val);
	phy_unlock_mdio_bus(phydev);

	return ret;
}

static int qca81xx_pcs_address(struct phy_device *phydev)
{
	return phydev->mdio.addr + PCS_ADDR_OFFSET;
}

static int qca81xx_pcs_read_mmd(struct phy_device *phydev,
	int devad, u32 regnum)
{
	int addr = qca81xx_pcs_address(phydev);

	return mdiobus_c45_read(phydev->mdio.bus, addr, devad, regnum);
}

static int qca81xx_pcs_modify_mmd(struct phy_device *phydev,
	int devad, u32 regnum, u16 mask, u16 set)
{
	int addr = qca81xx_pcs_address(phydev);

	return mdiobus_c45_modify(phydev->mdio.bus, addr, devad, regnum,
		mask, set);
}

static int qca81xx_pcs_modify(struct phy_device *phydev,
	u32 regnum, u16 mask, u16 set)
{
	int addr = qca81xx_pcs_address(phydev);

	return mdiobus_modify(phydev->mdio.bus, addr, regnum,
		mask, set);
}

static u32 qca81xx_soc_address(struct phy_device *phydev)
{
	return phydev->mdio.addr + SOC_ADDR_OFFSET;
}

static u32 __qca81xx_soc_read(struct phy_device *phydev, u32 reg)
{
	u32 reg_e, val;
	int addr;
	struct qca81xx_phy_mdio_data *mdio_priv = phydev->mdio.bus->priv;

	addr = qca81xx_soc_address(phydev);
	reg_e = TO_QCA81XX_PHY_SOC_ADDR(addr, reg);

	if (mdio_priv && mdio_priv->sw_read)
		val = mdio_priv->sw_read(phydev->mdio.bus, reg_e);

	return val;
}

static int __qca81xx_soc_write(struct phy_device *phydev,
	u32 reg, u32 val)
{
	u32 reg_e;
	int addr;
	struct qca81xx_phy_mdio_data *mdio_priv = phydev->mdio.bus->priv;

	addr = qca81xx_soc_address(phydev);
	reg_e = TO_QCA81XX_PHY_SOC_ADDR(addr, reg);

	if (mdio_priv && mdio_priv->sw_write)
		mdio_priv->sw_write(phydev->mdio.bus, reg_e, val);

	return 0;
}

static int qca81xx_soc_modify(struct phy_device *phydev, u32 reg,
	u32 mask, u32 set)
{
	u32 val;

	phy_lock_mdio_bus(phydev);
	val = __qca81xx_soc_read(phydev, reg);
	val = (val & ~mask) | set;
	__qca81xx_soc_write(phydev, reg, val);
	phy_unlock_mdio_bus(phydev);

	return 0;

}

static int qca81xx_pcs_txclk_en_set(struct phy_device *phydev,
	bool enable)
{
	return qca81xx_soc_modify(phydev, GCC_E2S_SRDS_CH0_TX_CBCR,
		GCC_CLK_ENABLE, enable ? GCC_CLK_ENABLE : 0);
}

static int qca81xx_pcs_rxclk_en_set(struct phy_device *phydev,
	bool enable)
{
	return qca81xx_soc_modify(phydev, GCC_E2S_SRDS_CH0_RX_CBCR,
		GCC_CLK_ENABLE, enable ? GCC_CLK_ENABLE : 0);
}

static int qca81xx_pcs_clk_en_set(struct phy_device *phydev,
	bool enable)
{
	int ret;

	ret = qca81xx_pcs_txclk_en_set(phydev, enable);
	if (ret < 0)
		return ret;

	return qca81xx_pcs_rxclk_en_set(phydev, enable);
}

static int qca81xx_pcs_clk_reset_update(struct phy_device *phydev,
	bool assert)
{
	int ret;

	ret = qca81xx_soc_modify(phydev, GCC_E2S_SRDS_CH0_RX_CBCR,
		GCC_CLK_ARES, assert ? GCC_CLK_ARES : 0);
	if (ret < 0)
		return ret;

	return qca81xx_soc_modify(phydev, GCC_E2S_SRDS_CH0_TX_CBCR,
		GCC_CLK_ARES, assert ? GCC_CLK_ARES : 0);
}

static int qca81xx_pcs_clk_reset(struct phy_device *phydev)
{
	int ret;

	ret = qca81xx_pcs_clk_reset_update(phydev, true);
	if (ret < 0)
		return ret;
	mdelay(1);

	return qca81xx_pcs_clk_reset_update(phydev, false);
}

static int qca81xx_pcs_sysclk_en_set(struct phy_device *phydev,
	bool enable)
{
	return qca81xx_soc_modify(phydev, GCC_SRDS_SYS_CBCR, GCC_CLK_ENABLE,
		enable ? GCC_CLK_ENABLE : 0);
}

static int qca81xx_pcs_sysclk_reset_update(struct phy_device *phydev,
	bool assert)
{
	return qca81xx_soc_modify(phydev, GCC_SRDS_SYS_CBCR, GCC_CLK_ARES,
		assert ? GCC_CLK_ARES : 0);
}

static int qca81xx_pcs_sysclk_reset(struct phy_device *phydev)
{
	int ret;

	ret = qca81xx_pcs_sysclk_reset_update(phydev, true);
	if (ret < 0)
		return ret;
	mdelay(1);

	return qca81xx_pcs_sysclk_reset_update(phydev, false);
}

static int qca81xx_xpcs_clk_reset_update(struct phy_device *phydev,
	bool assert)
{
	return qca81xx_soc_modify(phydev, GCC_SERDES_CTL, XPCS_PWR_ARES,
		assert ? XPCS_PWR_ARES : 0);
}

static int qca81xx_phy_clk_en_set(struct phy_device *phydev, bool enable)
{
	int ret;

	ret = qca81xx_soc_modify(phydev, GCC_E2S_GEPHY_TX_CBCR,
		GCC_CLK_ENABLE, enable ? GCC_CLK_ENABLE : 0);
	if (ret < 0)
		return ret;

	return qca81xx_soc_modify(phydev, GCC_E2S_GEPHY_RX_CBCR,
		GCC_CLK_ENABLE, enable ? GCC_CLK_ENABLE : 0);
}

static int qca81xx_phy_txclk_reset_update(struct phy_device *phydev,
	bool assert)
{
	return qca81xx_soc_modify(phydev, GCC_E2S_GEPHY_TX_CBCR, GCC_CLK_ARES,
		assert ? GCC_CLK_ARES : 0);
}

static int qca81xx_phy_rxclk_reset_update(struct phy_device *phydev,
	bool assert)
{
	return qca81xx_soc_modify(phydev, GCC_E2S_GEPHY_RX_CBCR, GCC_CLK_ARES,
		assert ? GCC_CLK_ARES : 0);
}

static int qca81xx_phy_clk_reset_update(struct phy_device *phydev,
	bool assert)
{
	int ret;

	ret = qca81xx_phy_txclk_reset_update(phydev, assert);
	if (ret < 0)
		return ret;

	return qca81xx_phy_rxclk_reset_update(phydev, assert);
}

static int qca81xx_phy_clk_reset(struct phy_device *phydev)
{
	int ret;

	ret = qca81xx_phy_clk_reset_update(phydev, true);
	if (ret < 0)
		return ret;
	mdelay(1);

	return qca81xx_phy_clk_reset_update(phydev, false);
}

static int qca81xx_phy_sysclk_reset_update(struct phy_device *phydev,
	bool assert)
{
	return qca81xx_soc_modify(phydev, GCC_GEPHY_SYS_CBCR, GCC_CLK_ARES,
		assert ? GCC_CLK_ARES : 0);
}

static int qca81xx_phy_sysclk_reset(struct phy_device *phydev)
{
	int ret;

	ret = qca81xx_phy_sysclk_reset_update(phydev, true);
	if (ret < 0)
		return ret;
	mdelay(1);

	return qca81xx_phy_sysclk_reset_update(phydev, false);
}

static int qca81xx_phy_speed_clk_set(struct phy_device *phydev)
{
	int ret, div0, div1;

	switch (phydev->speed) {
	case SPEED_100:
		/* 312.5 divided by 2.5*5 */
		div0 = 4;
		div1 = 4;
		break;
	case SPEED_1000:
		/* 312.5 divided by 2.5*1 */
		div0 = 4;
		div1 = 0;
		break;
	case SPEED_2500:
		/* 312.5 divided by 1*4 */
		div0 = 1;
		div1 = 3;
		break;
	case SPEED_5000:
		/* 312.5 divided by 1*2 */
		div0 = 1;
		div1 = 1;
		break;
	case SPEED_10000:
		/* 312.5 divided by 1*1 */
		div0 = 1;
		div1 = 0;
		break;
	default:
		break;
	}

	ret = qca81xx_soc_modify(phydev, GCC_E2S_TX_CFG_RCGR,
		SRC_DIV_MASK, div0);
	if (ret < 0)
		return ret;
	ret = qca81xx_soc_modify(phydev, GCC_E2S_TX_DIV_CDIVR,
		CLK_DIV_MASK, div1);
	if (ret < 0)
		return ret;
	ret = qca81xx_soc_modify(phydev, GCC_E2S_TX_CMD_RCGR,
		CLK_CMD_UPDATE, CLK_CMD_UPDATE);
	if (ret < 0)
		return ret;

	ret = qca81xx_soc_modify(phydev, GCC_E2S_RX_CFG_RCGR,
		SRC_DIV_MASK, div0);
	if (ret < 0)
		return ret;
	ret = qca81xx_soc_modify(phydev, GCC_E2S_RX_DIV_CDIVR,
		CLK_DIV_MASK, div1);
	if (ret < 0)
		return ret;

	return qca81xx_soc_modify(phydev, GCC_E2S_RX_CMD_RCGR,
		CLK_CMD_UPDATE, CLK_CMD_UPDATE);
}

static int qca81xx_pcs_eee_enable(struct phy_device *phydev)
{
	int ret = 0;

	/*Configure the EEE related timer*/
	ret = qca81xx_pcs_modify_mmd(phydev,
		MDIO_MMD_PCS, QCA81XX_PCS_MMD3_EEE_MODE_CTRL,
		0xf40,
		QCA81XX_PCS_MMD3_EEE_RES_REGS |
		QCA81XX_PCS_MMD3_EEE_SIGN_BIT_REGS);
	if (ret < 0)
		return ret;

	ret = qca81xx_pcs_modify_mmd(phydev,
		MDIO_MMD_PCS, QCA81XX_PCS_MMD3_EEE_TX_TIMER,
		0x1fff,
		QCA81XX_PCS_MMD3_EEE_TSL_REGS|
		QCA81XX_PCS_MMD3_EEE_TLU_REGS |
		QCA81XX_PCS_MMD3_EEE_TWL_REGS);
	if (ret < 0)
		return ret;

	ret = qca81xx_pcs_modify_mmd(phydev,
		MDIO_MMD_PCS, QCA81XX_PCS_MMD3_EEE_RX_TIMER,
		0x1fff,
		QCA81XX_PCS_MMD3_EEE_100US_REG_REGS|
		QCA81XX_PCS_MMD3_EEE_RWR_REG_REGS);
	if (ret < 0)
		return ret;

	/*enable TRN_LPI*/
	ret = qca81xx_pcs_modify_mmd(phydev,
		MDIO_MMD_PCS, QCA81XX_PCS_MMD3_EEE_MODE_CTRL1,
		0x101,
		QCA81XX_PCS_MMD3_EEE_TRANS_LPI_MODE |
		QCA81XX_PCS_MMD3_EEE_TRANS_RX_LPI_MODE);
	if (ret < 0)
		return ret;

	/*enable TX/RX LPI pattern*/
	ret = qca81xx_pcs_modify_mmd(phydev, MDIO_MMD_PCS,
		QCA81XX_PCS_MMD3_EEE_MODE_CTRL, 0x3,
		QCA81XX_PCS_MMD3_EEE_EN);

	return ret;
}

static int qca81xx_phy_soft_reset(struct phy_device *phydev)
{
	int ret;

	ret = phy_modify_mmd_changed(phydev, MDIO_MMD_VEND2,
		QCA81XX_SMART_SPEED,
		QCA81XX_AUTO_SOFT_RESET,
		QCA81XX_AUTO_SOFT_RESET);
	if (ret < 0)
		return ret;

	genphy_c45_pma_suspend(phydev);
	mdelay(10);
	genphy_c45_pma_resume(phydev);

	return 0;
}

static int qca81xx_pcs_usxgmii_init(struct phy_device *phydev)
{
	int ret = 0;
	u16 phy_data = 0;

	ret = qca81xx_phy_clk_en_set(phydev, false);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_clk_en_set(phydev, false);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_sysclk_en_set(phydev, true);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_sysclk_reset(phydev);
	if (ret < 0)
		return ret;
	ret = qca81xx_xpcs_clk_reset_update(phydev, true);
	if (ret < 0)
		return ret;
	/* optional, would settle after SOD VI, write 1 to */
	/* CSR0 MMD1_reg0x7c[3],to invert pcs txclk */
	ret = qca81xx_pcs_modify_mmd(phydev,
		MDIO_MMD_PMAPMD, QCA81XX_PCS_MMD1_MODE_CTRL,
		QCA81XX_PCS_MMD1_MODE_MASK,
		QCA81XX_PCS_MMD1_XPCS_MODE);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_clk_reset(phydev);
	if (ret < 0)
		return ret;
	ret = qca81xx_phy_clk_reset(phydev);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_modify(phydev,
		QCA81XX_PCS_PLL_POWER_ON_AND_RESET,
		QCA81XX_PCS_ANA_SOFT_RESET_MASK,
		QCA81XX_PCS_ANA_SOFT_RESET);
	if (ret < 0)
		return ret;
	mdelay(1);
	ret |= qca81xx_pcs_modify(phydev,
		QCA81XX_PCS_PLL_POWER_ON_AND_RESET,
		QCA81XX_PCS_ANA_SOFT_RESET_MASK,
		QCA81XX_PCS_ANA_SOFT_RELEASE);
	if (ret < 0)
		return ret;
	ret = read_poll_timeout(qca81xx_pcs_read_mmd,
		phy_data, (phy_data & QCA81XX_PCS_MMD1_CALIBRATION_DONE),
		1000, 100000, true, phydev, MDIO_MMD_PMAPMD,
		QCA81XX_PCS_MMD1_CALIBRATION4);
	if (ret < 0)
		phydev_warn(phydev, "PCS callibration time out!\n");
	ret = qca81xx_pcs_modify_mmd(phydev,
		MDIO_MMD_PMAPMD, QCA81XX_PCS_MMD1_CDA_CONTROL1,
		QCA81XX_PCS_MMD1_SSCG_ENABLE,
		QCA81XX_PCS_MMD1_SSCG_ENABLE);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_txclk_en_set(phydev, true);
	if (ret < 0)
		return ret;
	ret = qca81xx_xpcs_clk_reset_update(phydev, false);
	if (ret < 0)
		return ret;
	ret = qca81xx_phy_soft_reset(phydev);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_modify_mmd(phydev,
		MDIO_MMD_PCS, QCA81XX_PCS_MMD3_PCS_CTRL2,
		QCA81XX_PCS_MMD3_PCS_TYPE_MASK,
		QCA81XX_PCS_MMD3_PCS_TYPE_10GBASE_R);
	if (ret < 0)
		return ret;
	ret = read_poll_timeout(qca81xx_pcs_read_mmd,
		phy_data, ((phy_data & QCA81XX_PCS_MMD3_10GBASE_R_UP)),
		10000, 500000, true, phydev, MDIO_MMD_PCS,
		QCA81XX_PCS_MMD3_10GBASE_R_PCS_STATUS1);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_modify_mmd(phydev, MDIO_MMD_PCS,
		QCA81XX_PCS_MMD3_DIG_CTRL1,
		QCA81XX_PCS_MMD3_USXGMII_EN,
		QCA81XX_PCS_MMD3_USXGMII_EN);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_modify_mmd(phydev,
		MDIO_MMD_PCS, QCA81XX_PCS_MMD3_DIG_CTRL1,
		QCA81XX_PCS_MMD3_XPCS_SOFT_RESET,
		QCA81XX_PCS_MMD3_XPCS_SOFT_RESET);
	if (ret < 0)
		return ret;
	ret = read_poll_timeout(qca81xx_pcs_read_mmd, phy_data,
		(!(phy_data & QCA81XX_PCS_MMD3_XPCS_SOFT_RESET)),
		1000, 100000, true, phydev, MDIO_MMD_PCS,
		QCA81XX_PCS_MMD3_DIG_CTRL1);
	if (ret < 0) {
		phydev_err(phydev, "xpcs software reset timeout\n");
		return ret;
	}
	ret = qca81xx_pcs_modify_mmd(phydev,
		MDIO_MMD_VEND2, QCA81XX_PCS_MMD31_MII_AN_INT_MSK,
		0x109,
		QCA81XX_PCS_MMD31_AN_COMPLETE_INT |
		QCA81XX_PCS_MMD31_MII_4BITS_CTRL |
		QCA81XX_PCS_MMD31_TX_CONFIG_CTRL);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_modify_mmd(phydev,
		MDIO_MMD_VEND2, QCA81XX_PCS_MMD31_MII_DIG_CTRL, BIT(0),
		QCA81XX_PCS_MMD31_PHY_MODE_CTRL_EN);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_modify_mmd(phydev,
		MDIO_MMD_VEND2, QCA81XX_PCS_MMD31_MII_CTRL,
		QCA81XX_PCS_MMD31_MII_AN_ENABLE,
		QCA81XX_PCS_MMD31_MII_AN_ENABLE);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_eee_enable(phydev);

	return ret;
}

/* adjust the AFE DAC parameter and the CDT threshold for cable */
/* status precision such as open, short, normal */
static int qca81xx_phy_cdt_thresh_init(struct phy_device *phydev)
{
	int ret = 0;

	ret = phy_write_mmd(phydev, MDIO_MMD_PCS,
		QCA81XX_MMD3_CDT_THRESH_CTRL2,
		QCA81XX_MMD3_CDT_THRESH_CTRL2_VAL);
	if (ret < 0)
		return ret;
	ret = phy_write_mmd(phydev, MDIO_MMD_PCS,
		QCA81XX_MMD3_CDT_THRESH_CTRL3,
		QCA81XX_MMD3_CDT_THRESH_CTRL3_VAL);
	if (ret < 0)
		return ret;
	ret = phy_write_mmd(phydev, MDIO_MMD_PCS,
		QCA81XX_MMD3_CDT_THRESH_CTRL4,
		QCA81XX_MMD3_CDT_THRESH_CTRL4_VAL);
	if (ret < 0)
		return ret;
	ret = phy_write_mmd(phydev, MDIO_MMD_PCS,
		QCA81XX_MMD3_CDT_THRESH_CTRL5,
		QCA81XX_MMD3_CDT_THRESH_CTRL5_VAL);
	if (ret < 0)
		return ret;
	ret = phy_write_mmd(phydev, MDIO_MMD_PCS,
		QCA81XX_MMD3_CDT_THRESH_CTRL6,
		QCA81XX_MMD3_CDT_THRESH_CTRL6_VAL);
	if (ret < 0)
		return ret;
	ret = phy_write_mmd(phydev, MDIO_MMD_PCS,
		QCA81XX_MMD3_CDT_THRESH_CTRL7,
		QCA81XX_MMD3_CDT_THRESH_CTRL7_VAL);
	if (ret < 0)
		return ret;
	ret = phy_write_mmd(phydev, MDIO_MMD_PCS,
		QCA81XX_MMD3_CDT_THRESH_CTRL9,
		QCA81XX_MMD3_CDT_THRESH_CTRL9_VAL);
	ret = phy_write_mmd(phydev, MDIO_MMD_PCS,
		QCA81XX_MMD3_CDT_THRESH_CTRL13,
		QCA81XX_MMD3_CDT_THRESH_CTRL13_VAL);
	if (ret < 0)
		return ret;
	ret = phy_write_mmd(phydev, MDIO_MMD_PCS,
		QCA81XX_MMD3_CDT_THRESH_CTRL14,
		QCA81XX_MMD3_CDT_THRESH_CTRL14_VAL);

	return ret;
}

static int qca81xx_phy_gcc_pre_init(struct phy_device *phydev)
{
	int ret;

	/* gephy system reset and release */
	/* gephy system clock is enabled in default */
	ret = qca81xx_phy_sysclk_reset(phydev);
	if (ret < 0)
		return ret;
	/* enable efuse loading into analog circuit */
	ret = qca81xx_soc_modify(phydev, EPHY_CFG, EPHY_LDO_CTRL, 0);
	mdelay(10);

	return ret;
}

static int qca81xx_phy_gcc_post_init(struct phy_device *phydev)
{
	int ret;

	/* security control clock switch as 25M */
	ret = qca81xx_soc_modify(phydev, GCC_SEC_CTRL_CFG_RCGR,
		GCC_E2S_SRC_MASK | SRC_DIV_MASK, 0x3);
	if (ret < 0)
		return ret;
	ret = qca81xx_soc_modify(phydev, GCC_SEC_CTRL_CMD_RCGR,
		CLK_CMD_UPDATE, CLK_CMD_UPDATE);
	if (ret < 0)
		return ret;

	/*select uphy rx, ephy tx clock source as srds_rxclk*/
	ret = qca81xx_soc_modify(phydev, GCC_E2S_TX_CFG_RCGR,
		GCC_E2S_SRC_MASK, GCC_E2S_SRC4_SRDS_RXCLK << 8);
	if (ret < 0)
		return ret;

	/*select uphy tx, ephy rx clock source as srds_txclk*/
	ret = qca81xx_soc_modify(phydev, GCC_E2S_RX_CFG_RCGR,
		GCC_E2S_SRC_MASK, GCC_E2S_SRC3_SRDS_TXCLK << 8);

	return ret;
}

/* Fix some chip can not link to 10G automatically with long cable */
static int qca81xx_phy_afe_dac_config_init(struct phy_device *phydev)
{
	int ret = 0;

	ret = qca81xx_phy_debug_write(phydev, QCA81XX_ANA_DEBUG_AFE_DAC8_DP,
		QCA81XX_ANA_DEBUG_AFE_DAC8_DP_VAL);
	if (ret < 0)
		return ret;
	ret = qca81xx_phy_debug_write(phydev, QCA81XX_ANA_DEBUG_AFE_DAC9_DP,
		QCA81XX_ANA_DEBUG_AFE_DAC9_DP_VAL);
	if (ret < 0)
		return ret;
	ret = qca81xx_phy_debug_write(phydev, QCA81XX_ANA_DEBUG_AFE_DAC38_DP,
		QCA81XX_ANA_DEBUG_AFE_DAC38_DP_VAL);
	if (ret < 0)
		return ret;
	ret = qca81xx_phy_debug_write(phydev, QCA81XX_ANA_DEBUG_AFE_DAC39_DP,
		QCA81XX_ANA_DEBUG_AFE_DAC39_DP_VAL);
	if (ret < 0)
		return ret;
	ret = phy_write_mmd(phydev, MDIO_MMD_PCS, QCA81XX_MMD3_DEBUG5,
		QCA81XX_MMD3_DEBUG5_VAL);

	return ret;
}

static int qca81xx_phy_config_init(struct phy_device *phydev)
{
	int ret = 0;

	ret = qca81xx_phy_gcc_pre_init(phydev);
	if (ret < 0)
		return ret;
	ret = qca81xx_phy_afe_dac_config_init(phydev);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_usxgmii_init(phydev);
	if (ret < 0)
		return ret;
	phydev->interface = PHY_INTERFACE_MODE_USXGMII;

	ret = qca81xx_phy_gcc_post_init(phydev);
	if (ret < 0)
		return ret;
	/* configure the eee as disable, 100M, 1G, 10G is disable in default */
	/* so only need disable 2.5G 5G eee */
	ret = phy_modify_mmd_changed(phydev, MDIO_MMD_AN,
		MDIO_AN_EEE_ADV2, GENMASK(1, 0), 0);
	if (ret < 0)
		return ret;

	ret = qca81xx_phy_cdt_thresh_init(phydev);

	return ret;
}

static int qca81xx_phy_get_features(struct phy_device *phydev)
{
	int ret;

	ret = genphy_c45_pma_read_abilities(phydev);
	if (ret < 0)
		return ret;

	linkmode_clear_bit(ETHTOOL_LINK_MODE_100baseT_Half_BIT,
		phydev->advertising);
	linkmode_clear_bit(ETHTOOL_LINK_MODE_100baseT_Half_BIT,
		phydev->supported);

	return 0;
}

static int qca81xx_phy_config_aneg(struct phy_device *phydev)
{
	bool changed = false;
	u16 reg = 0;
	int ret = 0;

	if (phydev->autoneg == AUTONEG_DISABLE)
		return genphy_c45_pma_setup_forced(phydev);

	ret = genphy_c45_an_config_aneg(phydev);
	if (ret < 0)
		return ret;
	if (ret > 0)
		changed = true;

	/* Clause 45 has no standardized support for 1000BaseT, */
	/* therefore use vendor registers. */
	if (linkmode_test_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
		phydev->advertising))
		reg |= QCA81XX_ADVERTISE_1000FULL;
	ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2,
		QCA81XX_1000BASET_CONTROL,
		QCA81XX_ADVERTISE_1000FULL,
		reg);
	if (ret < 0)
		return ret;

	return genphy_c45_check_and_restart_aneg(phydev, changed);
}

static int qca81xx_phy_fifo_reset(struct phy_device *phydev,
	bool enable)
{
	u16 phy_data = 0;

	if (!enable)
		phy_data |= QCA81XX_FIFO_RESET;

	return phy_modify_mmd(phydev, MDIO_MMD_VEND2,
		QCA81XX_FIFO_CONTROL,
		QCA81XX_FIFO_RESET, phy_data);
}

static int qca81xx_phy_speed_fixup(struct phy_device *phydev)
{
	int ret = 0;
	bool port_clock_en = false;
	u16 phy_data = 0;

	read_poll_timeout(qca81xx_pcs_read_mmd, phy_data,
		((phy_data & QCA81XX_PCS_MMD31_MII_AN_COMPLETE_INT)),
		1000, 500000, true, phydev, MDIO_MMD_VEND2,
		QCA81XX_PCS_MMD31_MII_ERR_SEL);
	ret = qca81xx_pcs_modify_mmd(phydev,
		MDIO_MMD_VEND2, QCA81XX_PCS_MMD31_MII_ERR_SEL,
		QCA81XX_PCS_MMD31_MII_AN_COMPLETE_INT, 0);
	if (ret < 0)
		return ret;
	mdelay(10);
	if (phydev->link) {
		ret = qca81xx_phy_speed_clk_set(phydev);
		if (ret < 0)
			return ret;
		/*avoid garbe data transmit out, need to assert ephy tx clock*/
		qca81xx_phy_txclk_reset_update(phydev, true);
		port_clock_en = true;
	}
	ret = qca81xx_pcs_clk_en_set(phydev, port_clock_en);
	if (ret < 0)
		return ret;
	ret = qca81xx_phy_clk_en_set(phydev, port_clock_en);
	if (ret < 0)
		return ret;
	mdelay(10);

	ret = qca81xx_pcs_clk_reset(phydev);
	if (ret < 0)
		return ret;
	ret = qca81xx_phy_clk_reset(phydev);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_modify_mmd(phydev,
		MDIO_MMD_PCS, QCA81XX_PCS_MII_DIG_CTRL,
		QCA81XX_PCS_MMD3_USXG_FIFO_RESET,
		QCA81XX_PCS_MMD3_USXG_FIFO_RESET);
	if (ret < 0)
		return ret;
	ret = qca81xx_phy_fifo_reset(phydev, true);
	if (ret < 0)
		return ret;
	mdelay(1);
	if (phydev->link) {
		ret = qca81xx_phy_fifo_reset(phydev, false);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static int qca81xx_phy_read_status(struct phy_device *phydev)
{
	int ret = 0;
	unsigned old_link = 0;


	old_link = phydev->link;

	/* Clause 45 has no standardized support for 1000BaseT, */
	/* therefore use vendor registers. */
	if (phydev->autoneg == AUTONEG_ENABLE) {
		ret = phy_read_mmd(phydev, MDIO_MMD_VEND2,
			QCA81XX_1000BASET_STATUS);
		if (ret < 0)
			return ret;
		linkmode_mod_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
			 phydev->lp_advertising,
			 ret & QCA81XX_LP_ADVERTISE_1000FULL);
	}
	ret = genphy_c45_read_status(phydev);
	if (ret < 0)
		return ret;
	/* Some PHY maybe have downgrade issue and */
	/* send incorrect advertise, then the link */
	/* speed will be not correct, so need to read */
	/* link speed from the vendor register directly */
	ret = phy_read_mmd(phydev, MDIO_MMD_VEND2,
		QCA81XX_SPEC_STATUS);
	if (ret < 0)
		return ret;
	if (ret & QCA81XX_INTR_DOWNSHIFT) {
		switch (ret & QCA81XX_SS_SPEED_MASK) {
		case QCA81XX_SS_SPEED_10000:
			phydev->speed = SPEED_10000;
			break;
		case QCA81XX_SS_SPEED_5000:
			phydev->speed = SPEED_5000;
			break;
		case QCA81XX_SS_SPEED_2500:
			phydev->speed = SPEED_2500;
			break;
		case QCA81XX_SS_SPEED_1000:
			phydev->speed = SPEED_1000;
			break;
		case QCA81XX_SS_SPEED_100:
			phydev->speed = SPEED_100;
			break;
		default:
			phydev->speed = SPEED_UNKNOWN;
		}
		if (ret & QCA81XX_SS_DUPLEX_FULL)
			phydev->duplex = DUPLEX_FULL;
		else
			phydev->duplex = DUPLEX_UNKNOWN;
	}

	if (phydev->link != old_link)
		qca81xx_phy_speed_fixup(phydev);

	return 0;
}

static int qca81xx_phy_ack_interrupt(struct phy_device *phydev)
{
	int ret = 0;

	ret = phy_read_mmd(phydev, MDIO_MMD_VEND2,
		QCA81XX_INTR_STATUS);

	return (ret < 0) ? ret : 0;
}

static int qca81xx_phy_config_intr(struct phy_device *phydev)
{
	int ret = 0;
	u16 phy_data = 0;

	phy_data = phy_read_mmd(phydev, MDIO_MMD_VEND2,
		QCA81XX_INTR_MASK);

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED) {
		ret = qca81xx_phy_ack_interrupt(phydev);
		if (ret < 0)
			return ret;
		phy_data = QCA81XX_INTR_STATUS_DOWN |
			QCA81XX_INTR_STATUS_UP;
		ret = phy_write_mmd(phydev, MDIO_MMD_VEND2,
			QCA81XX_INTR_MASK, phy_data);
	} else {
		ret = phy_write_mmd(phydev, MDIO_MMD_VEND2,
			QCA81XX_INTR_MASK, 0);
		if (ret < 0)
			return ret;
		ret = qca81xx_phy_ack_interrupt(phydev);
	}

	return ret;
}

static struct phy_driver qca81xx_phy_driver[] = {
{
	PHY_ID_MATCH_EXACT(QCA8111_PHY),
	.name = "Qualcomm QCA81xx",
	.flags = PHY_POLL_CABLE_TEST,
	.config_init = qca81xx_phy_config_init,
	.get_features = qca81xx_phy_get_features,
	.config_aneg = qca81xx_phy_config_aneg,
	.config_intr = qca81xx_phy_config_intr,
	.read_status = qca81xx_phy_read_status,
	.suspend = genphy_c45_pma_suspend,
	.resume = genphy_c45_pma_resume,
	.soft_reset = qca81xx_phy_soft_reset,
},
};

module_phy_driver(qca81xx_phy_driver);
MODULE_DESCRIPTION("QCA81XX PHY Driver");
MODULE_LICENSE("Dual BSD/GPL");
