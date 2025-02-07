/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2016-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel, Matthias Schiffer
 *
 * Configuration settings for the TQ-Systems MBa6ULx carrier board for
 * TQMa6UL[L]x[L] SOM family.
 */

#ifndef __CONFIG_TQMA6UL_MBA6ULXL_H
#define __CONFIG_TQMA6UL_MBA6ULXL_H

#include "tqma6ul.h"

#define CONSOLE_DEV			"ttymxc0"
#define CFG_MXC_UART_BASE		UART1_BASE
#define CFG_SYS_FSL_ESDHC_ADDR		USDHC1_BASE_ADDR

#define BOOT_ENV_MBA6ULXL                                    \
	"console=" __stringify(CONSOLE_DEV) "\0"             \
	""

#if (IS_ENABLED(CONFIG_USB_FUNCTION_FASTBOOT))

#define MMC_BOOT_PARTITION 1 /* 0=user 1=boot1 2=boot2 */

#define BB_ENV_SETTINGS                                                \
	"fastboot_partition_alias_all="                                \
		__stringify(CONFIG_FASTBOOT_FLASH_MMC_DEV) ":0\0"      \
	"fastboot_raw_partition_bootloader="                           \
		__stringify(TQMA6UL_MMC_UBOOT_SECTOR_START) " "        \
		__stringify(TQMA6UL_MMC_UBOOT_SECTOR_COUNT) " mmcpart "\
		__stringify(MMC_BOOT_PARTITION)"\0"                    \
	"fastbootcmd=fastboot usb 0\0"                                 \
	BOOT_ENV_MBA6ULXL                                              \

#else

#define BB_ENV_SETTINGS   \
	BOOT_ENV_MBA6ULXL \
	""

#endif /* IS_ENABLED(CONFIG_USB_FUNCTION_FASTBOOT) */

#endif /* __CONFIG_TQMA6UL_MBA6ULXL_H */
