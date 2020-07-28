// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 *
 */

#include <common.h>
#include <command.h>
#include <i2c.h>
#include <asm/gpio.h>
#include <netdev.h>
#include <malloc.h>
#include <fsl_mdio.h>
#include <miiphy.h>
#include <phy.h>
#include <fm_eth.h>
#include <asm/io.h>
#include <exports.h>
#include <asm/arch/fsl_serdes.h>
#include <fsl-mc/fsl_mc.h>
#include <fsl-mc/ldpaa_wriop.h>

DECLARE_GLOBAL_DATA_PTR;

struct mblx2160a_gpio {
	char name[20];
	char description[20];
	unsigned long flags;
	s8 initval;
};

enum mblx2160a_phys {
	ETH_NO,
	ETH_01,
	ETH_02,
	ETH_03,
	ETH_04,
	ETH_05,
	ETH_06,
	ETH_07,
	ETH_08,
	ETH_09,
	ETH_10,
	XFI_01,
	XFI_02,
	CAUI_4,
};

u8 mac_to_dpmac[] = {
	WRIOP1_DPMAC1,
	WRIOP1_DPMAC9,
	WRIOP1_DPMAC10,
	WRIOP1_DPMAC12,
	WRIOP1_DPMAC13,
	WRIOP1_DPMAC14,
	WRIOP1_DPMAC16,
	WRIOP1_DPMAC17,
	WRIOP1_DPMAC18,
};

struct mblx2160a_srds_config {
	struct {
		u32 srds_s1;
		u32 srds_s2;
		u32 srds_s3;
	} serdes_config;
	u8 macs[ARRAY_SIZE(mac_to_dpmac)];
};

struct i2c_reg_setting {
		u8 reg;
		u8 val;
		u8 mask;
};

struct phy_info {
	char *reset_name;
	u8 phy_address;
	char *mdio_bus;
};

struct phy_info phy_infos[] = {
	[ETH_01] = {"RESET_ETH1", 0x1, DEFAULT_WRIOP_MDIO1_NAME },
	[ETH_02] = {"RESET_ETH2", 0x2, DEFAULT_WRIOP_MDIO1_NAME },
	[ETH_03] = {"RESET_ETH3", 0x3, DEFAULT_WRIOP_MDIO1_NAME },
	[ETH_04] = {"RESET_ETH4", 0x4, DEFAULT_WRIOP_MDIO1_NAME },
	[ETH_05] = {"RESET_ETH5", 0x5, DEFAULT_WRIOP_MDIO1_NAME },
	[ETH_06] = {"RESET_ETH6", 0x6, DEFAULT_WRIOP_MDIO1_NAME },
	[ETH_07] = {"RESET_ETH7", 0x1, DEFAULT_WRIOP_MDIO2_NAME },
	[ETH_08] = {"RESET_ETH8", 0x2, DEFAULT_WRIOP_MDIO2_NAME },
	[ETH_09] = {"RESET_ETH9", 0x3, DEFAULT_WRIOP_MDIO2_NAME },
	[ETH_10] = {"RESET_ETH10", 0x4, DEFAULT_WRIOP_MDIO2_NAME },
};

struct mblx2160a_srds_config srds_configs[] = {
	/* SERDES  , MAC 1,  MAC 9,  MAC 10, MAC 12, MAC 13, MAC 14, MAC 16, MAC 17, MAC 18 */
	{{0, 0, 0}, {ETH_NO, ETH_NO, ETH_NO, ETH_NO, ETH_NO, ETH_NO, ETH_NO, ETH_09, ETH_10} },
	{{12, 7, 0}, {ETH_NO, ETH_07, ETH_08, ETH_01, XFI_01, XFI_02, ETH_04, ETH_02, ETH_03} },
	{{12, 8, 0}, {ETH_NO, ETH_07, ETH_08, ETH_NO, XFI_01, XFI_02, ETH_NO, ETH_09, ETH_10} },
	{{12, 11, 0}, {ETH_NO, ETH_07, ETH_08, ETH_01, ETH_05, ETH_06, ETH_04, ETH_02, ETH_03} },
	{{14, 7, 0}, {CAUI_4, ETH_07, ETH_08, ETH_01, XFI_01, XFI_02, ETH_04, ETH_02, ETH_03} },
	{{14, 11, 0}, {CAUI_4, ETH_NO, ETH_NO, ETH_01, ETH_05, ETH_06, ETH_04, ETH_02, ETH_03} },
};

