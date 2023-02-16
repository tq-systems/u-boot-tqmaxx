/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2022-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#ifndef __TQMA93XX_H
#define __TQMA93XX_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include <asm/arch/imx-regs.h>
#include "imx_env.h"

#define CFG_SYS_UBOOT_BASE	\
	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#define CFG_MFG_ENV_SETTINGS                                           \
	CFG_MFG_ENV_SETTINGS_DEFAULT                                   \
	"initrd_addr=0x83800000\0"                                     \
	"emmc_dev=0\0"                                                 \
	"sd_dev=1\0"

/* Initial environment variables */
#define CFG_MODULE_ENV_SETTINGS                                        \
	"scriptaddr=0x83500000\0"                                      \
	"image=Image\0"                                                \
	"kernel_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0"        \
	"fdt_addr_r=0x83000000\0"                                      \
	"initrd_addr=0x83800000\0"                                     \
	"mmcdev=" __stringify(CONFIG_SYS_MMC_ENV_DEV) "\0"             \
	"mmcfwpart=1\0"                                                \
	"mmcfwpath=/\0"                                                \
	"mmcautodetect=yes\0"                                          \
	"loadbootscript=mmc dev ${mmcdev}; mmc rescan;"                \
		"load mmc ${mmcdev}:${mmcfwpart} ${loadaddr} "         \
		"${mmcfwpath}${script};\0"                             \
	"bootscript=echo Running bootscript from mmc ...; "            \
		"source\0"                                             \
	"boot_os=booti ${kernel_addr_r} - ${fdt_addr_r};\0"            \
	"uboot_mmc_start=0x40\0"                                       \
	"uboot_mmc_size=0xfc0\0"                                       \
	"uboot_spi_start=0x0\0"                                        \
	"uboot_spi_size=0x400000\0"                                    \
	"uboot=bootstream.bin\0"                                       \

#if !defined(CONFIG_BOOTCOMMAND)
#warn "CONFIG_BOOTCOMMAND missing in configuration"
#endif

/* Link Definitions */
#if defined(CONFIG_BLOBLIST_SIZE)
#define INIT_RAM_OFFSET			(CONFIG_BLOBLIST_SIZE)
#else
#define INIT_RAM_OFFSET			0x0
#endif
#define CFG_SYS_INIT_RAM_ADDR	(0x80000000 + INIT_RAM_OFFSET)
#define CFG_SYS_INIT_RAM_SIZE	0x200000

#define CFG_SYS_SDRAM_BASE		0x80000000
#define PHYS_SDRAM			0x80000000
#define PHYS_SDRAM_SIZE		SZ_1G /* 1GB DDR */

/* Using ULP WDOG for reset */
#define WDOG_BASE_ADDR			WDG3_BASE_ADDR

#ifdef CONFIG_AHAB_BOOT
#define AHAB_ENV "sec_boot=yes\0"
#else
#define AHAB_ENV "sec_boot=no\0"
#endif

/* both variants use the same settings */
#if defined(CONFIG_TQMA93XX_BB_MBA93XXCA) || defined(CONFIG_TQMA93XX_BB_MBA93XXLA)
#include "tqma93xx-mba93xx.h"
#else
#error "no mainboard variant selected"
#endif

#if defined(CONFIG_ETHPRIME)
#define NETDEV_ENV "netdev=" CONFIG_ETHPRIME "\0"
#else
#define NETDEV_ENV "netdev=eth0\0"
#endif

#include "tq-imx-shared-env.h"

#define CFG_EXTRA_ENV_SETTINGS                                         \
	CFG_MODULE_ENV_SETTINGS                                        \
	TQ_IMX_SHARED_ENV_SETTINGS                                     \
	TQ_IMX_SPI_UBOOT_UPDATE                                        \
	BB_ENV_SETTINGS                                                \
	CFG_MFG_ENV_SETTINGS                                           \
	BOOTENV                                                        \
	NETDEV_ENV                                                     \
	AHAB_ENV

#endif /* __TQMA93XX_H */
