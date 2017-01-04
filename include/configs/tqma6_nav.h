/*
 * Copyright (C) 2017 Markus Niebel <Markus.Niebel@tq-group.com>
 *
 * Configuration settings for the TQ Systems TQMa6D module on NAV baseboard.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_TQMA6_NAV_H
#define __CONFIG_TQMA6_NAV_H

#if defined(CONFIG_TQMA6S) || defined(CONFIG_TQMA6DL)
#error
#elif defined(CONFIG_TQMA6Q)
#define CONFIG_DEFAULT_FDT_FILE		"imx6q-nav.dtb"
#endif

#define CONFIG_FEC_XCV_TYPE		RMII
#define CONFIG_ETHPRIME			"FEC"

#define CONFIG_FEC_MXC_PHYADDR		0x0
#define CONFIG_PHY_SMSC

#define CONFIG_MXC_UART_BASE		UART2_BASE
#define CONFIG_CONSOLE_DEV		"ttymxc1"

/*
 * Remove all unused interfaces / commands that are defined in
 * the common header tqms6.h
 */
#undef CONFIG_CMD_SF
#undef CONFIG_CMD_SPI
#undef CONFIG_MXC_SPI

#endif /* __CONFIG_TQMA6_NAV_H */