struct mblx2160a_gpio mblx2160a_gpios[] = {
	{"gpio@20_0",		"QSFP_MODSEL",		GPIOD_IS_OUT, 0 },
	{"gpio@20_1",		"QSFP_RESET#",		GPIOD_IS_OUT, 1 },
	{"gpio@20_2",		"QSFP_MODPRS#",		GPIOD_IS_IN, 0 },
	{"gpio@20_3",		"QSFP_INT#",		GPIOD_IS_IN, 0 },
	{"gpio@20_4",		"QSFP_LPMOD",		GPIOD_IS_IN, 0 },
	{"gpio@20_5",		"QSFP_RETIMER_1#",	GPIOD_IS_IN, 0 },
	{"gpio@20_6",		"QSFP_RETIMER_2#",	GPIOD_IS_IN, 0 },
	{"gpio@20_7",		"MPCIE_1_WAKE#",	GPIOD_IS_OUT, 0 },
	{"gpio@20_8",		"MPCIE_1_DISABLE#",	GPIOD_IS_OUT, 0 },
	{"gpio@20_9",		"MPCIE_1_RESET#",	GPIOD_IS_OUT, 0 },
	{"gpio@20_10",		"MPCIE_2_WAKE#",	GPIOD_IS_OUT, 0 },
	{"gpio@20_11",		"MPCIE_2_DISABLE#",	GPIOD_IS_OUT, 0 },
	{"gpio@20_12",		"MPCIE_2_RESET#",	GPIOD_IS_OUT, 0 },
	{"gpio@20_13",		"EN_SILENT_CAN1",	GPIOD_IS_OUT, 0 },
	{"gpio@20_14",		"EN_SILENT_CAN2",	GPIOD_IS_OUT, 0 },
	{"gpio@20_15",		"SIM_CARD_DETECT",	GPIOD_IS_IN, 0 },

	{"gpio@21_0",		"RESET_USB_HUB#",	GPIOD_IS_OUT, 0 },
	{"gpio@21_1",		"RESET_ETH1",		GPIOD_IS_OUT, 0 },
	{"gpio@21_2",		"RESET_ETH2",		GPIOD_IS_OUT, 0 },
	{"gpio@21_3",		"RESET_ETH3",		GPIOD_IS_OUT, 0 },
	{"gpio@21_4",		"RESET_ETH4",		GPIOD_IS_OUT, 0 },
	{"gpio@21_5",		"RESET_ETH5",		GPIOD_IS_OUT, 0 },
	{"gpio@21_6",		"RESET_ETH6",		GPIOD_IS_OUT, 0 },
	{"gpio@21_7",		"RESET_ETH7",		GPIOD_IS_OUT, 0 },
	{"gpio@21_8",		"RESET_ETH8",		GPIOD_IS_OUT, 0 },
	{"gpio@21_9",		"RESET_ETH9",		GPIOD_IS_OUT, 0 },
	{"gpio@21_10",		"RESET_ETH10",		GPIOD_IS_OUT, 0 },
	{"gpio@21_11",		"CAN1_SEL",		GPIOD_IS_OUT, 1 },
	{"gpio@21_12",		"CAN2_SEL",		GPIOD_IS_OUT, 1 },
	{"gpio@21_13",		"RST_M2_SATA_1",	GPIOD_IS_OUT, 1 },
	{"gpio@21_14",		"RST_M2_SATA_2",	GPIOD_IS_OUT, 1 },
	{"gpio@21_15",		"EN_USER_LED_2",	GPIOD_IS_OUT, 1 },

