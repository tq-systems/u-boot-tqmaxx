/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 TQ Systems GmbH
 */

#include <common.h>
#include <asm/io.h>
#include <miiphy.h>
#include "tqmls1028a_bb.h"

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
