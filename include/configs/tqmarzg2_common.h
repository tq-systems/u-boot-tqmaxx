/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * include/configs/tqmrzg2_common.h
 *	Configuration settings for the TQ Systems TQMaRZG2x modules on
 *	MBaRZG2x starter kit
 *
 * Copyright (C) 2019 Renesas Electronics Corporation
 * Copyright (C) 2021 TQ-Systems GmbH
 */

#ifndef __TQMARZG2X_MBARZG2X_H
#define __TQMARZG2X_MBARZG2X_H

#include "rcar-gen3-common.h"

/* Ethernet RAVB */
#define CONFIG_BITBANGMII
#define CONFIG_BITBANGMII_MULTI

/* Generic Timer Definitions (use in assembler source) */
#define COUNTER_FREQUENCY	0x17D7840	/* 25.00MHz from CPclk */

/* Environment in eMMC, at the end of 2nd "boot sector" */
#define CONFIG_ENV_OFFSET		(-CONFIG_ENV_SIZE)
#define CONFIG_SYS_MMC_ENV_DEV		1
#define CONFIG_SYS_MMC_ENV_PART		2

#define CONFIG_BOARD_LATE_INIT

#define CONFIG_SYS_EEPROM_BUS_NUM		7
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x57
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1

#define MAX_UBOOT_SIZE			0x300000
#define MMC_UBOOT_OFFSET		0x0		/* Blocks */

/* override bootcmd from rcar-gen3-common.h */

#undef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND	\
	"run mmcargs; " \
	"fatload mmc ${mmcblkdev}:${bootpart} ${kernel_addr_r} ${kernel};" \
	"fatload mmc ${mmcblkdev}:${bootpart} ${fdt_addr_r} ${fdt_file};" \
	"booti ${kernel_addr_r} - ${fdt_addr_r}"

#define EXTRA_ENV_SETTINGS						\
	"kernel_addr_r=0x48080000\0"					\
	"fdt_addr_r=0x48000000\0"					\
	"load_addr=0x58000000\0"					\
	"bootpart=1\0"							\
	"mmcdev_sdhc=0\0"						\
	"mmcdev_emmc=1\0"						\
	"kernel=Image\0"						\
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE ".dtb\0"			\
	"rootfsmode=rw\0"						\
	"mmcblkdev=0\0"							\
	"mmcrootpart=2\0"						\
	"mmcargs=setenv bootargs root=/dev/mmcblk${mmcblkdev}p${mmcrootpart} " \
		"${rootfsmode} rootfstype=ext4 rootwait\0"		\
	"uboot=uboot.bin\0"						\
	"uboot_mmc_offset=" __stringify(MMC_UBOOT_OFFSET) "\0"		\
	"uboot_max_size=" __stringify(MAX_UBOOT_SIZE) "\0"		\
	"update_uboot_mmc=run set_getcmd; if ${get_cmd} ${uboot}; then " \
		"if itest ${filesize} > 0; then "			\
			"mmc dev ${mmcdev_emmc} 2; mmc rescan; "	\
			"setexpr blkc ${filesize} + 0x1ff; "		\
			"setexpr blkc ${blkc} / 0x200; "		\
			"if itest ${filesize} <= ${uboot_max_size}; then " \
				"mmc write ${load_addr} ${uboot_mmc_offset} ${blkc}; " \
			"fi; "						\
			"mmc dev ${mmcdev_emmc}; mmc rescan; "		\
		"fi; fi; "						\
		"setenv filesize;\0"					\
	"update_fdt_mmc=run set_getcmd; if ${get_cmd} ${fdt_file}; then " \
		"if itest ${filesize} > 0; then "			\
			"fatwrite mmc ${mmcdev_emmc}:${bootpart} ${load_addr} ${fdt_file} ${filesize};" \
		"fi; fi; "						\
		"setenv filesize;\0"					\
	"update_kernel_mmc=run set_getcmd; if ${get_cmd} ${kernel}; then " \
		"if itest ${filesize} > 0; then "			\
			"fatwrite mmc ${mmcdev_emmc}:${bootpart} ${load_addr} ${kernel} ${filesize};" \
		"fi; fi; "						\
		"setenv filesize;\0"					\
	"update_fdt_sd=run set_getcmd; if ${get_cmd} ${fdt_file}; then " \
		"if itest ${filesize} > 0; then "			\
			"fatwrite mmc ${mmcdev_sdhc}:${bootpart} ${load_addr} ${fdt_file} ${filesize};" \
		"fi; fi; "						\
		"setenv filesize;\0"					\
	"update_kernel_sd=run set_getcmd; if ${get_cmd} ${kernel}; then " \
		"if itest ${filesize} > 0; then "			\
			"fatwrite mmc ${mmcdev_sdhc}:${bootpart} ${load_addr} ${kernel} ${filesize};" \
		"fi; fi; "						\
		"setenv filesize;\0"					\
	"set_getcmd=if test \"${ip_dyn}\" = yes; then "			\
			"setenv get_cmd dhcp; "				\
		"else "							\
			"setenv get_cmd tftp; "				\
		"fi; \0"

#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"usb_pgood_delay=2000\0"		\
	EXTRA_ENV_SETTINGS			\
	"fdt_high=0xffffffffffffffff\0"		\
	"initrd_high=0xffffffffffffffff\0"	\

#endif /*__TQMARZG2X_MBARZG2X_H */