	{"gpio@22_0",		"XFI1_TX_FAULT",	GPIOD_IS_IN, -1},
	{"gpio@22_1",		"XFI1_TX_DIS",		GPIOD_IS_OUT, 1},
	{"gpio@22_2",		"XFI1_MOD_DECTECT",	GPIOD_IS_IN, -1},
	{"gpio@22_3",		"XFI1_RX_LOSS",		GPIOD_IS_IN, -1},
	{"gpio@22_4",		"XFI2_TX_FAULT",	GPIOD_IS_IN, -1},
	{"gpio@22_5",		"XFI2_TX_DIS",		GPIOD_IS_OUT, 1},
	{"gpio@22_6",		"XFI2_MOD_DECTECT",	GPIOD_IS_IN, -1},
	{"gpio@22_7",		"XFI2_RX_LOSS",		GPIOD_IS_IN, -1},
	{"gpio@22_8",		"XFI1_RET_LOSS",	GPIOD_IS_IN, -1},
	{"gpio@22_9",		"XFI2_RET_LOSS",	GPIOD_IS_IN, -1},
	{"gpio@22_10",		"PCIE_1_PERST#",	GPIOD_IS_OUT, -1},
	{"gpio@22_11",		"PCIE_2_PERST#",	GPIOD_IS_OUT, -1},
	{"gpio@22_12",		"PCIE_WAKE#",		GPIOD_IS_OUT, -1},
	{"gpio@22_13",		"X8_PRSNT1#",		GPIOD_IS_IN, -1},
	{"gpio@22_14",		"X4_1_PRSNT1#",		GPIOD_IS_IN, -1},
	{"gpio@22_15",		"X4_2_PRSNT1#",		GPIOD_IS_IN, -1},
};

static int __gpio_idx_by_name(const char *name)
{
	int i;

	if ((name == NULL) || (strlen(name) <= 0))
		return -1;

	for (i = 0; i < ARRAY_SIZE(mblx2160a_gpios); i++) {
		if ((strlen(mblx2160a_gpios[i].description) == strlen(name)) &&
		   (strncmp(mblx2160a_gpios[i].description, name, strlen(name)) == 0)) {
			return i;
		}
	}

	return -1;
}

int mblx2160a_gpios_init(void)
{
	struct gpio_desc desc;
	int ret;

	for (int i = 0; i < ARRAY_SIZE(mblx2160a_gpios); i++) {
		ret = dm_gpio_lookup_name(mblx2160a_gpios[i].name, &desc);

		if (ret)
			return ret;

		dm_gpio_request(&desc, mblx2160a_gpios[i].description);

		if (ret)
			return ret;

		desc.flags = mblx2160a_gpios[i].flags;

		dm_gpio_set_dir(&desc);

		if (mblx2160a_gpios[i].flags & GPIOD_IS_OUT)
			ret = dm_gpio_set_value(&desc, mblx2160a_gpios[i].initval);

		if (ret)
			return ret;
	}

	return 0;
}

int mblx2160a_get_gpio(const char *name)
{
	int i;
	int ret;
	struct gpio_desc desc;

	i = __gpio_idx_by_name(name);

	ret = dm_gpio_lookup_name(mblx2160a_gpios[i].name, &desc);

	if (ret)
		return ret;

	desc.flags = mblx2160a_gpios[i].flags;

	return dm_gpio_get_value(&desc);
}

int mblx2160a_set_gpio(const char *name, int val)
{
	int i;
	int ret;
	struct gpio_desc desc;

	i = __gpio_idx_by_name(name);

	ret = dm_gpio_lookup_name(mblx2160a_gpios[i].name, &desc);

	if (ret) {
		printf("gpio not found %s: num: %d error: %d\n", name, i, ret);
		return ret;
	}

	desc.flags = mblx2160a_gpios[i].flags;

	return dm_gpio_set_value(&desc, val);
}

static uint16_t _dp83868_phy_read_indirect(struct phy_device *phydev,
					   u8 addr)
{
	phy_write(phydev, MDIO_DEVAD_NONE, 0x0d, 0x001f);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x0e, addr);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x0d, 0x401f);
	return phy_read(phydev, MDIO_DEVAD_NONE, 0x0e);
}

static void _dp83867_phy_write_indirect(struct phy_device *phydev,
					u8 addr, uint16_t value)
{
	phy_write(phydev, MDIO_DEVAD_NONE, 0x0d, 0x001f);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x0e, addr);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x0d, 0x401f);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x0e, value);
}

