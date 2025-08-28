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

#define PHY_ANEG_TIMEOUT	8000	/* KSZ9031 PHY needs a longer aneg time */

#define CFG_MXC_UART_BASE		UART2_BASE
#define CONSOLE_DEV		"ttymxc1"

#define BOOT_ENV_MBA6 \
	"console=" CONSOLE_DEV "\0" \

#if (IS_ENABLED(CONFIG_USB_FUNCTION_FASTBOOT))

#define MMC_BOOT_PARTITION 1 /* 0=user 1=boot1 2=boot2 */

#define BB_ENV_SETTINGS                                              \
	"fastboot_partition_alias_all="                              \
		__stringify(CONFIG_FASTBOOT_FLASH_MMC_DEV) ":0\0"    \
	"fastboot_raw_partition_bootloader="                         \
		__stringify(TQMA6_MMC_UBOOT_SECTOR_START) " "        \
		__stringify(TQMA6_MMC_UBOOT_SECTOR_COUNT) " mmcpart "\
		__stringify(MMC_BOOT_PARTITION)"\0"                  \
	"fastbootcmd=fastboot usb 0\0"                               \
	BOOT_ENV_MBA6                                                \

#else

#define BB_ENV_SETTINGS \
	BOOT_ENV_MBA6   \

#endif /* IS_ENABLED(CONFIG_USB_FUNCTION_FASTBOOT) */

#endif /* __CONFIG_TQMA6_MBA6_H */
