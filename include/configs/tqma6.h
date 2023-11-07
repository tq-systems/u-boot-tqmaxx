/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013, 2014, 2017 Markus Niebel <Markus.Niebel@tq-group.com>
 *
 * Configuration settings for the TQ-Systems TQMa6<Q,D,DL,S> module.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/kconfig.h>
#include <linux/stringify.h>

/* place code in last 4 MiB of RAM */

#include "mx6_common.h"

/* I2C Configs */
#define CFG_I2C_MULTI_BUS

/* MMC Configs */
#define CFG_SYS_FSL_ESDHC_ADDR	0

/* USB Configs */
#define CFG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)

/* 128 MiB offset as in ARM related docu for linux suggested */
#define TQMA6_FDT_ADDRESS	0x18000000

#define TQMA6_SPI_FLASH_SECTOR_SIZE SZ_64K

#define CFG_MODULE_ENV_SETTINGS                                                \
	"board=tqma6\0"                                                        \
	"boot_os=bootz ${kernel_addr_r} - ${fdt_addr_r}\0"                     \
	"fdt_addr_r=" __stringify(TQMA6_FDT_ADDRESS)"\0"                       \
	"image=zImage\0"                                                       \
	"kernel_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0"                \
	"mmcautodetect=yes\0"                                                  \
	"mmcblkdev=0\0"                                                        \
	"mmcdev=" __stringify(CONFIG_SYS_MMC_ENV_DEV)"\0"                      \
	"netdev=eth0\0"                                                        \
	"uboot=u-boot.imx\0"                                                   \
	"uboot_mmc_size=0x7fe\0"                                               \
	"uboot_mmc_start=0x2\0"                                                \
	"uboot_spi_sector_size=" __stringify(TQMA6_SPI_FLASH_SECTOR_SIZE) "\0" \
	"uboot_spi_size=0xa0000\0"                                             \
	"uboot_spi_start=0x400\0"                                              \

/* Physical Memory Map */
#define PHYS_SDRAM		MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE	PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/*
 * All the defines above are for the TQMa6 SoM
 *
 * Now include the baseboard specific configuration
 */
#ifdef CONFIG_MBA6
#include "tqma6_mba6.h"
#elif CONFIG_WRU4
#include "tqma6_wru4.h"
#else
#error "No baseboard for the TQMa6 defined!"
#endif

#include "tq-imx-shared-env.h"

#define CFG_EXTRA_ENV_SETTINGS		\
	TQ_IMX_SHARED_ENV_SETTINGS	\
	CFG_MODULE_ENV_SETTINGS		\
	BB_ENV_SETTINGS			\

/* Support at least the sensor on TQMa6 SOM */

#endif /* __CONFIG_H */