int _config_i2c_device(struct udevice *dev, struct i2c_reg_setting *settings, int count)
{
	u8 val;
	int ret;

	debug("Configuring i2c device\n");
	for (int i = 0; i < count; i++) {
		val = dm_i2c_reg_read(dev, settings[i].reg);
		if (ret)
			return ret;

		debug("Read reg %2x: val: %2x ", settings[i].reg, val);

		/* Clear bits to write */
		val &= ~settings[i].mask;
		val |= (settings[i].mask & settings[i].val);

		debug("Writing: %2x\n", val);
		ret = dm_i2c_reg_write(dev, settings[i].reg, val);

		if (ret)
			return ret;
	}
	return 0;
}

int _reconfigure_serdes_tx_lane(int serdes_nr, int lane_nr, u32 val, u32 mask)
{
	void *serdes_base;
	u32 *tx_rst_reg;
	u32 *tx_eq_reg;
	u32 hlt_req_mask = 0x08000000;
	u32 rst_req_mask = 0x80000000;
	int timeout = 100;
	u32 reg_val;

	if (serdes_nr < 1 || serdes_nr > 3 || lane_nr < 0 || lane_nr > 7)
		return -1;

	serdes_base = (void *)CONFIG_SYS_FSL_LSCH3_SERDES_ADDR + (serdes_nr - 1) * 0x10000;
	tx_rst_reg = serdes_base + 0x800 + (lane_nr * 0x100) + 0x20;
	tx_eq_reg = serdes_base + 0x800 + (lane_nr * 0x100) + 0x30;

	/* Reset Lane */
	reg_val = in_le32(tx_rst_reg);
	out_le32(tx_rst_reg, reg_val | hlt_req_mask);

	while ((in_le32(tx_rst_reg) & hlt_req_mask)) {
		if (timeout == 0) {
			printf ("Timeout waiting for halt lane %d_%d.\n", serdes_nr, lane_nr);
			return -ETIMEDOUT;
		}
		timeout--;
		mdelay(1);
	}

	/* Write Value */
	reg_val = in_le32(tx_eq_reg);
	reg_val &= ~mask;
	reg_val |= (mask & val);
	out_le32(tx_eq_reg, reg_val);

	/* Release from Rreset */
	reg_val = in_le32(tx_rst_reg);
	out_le32(tx_rst_reg, reg_val | rst_req_mask);

	return 0;
}

int xfi_config(int xfi_nr)
{
	int ret;
	struct udevice *dev;
	int bus;
	int addr;
	int lane;

	if (xfi_nr == XFI_01)
		lane = 6;
	else if (xfi_nr == XFI_02)
		lane = 7;
	else
		return -ENODEV;

	/* Configure EQ_AMP_RED */
	ret = _reconfigure_serdes_tx_lane(2, lane, 0x10820c20, 0xFFFFFFFF);

	struct i2c_reg_setting xfi_retimer_settings[] = {
		{0xff, 0x0c, 0x0c},
		{0x00, 0x04, 0x04},
		{0x0a, 0x0c, 0x0c},
		{0x2f, 0xc0, 0xf0},
		{0x31, 0x20, 0x20},
		{0x3a, 0x00, 0xff},
		{0x1e, 0x08, 0x08},
		{0x2d, 0x07, 0x07},
		{0x15, 0x00, 0x47},
		{0x0a, 0x00, 0x0c},
	};

	if (xfi_nr == XFI_01) {
		bus = I2C_XFI1_BUS;
		addr = I2C_XFI1_RETIMER_ADDR;
	} else if (xfi_nr == XFI_02) {
		bus = I2C_XFI1_BUS;
		addr = I2C_XFI1_RETIMER_ADDR;
	} else {
		printf("Error: unknown XFI device: %d\n", xfi_nr);
		return -ENODEV;
	}

	if (i2c_get_chip_for_busnum(bus, addr, 1, &dev))
		return -ENODEV;

	ret = _config_i2c_device(dev, &xfi_retimer_settings[0], ARRAY_SIZE(xfi_retimer_settings));

	if (xfi_nr == XFI_01)
		ret = mblx2160a_set_gpio("XFI1_TX_DIS", 0);
	else
		ret = mblx2160a_set_gpio("XFI2_TX_DIS", 0);

	return ret;
}

int caui4_config(int caui_nr)
{
	int ret;

	ret = _reconfigure_serdes_tx_lane(1, 4, 0x20820c20, 0xFFFFFFFF);
	ret = _reconfigure_serdes_tx_lane(1, 5, 0x20820c20, 0xFFFFFFFF);
	ret = _reconfigure_serdes_tx_lane(1, 6, 0x20820c20, 0xFFFFFFFF);
	ret = _reconfigure_serdes_tx_lane(1, 7, 0x20820c20, 0xFFFFFFFF);

	return ret;
}

