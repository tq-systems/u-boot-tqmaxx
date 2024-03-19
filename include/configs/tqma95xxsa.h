/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2024-2025 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#ifndef __TQMA95XXSA_H
#define __TQMA95XXSA_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include <asm/arch/imx-regs.h>

#define CFG_SYS_UBOOT_BASE	\
	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#if defined(CONFIG_FASTBOOT) && !defined(CONFIG_FSL_FASTBOOT)

#if !defined(CONFIG_FASTBOOT_FLASH_MMC_DEV)
#error "FASTBOOT_FLASH_MMC_DEV must be set to device index of eMMC device"
#endif

#define CFG_MFG_ENV_SETTINGS                                           \
	"fastboot_partition_alias_all="	                               \
		__stringify(CONFIG_FASTBOOT_FLASH_MMC_DEV) ".0:0\0"    \
	"fastboot_partition_alias_bootloader="                         \
		__stringify(CONFIG_FASTBOOT_FLASH_MMC_DEV) ".1:0\0"    \
	"emmc_dev=" __stringify(CONFIG_FASTBOOT_FLASH_MMC_DEV) "\0"    \
	"sd_dev=1\0"

#else

#define CFG_MFG_ENV_SETTINGS                                           \
	"emmc_dev=0\0"                                                 \
	"sd_dev=1\0"

#endif

#if defined(CONFIG_IMX_BOOTAUX)
/* TODO: define environment CFG_CORTEXM_ENV_SETTINGS */
#error "bootaux needs useful settings in environment"
#else
#define CFG_CORTEXM_ENV_SETTINGS
#endif

/* Initial environment variables */
#define CFG_MODULE_ENV_SETTINGS                                        \
	"scriptaddr=0x95500000\0"                                      \
	"image=Image\0"                                                \
	"kernel_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0"        \
	"pxefile_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0"       \
	"fdt_addr_r=0x95000000\0"                                      \
	"fdtoverlay_addr_r=0x95080000\0"                               \
	"initrd_addr=0x95800000\0"                                     \
	"mmcdev=" __stringify(CONFIG_SYS_MMC_ENV_DEV) "\0"             \
	"mmcautodetect=yes\0"                                          \
	"boot_os=booti ${kernel_addr_r} - ${fdt_addr_r};\0"            \
	"emmc_bootp_start=0x0\0"                                       \
	"uboot_mmc_start=0x40\0"                                       \
	"uboot_mmc_size=0x1fc0\0"                                      \
	"uboot_spi_start=0x0\0"                                        \
	"uboot_spi_size=0x400000\0"                                    \
	"uboot=bootstream.bin\0"                                       \

#if !defined(CONFIG_BOOTCOMMAND)
#warn "CONFIG_BOOTCOMMAND missing in configuration"
#endif

/* Link Definitions */
#define CFG_SYS_INIT_RAM_ADDR		0x90000000
#define CFG_SYS_INIT_RAM_SIZE		0x200000

#if defined(CONFIG_TQMA95XXSA_RAM_2GB_DEFAULT)

#define CFG_SYS_SDRAM_BASE		0x90000000
#define PHYS_SDRAM			0x90000000
/* Totally 2GB in 2GB variant */
#define PHYS_SDRAM_SIZE			SZ_2G - SZ_256M /* 2GB - 256MB DDR */

#elif defined(CONFIG_TQMA95XXSA_RAM_4GB_DEFAULT)

#define CFG_SYS_SDRAM_BASE		0x90000000
#define PHYS_SDRAM			0x90000000
/* Totally 2GB in 4GB variant */
#define PHYS_SDRAM_SIZE			SZ_2G - SZ_256M /* 2GB - 256MB DDR */
/* Additional 2GB in 4GB variant */
#define PHYS_SDRAM_2_SIZE		0x80000000

#else

#error "RAM size not supported"

#endif /* RAM SIZE Settings */

/* Using ULP WDOG for reset */
#define WDOG_BASE_ADDR			WDG3_BASE_ADDR

#if defined(CONFIG_TQMA95XXSA_BB_MB_SMARC_2)
#include "tqma95xxsa-mb-smarc-2.h"
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
	CFG_CORTEXM_ENV_SETTINGS                                       \
	CFG_MODULE_ENV_SETTINGS                                        \
	TQ_IMX_SHARED_ENV_SETTINGS                                     \
	TQ_IMX_SPI_UBOOT_UPDATE                                        \
	BB_ENV_SETTINGS                                                \
	CFG_MFG_ENV_SETTINGS                                           \
	BOOTENV                                                        \
	NETDEV_ENV

#endif /* __TQMA95XXSA_H */
