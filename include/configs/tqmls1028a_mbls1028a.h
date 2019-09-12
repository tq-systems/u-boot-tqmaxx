/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2019
 *
 * Configuration settings for the TQ Systems MBLS1028A Carrier board for
 * TQMLS1028A.
 */

#ifndef __CONFIG_TQMLS1028A_MBLS1028AL_H__
#define __CONFIG_TQMLS1028A_MBLS1028AL_H__

#define RGMII_PHY_DEV_ADDR 0x00
#define PHY_MDIO_NAME "netc_mdio"

#define CLOCKGEN_I2C_BUS_NUM	0x4
#define CLOCKGEN_I2C_ADDR		0x70
#endif /* __CONFIG_TQMLS1028A_MBLS1028AL_H__ */
