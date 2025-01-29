/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2016-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Marco Felsch, Matthias Schiffer
 *
 * Configuration settings for the TQ-Systems TQMa6UL[L]x[L] SOM family.
 */

#ifndef __TQMA6UL_CONFIG_H
#define __TQMA6UL_CONFIG_H

#include "mx6_common.h"

/* 128 MiB offset as suggested in ARM related Linux docs */
#define TQMA6UL_FDT_ADDRESS		0x88000000
#define FDT_OVERLAY_ADDR		(TQMA6UL_FDT_ADDRESS + SZ_256K)

/* 16MiB above TQMA6UL_FDT_ADDRESS */
#define TQMA6UL_INITRD_ADDRESS		(TQMA6UL_FDT_ADDRESS + SZ_16M)

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR		IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE		IRAM_SIZE

/* u-boot.img base address for SPI-NOR boot */
#define CFG_SYS_UBOOT_BASE	(QSPI0_AMBA_BASE + 0x1000 + CONFIG_SPL_PAD_TO)

#define CFG_SYS_INIT_SP_OFFSET \
	(CFG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CFG_SYS_INIT_SP_ADDR \
	(CFG_SYS_INIT_RAM_ADDR + CFG_SYS_INIT_SP_OFFSET)

#define TQMA6UL_MMC_UBOOT_SECTOR_START	0x2
#define TQMA6UL_MMC_UBOOT_SECTOR_COUNT	0x7fe

#define TQMA6UL_SPI_FLASH_SECTOR_SIZE	SZ_64K
#define TQMA6UL_SPI_UBOOT_START		0x1000
#define TQMA6UL_SPI_UBOOT_SIZE		0xf0000

#define CFG_MODULE_ENV_SETTINGS                                                  \
	"emmc_dev=0\0"                                                           \
	"sd_dev=1\0"                                                             \
	"board=tqma6ul\0"                                                        \
	"boot_os=bootz ${kernel_addr_r} - ${fdt_addr_r}\0"                       \
	"fdt_addr_r=" __stringify(TQMA6UL_FDT_ADDRESS)"\0"                       \
	"fdtoverlay_addr_r=" __stringify(FDT_OVERLAY_ADDR)"\0"                   \
	"image=zImage\0"                                                         \
	"kernel_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR)"\0"                   \
	"pxefile_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0"                 \
	"ramdisk_addr_r=" __stringify(TQMA6UL_INITRD_ADDRESS) "\0"               \
	"mmcautodetect=yes\0"                                                    \
	"mmcblkdev=0\0"                                                          \
	"mmcdev=" __stringify(CONFIG_SYS_MMC_ENV_DEV)"\0"                        \
	"netdev=eth1\0"                                                          \
	"uboot=u-boot-with-spl.imx\0"                                            \
	"uboot_mmc_start=" __stringify(TQMA6UL_MMC_UBOOT_SECTOR_START)"\0"       \
	"uboot_mmc_size=" __stringify(TQMA6UL_MMC_UBOOT_SECTOR_COUNT) "\0"       \
	"uboot_spi_sector_size=" __stringify(TQMA6UL_SPI_FLASH_SECTOR_SIZE) "\0" \
	"uboot_spi_start=" __stringify(TQMA6UL_SPI_UBOOT_START) "\0"             \
	"uboot_spi_size=" __stringify(TQMA6UL_SPI_UBOOT_SIZE) "\0"               \

#include "tq-imx-shared-env.h"

#define CFG_EXTRA_ENV_SETTINGS			\
	TQ_IMX_SHARED_ENV_SETTINGS		\
	CFG_MODULE_ENV_SETTINGS			\
	BB_ENV_SETTINGS				\
	TQ_QSPIHDR_IMX_SPI_UBOOT_UPDATE		\

#endif /* __TQMA6UL_CONFIG_H */
