/*
 * Copyright (C) 2016 Markus Niebel <Markus.Niebel@tq-group.com>
 *
 * Configuration settings for the TQ Systems TQMa7 module.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_TQMA6UL_MB6UL_H
#define __CONFIG_TQMA6UL_MBA6UL_H

#define CONFIG_DEFAULT_FDT_FILE		"imx6ul-mba6ul.dtb"

/* FEC */
#define TQMA6UL_ENET1_PHYADDR		0x00
#define TQMA6UL_ENET2_PHYADDR		0x01
#define CONFIG_MBA6UL_ENET_RST
#define CONFIG_FEC_XCV_TYPE		RMII
#define CONFIG_ETHPRIME			"FEC"
#define CONFIG_PHY_SMSC

#define CONFIG_MXC_UART_BASE		UART1_BASE
/* TODO: for kernel command line */
#define CONFIG_CONSOLE_DEV		"ttymxc0"

#define CONFIG_CMD_GPIO

#endif /* __CONFIG_TQMA6UL_MBA6UL_H */
