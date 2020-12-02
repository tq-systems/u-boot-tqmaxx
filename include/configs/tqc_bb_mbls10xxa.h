/* SPDX-License-Identifier: GPL-2.0+ */

/*
 * Copyright 2019 TQ-Systems GmbH
 */

#ifndef __TQC_BB_MBLS10XX_H__
#define __TQC_BB_MBLS10XX_H__

/* Fman ethernet settings */
#if defined(CONFIG_FMAN_ENET) || defined(CONFIG_FSL_MC_ENET)
#define CONFIG_PHY_TI
#define CONFIG_PHYLIB_10G

#define RGMII_PHY1_ADDR			0x0E
#define RGMII_PHY2_ADDR			0x0C
#define QSGMII_PHY1_ADDR_BASE	0x1C
#define QSGMII_PHY2_ADDR_BASE	0x00
#endif


#if defined(CONFIG_FMAN_ENET)
#define CONFIG_ETHPRIME			"FM1@DTSEC3"
#elif defined(CONFIG_FSL_MC_ENET)
#define CONFIG_ETHPRIME			"DPMAC4@rgmii-id"
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

/* I2C clock buffer on baseboard (8T49N006A) */
#define TQC_MBLS10XXA_I2C_CLKBUF_BUS_NUM  3
#define TQC_MBLS10xxA_I2C_CLKBUF_ADDR     0x6e

/* I2C XFI retimers on baseboard */
#define TQC_MBLS10XXA_I2C_RETIMER_BUS_NUM  3
#define TQC_MBLS10XXA_I2C_RETIMER_ADDRS    { 0x18, 0x19 }

#endif /* __TQC_BB_MBLS10XX_H__ */

