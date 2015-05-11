/*
 * SMSC PHY drivers
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Base code from drivers/net/phy/davicom.c
 *   Copyright 2010-2011 Freescale Semiconductor, Inc.
 *   author Andy Fleming
 *
 * Some code copied from linux kernel
 * Copyright (c) 2006 Herbert Valerio Riedel <hvr@gnu.org>
 */
#include <miiphy.h>

/* This code does not check the partner abilities. */
static int smsc_parse_status(struct phy_device *phydev)
{
	int mii_reg;

	mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMSR);

	if (mii_reg & (BMSR_100FULL | BMSR_100HALF))
		phydev->speed = SPEED_100;
	else
		phydev->speed = SPEED_10;

	if (mii_reg & (BMSR_10FULL | BMSR_100FULL))
		phydev->duplex = DUPLEX_FULL;
	else
		phydev->duplex = DUPLEX_HALF;

	return 0;
}

static int smsc_startup(struct phy_device *phydev)
{
	int ret;

	ret = genphy_update_link(phydev);
	if (ret)
		return ret;

	return smsc_parse_status(phydev);
}

static struct phy_driver lan8700_driver = {
	.name = "SMSC LAN8700",
	.uid = 0x0007c0c0,
	.mask = 0xffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &smsc_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver lan911x_driver = {
	.name = "SMSC LAN911x Internal PHY",
	.uid = 0x0007c0d0,
	.mask = 0xffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &smsc_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver lan8710_driver = {
	.name = "SMSC LAN8710/LAN8720",
	.uid = 0x0007c0f0,
	.mask = 0xffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver lan8740_driver = {
	.name = "SMSC LAN8740",
	.uid = 0x0007c110,
	.mask = 0xffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver lan8742_driver = {
	.name = "SMSC LAN8742",
	.uid = 0x0007c130,
	.mask = 0xffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

/*
 * SMSC8820
 */
#define SMSC8820_ARAP_ADDR	0x007F
#define SMSC8820_ARAP_READ	0x8180

#define SMSC8820_ARAP		0x14
#define SMSC8820_ARRDP		0x15

/* Accessors to extended registers*/
int smsc8820_extended_read(struct phy_device *phydev, int regnum)
{
	if ((regnum & SMSC8820_ARAP_ADDR) != SMSC8820_ARAP_ADDR)
		return -1;
	phy_write(phydev, MDIO_DEVAD_NONE, SMSC8820_ARAP,
		  regnum | SMSC8820_ARAP_READ);
	return phy_read(phydev, MDIO_DEVAD_NONE, SMSC8820_ARRDP);
}

static int smsc8820_extread(struct phy_device *phydev,
			    int addr, int devad, int reg)
{
	return smsc8820_extended_read(phydev, reg);
};

static int smsc8820_extwrite(struct phy_device *phydev, int addr,
			     int devad, int reg, u16 val)
{
	return -1;
};

static struct phy_driver lan8820_driver = {
	.name = "SMSC LAN8820",
	.uid = 0x0007c0e0,
	.mask = 0xffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,

	.writeext = &smsc8820_extwrite,
	.readext = &smsc8820_extread,
};

int phy_smsc_init(void)
{
	phy_register(&lan8710_driver);
	phy_register(&lan911x_driver);
	phy_register(&lan8700_driver);
	phy_register(&lan8740_driver);
	phy_register(&lan8742_driver);
	phy_register(&lan8820_driver);

	return 0;
}
