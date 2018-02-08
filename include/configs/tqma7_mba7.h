/*
 * Copyright (C) 2016 Markus Niebel <Markus.Niebel@tq-group.com>
 *
 * Configuration settings for the TQ Systems TQMa7 module.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_TQMA7_MBA7_H
#define __CONFIG_TQMA7_MBA7_H

#define CONFIG_DTT_SENSORS		{ 0, 1 }

#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */

#define CONFIG_PCA953X
#define CONFIG_SYS_I2C_PCA953X_ADDR	0x20
#define CONFIG_SYS_I2C_PCA953X_WIDTH	{ {0x20, 16} }
#define CONFIG_CMD_PCA953X
#define CONFIG_CMD_PCA953X_INFO

#define CONFIG_FEC_XCV_TYPE		RGMII
#define CONFIG_ETHPRIME			"FEC"
#define CONFIG_PHY_TI

/* TODO: correct ? */
#define TQMA7_ENET1_PHYADDR		0x00
#define TQMA7_ENET2_PHYADDR		0x00

#define CONFIG_MXC_UART_BASE		UART6_IPS_BASE_ADDR
/* TODO: for kernel command line */
#define CONFIG_CONSOLE_DEV		"ttymxc5"

#define MTDIDS_DEFAULT \
	"nor0=nor0\0"

#define MTDPARTS_DEFAULT \
	"mtdparts=nor0:"                                               \
		"832k@0k(U-Boot),"                                     \
		"64k@832k(ENV1),"                                      \
		"64k@896k(ENV2),"                                      \
		"64k@960k(DTB),"                                       \
		"7M@1M(Linux),"                                        \
		"56M@8M(RootFS)"                                       \

#endif /* __CONFIG_TQMA7_MBA7_H */
