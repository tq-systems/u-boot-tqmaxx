/*
 * Copyright (C) 2016 Markus Niebel <Markus.Niebel@tq-group.com>
 *
 * Configuration settings for the TQ Systems MBa6UL Carrier board for TQMa6UL.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_TQMA6UL_MB6UL_H
#define __CONFIG_TQMA6UL_MBA6UL_H

#if defined(CONFIG_TQMA6UL_VARIANT_STANDARD)
#define CONFIG_DEFAULT_FDT_FILE		"imx6ul-mba6ulx.dtb"
#elif defined(CONFIG_TQMA6UL_VARIANT_LGA)
#define CONFIG_DEFAULT_FDT_FILE		"imx6ul-mba6ulx-lga.dtb"
#else
#error
#endif

#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */

#define CONFIG_PCA953X
#define CONFIG_SYS_I2C_PCA953X_WIDTH	{ {0x20, 8}, {21, 8}, {22, 8} }
#define CONFIG_CMD_PCA953X
#define CONFIG_CMD_PCA953X_INFO

/* FEC */
#define CONFIG_FEC_MXC_MDIO_BASE	ENET2_BASE_ADDR
#define TQMA6UL_ENET1_PHYADDR		0x00
#define TQMA6UL_ENET2_PHYADDR		0x01
#define CONFIG_FEC_XCV_TYPE		RMII
#define CONFIG_ETHPRIME			"FEC"
#define CONFIG_PHY_SMSC

#define CONFIG_MXC_UART_BASE		UART1_BASE
/* TODO: for kernel command line */
#define CONFIG_CONSOLE_DEV		"ttymxc0"

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

#endif /* __CONFIG_TQMA6UL_MBA6UL_H */
