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


#endif /* __TQMLS1046A_BB_MBLS10XX_H__ */

