/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 TQ Systems GmbH
 */

#include <common.h>
#include <asm/io.h>
#include <miiphy.h>
#include "tqmls1028a_bb.h"
#include <i2c.h>

int checkboard(void)
{
	printf("Board: MBLS1028A Booting from: ");
#ifdef CONFIG_SD_BOOT
	puts("SD\n");
#elif CONFIG_EMMC_BOOT
	puts("EMMC\n");
#else
	puts("\n");
#endif

	return 0;
}

static int clockgen_init(void)
{
	int ret;

	ret = i2c_set_bus_num(CLOCKGEN_I2C_BUS_NUM);

	if (ret)
		return ret;

	u8 regvals[][2] = {
		{0x6, 0x1d},
		{0x2f, 0x14},
		{0x6a, 0x80},
		{0x74, 0x80},
		{0x1e, 0xb0},
		{0x1d, 0x90},
		{0x24, 0x7},
		{0x25, 0x7},
		{0x28, 0xe7},
		{0x26, 0x7},
		{0x29, 0x1c},
		{0x27, 0x6},
		{0x23, 0xaa},
		{0x2a, 0x24},
		{0x62, 0x30},
		{0x67, 0x1},
		{0x35, 0x80},
		{0x36, 0xa},
		{0x3b, 0x1},
		{0x1c, 0x16},
		{0x30, 0x3a},
		{0x33, 0x7},
		{0x32, 0xc4},
		{0x1f, 0xc0},
		{0x40, 0x80},
		{0x41, 0xa},
		{0x42, 0x54},
		{0x46, 0x1},
		{0x20, 0xc0},
		{0x4b, 0x80},
		{0x4c, 0xa},
		{0x51, 0x1},
		{0x21, 0xc0},
		{0x57, 0x8},
		{0x5c, 0x1},
		{0x22, 0xc0},
		{0xf1, 0x65},
		{0x2d, 0xee},
		{0x2e, 0x1},
		{0x31, 0x80},
	};

	for (int i = 0; i < ARRAY_SIZE(regvals); i++)
		i2c_reg_write(CLOCKGEN_I2C_ADDR, regvals[i][0], regvals[i][1]);

	return 0;
}

int board_bb_init(void)
{
	/* should be called after clockgen_init */
	return clockgen_init();
}

#define PCS_INF(fmt, args...)  printf("PCS: " fmt, ##args)
#define PCS_ERR(fmt, args...)  printf("PCS: " fmt, ##args)

static uint16_t _rgmii_phy_read_indirect(struct mii_dev *ext_bus, uint8_t addr)
{
	if (ext_bus)
		return 0;

	ext_bus->write(ext_bus, RGMII_PHY_DEV_ADDR, MDIO_DEVAD_NONE,
		       0x0d, 0x001f);
	ext_bus->write(ext_bus, RGMII_PHY_DEV_ADDR, MDIO_DEVAD_NONE,
		       0x0e, addr);
	ext_bus->write(ext_bus, RGMII_PHY_DEV_ADDR, MDIO_DEVAD_NONE,
		       0x0d, 0x401f);
	return ext_bus->read(ext_bus, RGMII_PHY_DEV_ADDR, MDIO_DEVAD_NONE,
			     0x0e);
}

static void _rgmii_phy_write_indirect(struct mii_dev *ext_bus, uint8_t addr,
				      uint16_t value)
{
	if (ext_bus)
		return;

	ext_bus->write(ext_bus, RGMII_PHY_DEV_ADDR, MDIO_DEVAD_NONE,
		       0x0d, 0x001f);
	ext_bus->write(ext_bus, RGMII_PHY_DEV_ADDR, MDIO_DEVAD_NONE,
		       0x0e, addr);
	ext_bus->write(ext_bus, RGMII_PHY_DEV_ADDR, MDIO_DEVAD_NONE,
		       0x0d, 0x401f);
	ext_bus->write(ext_bus, RGMII_PHY_DEV_ADDR, MDIO_DEVAD_NONE,
		       0x0e, value);
}

static void setup_RGMII(void)
{
	#define NETC_PF1_BAR0_BASE	0x1f8050000
	#define NETC_PF1_ECAM_BASE	0x1F0001000
	struct mii_dev *ext_bus;
	char *mdio_name = RGMII_MDIO_NAME;
	int val;

	PCS_INF("trying to set up RGMII\n");

	/* turn on PCI function */
	out_le16(NETC_PF1_ECAM_BASE + 4, 0xffff);
	out_le32(NETC_PF1_BAR0_BASE + 0x8300, 0x8006);

	ext_bus = miiphy_get_dev_by_name(mdio_name);
	if (!ext_bus) {
		PCS_ERR("couldn't find MDIO bus, ignoring the PHY\n");
		return;
	}

	/* Set RGMII PHY LEDs Led1 and Led2 to active low */
	val = ext_bus->read(ext_bus, RGMII_PHY_DEV_ADDR, MDIO_DEVAD_NONE, 0x19);
	val &= 0xFBBF;
	ext_bus->write(ext_bus, RGMII_PHY_DEV_ADDR, MDIO_DEVAD_NONE, 0x19, val);

	/* enable RGMII delay in both directions */
	val = _rgmii_phy_read_indirect(ext_bus, 0x32);
	val |= 0x0003;
	_rgmii_phy_write_indirect(ext_bus, 0x32, val);
	val = _rgmii_phy_read_indirect(ext_bus, 0x32);

	/* set RGMII delay in both directions to 1,5ns */
	val = _rgmii_phy_read_indirect(ext_bus, 0x86);
	val = (val & 0xFF00) | 0x0055;
	_rgmii_phy_write_indirect(ext_bus, 0x86, val);
	val = _rgmii_phy_read_indirect(ext_bus, 0x86);
}

void tqmls1028a_bb_late_init(void)
{
	setup_RGMII();
}