int tqc_bb_board_eth_init(bd_t *bis)
{
#if defined(CONFIG_FSL_MC_ENET)
	struct mii_dev *mii_dev;
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 srds_s1, srds_s2;
	struct list_head *mii_devs, *entry;
	struct phy_device *dev;
	int i;
	int found = 0;
	uint mask;

	srds_s1 = in_le32(&gur->rcwsr[28]) &
				FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_MASK;
	srds_s1 >>= FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_SHIFT;

	srds_s2 = in_le32(&gur->rcwsr[28]) &
				FSL_CHASSIS3_RCWSR28_SRDS2_PRTCL_MASK;
	srds_s2 >>= FSL_CHASSIS3_RCWSR28_SRDS2_PRTCL_SHIFT;

	/* Reset all Phys */
	for (int i = 1; i < ARRAY_SIZE(phy_infos); i++) {
		mblx2160a_set_gpio(phy_infos[i].reset_name, 1);
	}
	/* Wait after getting all PHYs out of reset */
	mdelay(10);

	for (i = 0; i < ARRAY_SIZE(srds_configs); i++) {
		if (srds_configs[i].serdes_config.srds_s1 == srds_s1 &&
		    srds_configs[i].serdes_config.srds_s2 == srds_s2) {
			printf("Configuring Ethernet for SerDes Configuration %d_%d_xx.\n",
			       srds_s1, srds_s2);
			found = 1;

			for (int j = 0; j < ARRAY_SIZE(srds_configs[i].macs); j++) {
				if (srds_configs[i].macs[j] >= ETH_01 && srds_configs[i].macs[j] <= ETH_10) {
					wriop_set_phy_address(mac_to_dpmac[j], 0, phy_infos[srds_configs[i].macs[j]].phy_address);
					mii_dev = miiphy_get_dev_by_name(phy_infos[srds_configs[i].macs[j]].mdio_bus);
					wriop_set_mdio(mac_to_dpmac[j], mii_dev);
				} else if (srds_configs[i].macs[j] == XFI_01 || srds_configs[i].macs[j] == XFI_02) {
					xfi_config(srds_configs[i].macs[j]);
				} else if (srds_configs[i].macs[j] == CAUI_4) {
					caui4_config(srds_configs[i].macs[j]);
				}
			}
		}
	}

	if (!found)
		printf("No ethernet configuriation for Serdes  %d_%d_xx found.\n", srds_s1, srds_s2);

	/* Configure PHY LEDs for all PHYs also if not used */
	mii_devs = mdio_get_list_head();
	list_for_each(entry, mii_devs) {
		mii_dev = list_entry(entry, struct mii_dev, link);

		for (int i = 0; i < PHY_MAX_ADDR; i++) {
			dev = mii_dev->phymap[i];
			mask = (1 << i);
			dev = phy_find_by_mask(mii_dev, mask, PHY_INTERFACE_MODE_NONE);

			if (dev) {
				_dp83867_phy_write_indirect(dev, 0x18, 0x6101);
				_dp83867_phy_write_indirect(dev, 0x19, 0x4400);
			}
		}
	}
#endif /* CONFIG_FSL_MC_ENET */

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	int val;

	if (phy_interface_is_rgmii(phydev)) {
		debug("configuring rgmii phy on addr: %x\n", phydev->addr);
		val = _dp83868_phy_read_indirect(phydev, 0x32);
		val |= 0x0003;
		_dp83867_phy_write_indirect(phydev, 0x32, val);

		/* set RGMII delay in both directions to 1,5ns */
		val = _dp83868_phy_read_indirect(phydev, 0x86);
		val = (val & 0xFF00) | 0x0055;
		_dp83867_phy_write_indirect(phydev, 0x86, val);
	}

	/* Configure LEDs on PHY */
	_dp83867_phy_write_indirect(phydev, 0x18, 0x6101);
	_dp83867_phy_write_indirect(phydev, 0x19, 0x4400);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

int fdt_fixup_board_phy(void *fdt)
{
	int ret;

	ret = 0;

	return ret;
}
