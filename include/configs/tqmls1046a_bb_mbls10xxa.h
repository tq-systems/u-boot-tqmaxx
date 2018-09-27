/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 TQ-Systems GmbH
 */

#ifndef __TQMLS1046A_BB_MBLS10XX_H__
#define __TQMLS1046A_BB_MBLS10XX_H__

/* Fman ethernet settings */
#ifdef CONFIG_FMAN_ENET
#define CONFIG_PHY_TI
#define CONFIG_PHYLIB_10G

#define RGMII_PHY1_ADDR			0x0E
#define RGMII_PHY2_ADDR			0x0C
#define SGMII_PHY1_ADDR			0x3
#define SGMII_PHY2_ADDR			0x4

#define FM1_10GEC1_PHY_ADDR		0x0

#define CONFIG_ETHPRIME			"FM1@DTSEC3"
#endif

/* I2C bus muxing */
#define CONFIG_SYS_I2C_MAX_HOPS		1
#define CONFIG_SYS_NUM_I2C_BUSES	6
#define CONFIG_SYS_I2C_BUSES	{ \
				{0, {I2C_NULL_HOP} }, \
				{3, {I2C_NULL_HOP} }, \
				{3, {{I2C_MUX_PCA9544, 0x70, 0} } }, \
				{3, {{I2C_MUX_PCA9544, 0x70, 1} } }, \
				{3, {{I2C_MUX_PCA9544, 0x70, 2} } }, \
				{3, {{I2C_MUX_PCA9544, 0x70, 3} } }, \
				}

/* I2C IO expanders on baseboard (PCA9555) */
#define TQC_MBLS10XXA_I2C_GPIO_BUS_NUM    2
#define CONFIG_PCA953X
#define CONFIG_SYS_I2C_PCA953X_WIDTH    { {0x20, 16}, {0x21, 16}, {0x22, 16} }

#endif /* __TQMLS1046A_BB_MBLS10XX_H__ */

