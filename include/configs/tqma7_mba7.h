/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2016-2024 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel, Steffen Doster
 *
 * Configuration settings for the TQ-Systems MBa7x carrier board for
 * TQMa7x module.
 */

#ifndef __CONFIG_TQMA7_MBA7_H
#define __CONFIG_TQMA7_MBA7_H

#define CFG_MXC_UART_BASE	UART6_IPS_BASE_ADDR
#define CONSOLE_DEV		"ttymxc5"

#if (IS_ENABLED(CONFIG_USB_FUNCTION_FASTBOOT))

#define MMC_BOOT_PARTITION 1 /* 0=user 1=boot1 2=boot2 */

#define BB_ENV_SETTINGS                                   \
	"fastboot_partition_alias_all="                              \
		__stringify(CONFIG_FASTBOOT_FLASH_MMC_DEV) ":0\0"    \
	"fastboot_raw_partition_bootloader="                         \
		__stringify(TQMA7_MMC_UBOOT_SECTOR_START) " "        \
		__stringify(TQMA7_MMC_UBOOT_SECTOR_COUNT) " mmcpart "\
		__stringify(MMC_BOOT_PARTITION)"\0"                  \
	"fastbootcmd=fastboot usb 0\0"                               \
	"console=" CONSOLE_DEV "\0"                                  \
	""

#else /* (IS_ENABLED(CONFIG_USB_FUNCTION_FASTBOOT)) */

#define BB_ENV_SETTINGS \
	"console=" CONSOLE_DEV "\0"                           \
	""

#endif /* (IS_ENABLED(CONFIG_USB_FUNCTION_FASTBOOT)) */

#endif /* __CONFIG_TQMA7_MBA7_H */
