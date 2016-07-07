/*
 * Copyright (C) 2016 Markus Niebel <Markus.Niebel@tq-group.com>
 *
 * Configuration settings for the TQ Systems TQMa7 module.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_TQMA7_MBA7_H
#define __CONFIG_TQMA7_MBA7_H

#define CONFIG_DEFAULT_FDT_FILE		"imx7d-mba7.dtb"

#define CONFIG_DTT_SENSORS		{ 0, 1 }

#define CONFIG_FEC_XCV_TYPE		RGMII
#define CONFIG_ETHPRIME			"FEC"
#define CONFIG_PHY_TI

/* TODO: correct ? */
#define TQMA7_ENET1_PHYADDR		0x00
#define TQMA7_ENET2_PHYADDR		0x00


#define CONFIG_MXC_UART_BASE		UART6_IPS_BASE_ADDR
/* TODO: for kernel command line */
#define CONFIG_CONSOLE_DEV		"ttymxc6"

#define CONFIG_CMD_GPIO

#endif /* __CONFIG_TQMA7_MBA7_H */
