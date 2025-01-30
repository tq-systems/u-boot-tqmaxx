/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2013-2023  TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 *
 * Configuration settings for the TQ-Systems TQMa6<Q,D,DL,S> module on
 * MBa6 starter kit
 */

#ifndef __CONFIG_TQMA6_MBA6_H
#define __CONFIG_TQMA6_MBA6_H

#define CFG_FEC_MXC_PHYADDR		0x03

#define CFG_MXC_UART_BASE		UART2_BASE
#define CONSOLE_DEV		"ttymxc1"

#define BOOT_ENV_MBA6 \
	"console=" CONSOLE_DEV "\0" \

#if (IS_ENABLED(CONFIG_USB_FUNCTION_FASTBOOT))

#define BB_ENV_SETTINGS                                          \
	"fastboot_partition_alias_all=0:0\0"                     \
	"fastboot_partition_alias_boot=0:1\0"                    \
	"fastboot_partition_alias_rootfs=0:2\0"                  \
	"fastboot_raw_partition_bootloader="                     \
		__stringify(TQMA6_MMC_UBOOT_SECTOR_START) " "    \
		__stringify(TQMA6_MMC_UBOOT_SECTOR_COUNT) "\0"   \
	"fastbootcmd=fastboot usb 0\0"                           \
	BOOT_ENV_MBA6                                            \

#else

#define BB_ENV_SETTINGS \
	BOOT_ENV_MBA6   \

#endif /* IS_ENABLED(CONFIG_USB_FUNCTION_FASTBOOT) */

#endif /* __CONFIG_TQMA6_MBA6_H */
