/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 *
 * Copyright (c) 2016-2024 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel, Steffen Doster
 *
 * Configuration settings for the TQ-Systems TQMa7x SOM
 */

#ifndef __TQMA7_CONFIG_H
#define __TQMA7_CONFIG_H

#include "mx7_common.h"

#if IS_ENABLED(CONFIG_TQMA7_512MB)
#define PHYS_SDRAM_SIZE			SZ_512M
#elif IS_ENABLED(CONFIG_TQMA7_1GB)
#define PHYS_SDRAM_SIZE			SZ_1G
#elif IS_ENABLED(CONFIG_TQMA7_2GB)
#define PHYS_SDRAM_SIZE			SZ_2G
#endif

/*
 * above 128 MiB offset as in ARM related docu for linux suggested
 * DTB is loaded at 128 MiB, so use just 16 MiB more
 */
#define TQMA7_INITRD_ADDRESS		0x89000000

#define TQMA7_UBOOT_OFFSET		SZ_1K
#define TQMA7_MMC_UBOOT_SECTOR_START	0x2
#define TQMA7_MMC_UBOOT_SECTOR_COUNT	0x7fe
#define TQMA7_SPI_FLASH_SECTOR_SIZE	SZ_64K
#define TQMA7_SPI_UBOOT_START		0x1000
#define TQMA7_SPI_UBOOT_SIZE		0xf0000

#if IS_ENABLED(CONFIG_IMX_BOOTAUX)

#define TQMA7_AUXCORE_BOOTDATA 0x7F8000 /* Set to TCML address */

#define TQMA7_M4_ENV                                                           \
	"firmwarepart=1\0"                                                     \
	"m4image=m4.bin\0"                                                     \
	"m4loadaddr=" __stringify(TQMA7_AUXCORE_BOOTDATA) "\0"                 \
	"loadm4image=load mmc ${mmcdev}:${firmwarepart} ${m4loadaddr} ${m4image}\0" \
	"m4boot=run loadm4image; bootaux ${m4loadaddr}\0"                      \
	"m4netboot=tftp ${m4image}; bootaux ${m4loadaddr}\0"                   \
	""

#else
#define TQMA7_M4_ENV
#endif

/* 128 MiB offset as in ARM related docs for Linux suggested */
#define TQMA7_FDT_ADDRESS		0x88000000

#define TQMA7_MODULE_ENV_SETTINGS \
	"board=tqma7\0"                                                        \
	"boot_os=bootz ${loadaddr} - ${fdt_addr_r}\0"                          \
	"fdt_addr_r=" __stringify(TQMA7_FDT_ADDRESS) "\0"                      \
	"image=zImage\0"                                                       \
	"kernel_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0"                \
	"netdev=eth0\0"                                                        \
	"ipmode=static\0"                                                      \
	"uboot=u-boot-dtb.imx\0"                                               \
	"uboot_mmc_start=" __stringify(TQMA7_MMC_UBOOT_SECTOR_START) "\0"      \
	"uboot_mmc_size=" __stringify(TQMA7_MMC_UBOOT_SECTOR_COUNT) "\0"       \
	"uboot_spi_sector_size=" __stringify(TQMA7_SPI_FLASH_SECTOR_SIZE) "\0" \
	"uboot_spi_start=" __stringify(TQMA7_SPI_UBOOT_START) "\0"             \
	"uboot_spi_size=" __stringify(TQMA7_SPI_UBOOT_SIZE) "\0"               \
	""

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CFG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CFG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/*
 * All the defines above are for the TQMa7 SoM
 *
 * Now include the baseboard specific configuration
 */

#if IS_ENABLED(CONFIG_MBA7)
#include "tqma7_mba7.h"
#else
#error "No baseboard for the TQMa7 SOM defined!"
#endif

#include "tq-imx-shared-env.h"

#define CFG_EXTRA_ENV_SETTINGS                                                 \
	TQ_IMX_SHARED_ENV_SETTINGS                                             \
	TQ_QSPIHDR_IMX_SPI_UBOOT_UPDATE                                        \
	TQMA7_M4_ENV                                                           \
	TQMA7_MODULE_ENV_SETTINGS                                              \
	BB_ENV_SETTINGS                                                        \
	""

#endif /* __TQMA7_CONFIG_H */
