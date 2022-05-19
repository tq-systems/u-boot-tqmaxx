/* SPDX-License-Identifier: GPL-2.0-or-later
 *
 * tqma335x.h
 *
 * Copyright (C) 2021 TQ-Systems GmbH
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __CONFIG_AM335X_MBA335X_H
#define __CONFIG_AM335X_MBA335X_H

#include <configs/ti_am335x_common.h>
#include <linux/sizes.h>

#ifndef CONFIG_SPL_BUILD
#define CONFIG_TIMESTAMP
#endif

#define CONFIG_SYS_DDR_DATA_INIT	0xdeadbeef
#define CONFIG_SYS_MEMTEST_START	0x81000000
#if defined(CONFIG_TQMA335X_512MB)
#define CONFIG_SYS_MEMTEST_END		0x9F000000
#elif defined(CONFIG_TQMA335X_256MB)
#define CONFIG_SYS_MEMTEST_END		0x8D000000
#else
#error "not a valid memory size config"
#endif
#define CONFIG_SYS_MEMTEST_SCRATCH	(CONFIG_SYS_MEMTEST_END)

#define CONFIG_SYS_BOOTM_LEN		SZ_16M

/* Clock Defines */
#define V_OSCK				24000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

/* Custom script for NOR */
#define CONFIG_SYS_LDSCRIPT		"board/tqc/tqma335x/u-boot.lds"

/* Always 128 KiB env size */
#define CONFIG_ENV_SIZE			SZ_128K

#define BOOTENV_DEV_LEGACY_MMC(devtypeu, devtypel, instance) \
	"bootcmd_" #devtypel #instance "=" \
	"setexpr bootdev ${mmcdev} ^ " #instance "; " \
	"setenv mmcdev ${bootdev}; "\
	"setenv bootpart ${bootdev}:2; "\
	"run mmcboot\0"

#define BOOTENV_DEV_NAME_LEGACY_MMC(devtypeu, devtypel, instance) \
	#devtypel #instance " "

#define BOOT_TARGET_DEVICES(func) \
	func(LEGACY_MMC, legacy_mmc, 0) \
	func(LEGACY_MMC, legacy_mmc, 1)

#include <config_distro_bootcmd.h>

#ifndef CONFIG_SPL_BUILD
#include <environment/ti/dfu.h>
#include <environment/ti/mmc.h>

#define CONFIG_EXTRA_ENV_SETTINGS \
	DEFAULT_LINUX_BOOT_ENV \
	DEFAULT_MMC_TI_ARGS \
	"boot_fit=0\0" \
	"mmcautodetect=yes\0" \
	"bootpart=0:2\0" \
	"bootdir=/boot\0" \
	"bootfile=zImage\0" \
	"fdtfile=" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0" \
	"console=ttyS4,115200n8\0" \
	"partitions=" \
		"uuid_disk=${uuid_gpt_disk};" \
		"name=bootloader,start=384K,size=1792K," \
			"uuid=${uuid_gpt_bootloader};" \
		"name=rootfs,start=2688K,size=-,uuid=${uuid_gpt_rootfs}\0" \
	"optargs=\0" \
	"uboot=u-boot.img\0" \
	"mlo=MLO\0" \
	"init_console=setenv console ttyS4,115200n8\0" \
	"mmcfatload=mmc dev ${mmcdev} && mmc rescan && " \
		"fatload mmc ${mmcdev}:1 ${loadaddr} ${fatfile}\0" \
	"mmcfatwrite= mmc dev ${mmcdev} && mmc rescan && " \
		"fatwrite mmc ${mmcdev}:1 ${loadaddr} ${fatfile} " \
			"${filesize}\0" \
	"mmcwrite_uboot=tftp ${mlo} && " \
		"setenv fatfile ${mlo} && run  mmcfatwrite && " \
		"tftp ${uboot} && " \
		"setenv fatfile ${uboot} && run mmcfatwrite\0" \
	"mmcwrite_kernel=tftp ${bootfile} && " \
		"setenv fatfile ${bootfile} && run mmcfatwrite\0" \
	"upd_uboot_spi_net=tftp ${mlo_spi} && " \
		"setenv spioffset 0x0 && run spiwrite && " \
		"tftp ${uboot_spi} && " \
		"setenv spioffset 0x20000 && run spiwrite\0" \
	"upd_uboot_spi_sd=setenv mmcdev 0 && " \
		"setenv fatfile ${mlo_spi} && run mmcfatload && " \
		"setenv spioffset 0x0 && run spiwrite && " \
		"setenv fatfile ${uboot_spi} && run mmcfatload &&" \
		"setenv spioffset 0x20000 && run spiwrite\0" \
	"upd_uboot_sd_net=setenv mmcdev 0 && run mmcwrite_uboot\0" \
	"upd_uboot_emmc_net=setenv mmcdev 1 && run mmcwrite_uboot\0" \
	"upd_kernel_sd_net=setenv mmcdev 0 && run mmcwrite_kernel\0" \
	"upd_kernel_emmc_net=setenv mmcdev 1 && run mmcwrite_kernel\0" \
	NETARGS \
	DFUARGS \
	BOOTENV
#endif

/* NS16550 Configuration */
#define CONFIG_SYS_NS16550_COM1		0x44e09000	/* Base EVM has UART0 */
#define CONFIG_SYS_NS16550_COM2		0x48022000	/* UART1 */
#define CONFIG_SYS_NS16550_COM3		0x48024000	/* UART2 */
#define CONFIG_SYS_NS16550_COM4		0x481a6000	/* UART3 */
#define CONFIG_SYS_NS16550_COM5		0x481a8000	/* UART4 */
#define CONFIG_SYS_NS16550_COM6		0x481aa000	/* UART5 */

#define CONFIG_ENV_EEPROM_IS_ON_I2C
#define CONFIG_SYS_EEPROM_BUS_NUM 0
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50	/* Main EEPROM */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2

/* PMIC support */
#define CONFIG_POWER_TPS65910

/* USB Device Firmware Update support */
#ifndef CONFIG_SPL_BUILD
#define DFUARGS \
	DFU_ALT_INFO_EMMC \
	DFU_ALT_INFO_MMC \
	DFU_ALT_INFO_RAM
#endif

#if defined(CONFIG_SPI_BOOT)
/* SPL related */
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x20000

#define CONFIG_ENV_SECT_SIZE		(4 << 10) /* 4 KB sectors */
#define CONFIG_ENV_OFFSET		(768 << 10) /* 768 KiB in */
#define CONFIG_ENV_OFFSET_REDUND	(896 << 10) /* 896 KiB in */
#endif

#ifdef CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV		1
#define CONFIG_SYS_MMC_ENV_PART		0
#define CONFIG_ENV_OFFSET		0x260000
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_SYS_MMC_MAX_DEVICE	2
#endif

#ifdef CONFIG_DRIVER_TI_CPSW
#define CONFIG_CLOCK_SYNTHESIZER
#define CLK_SYNTHESIZER_I2C_ADDR 0x65
#endif

#endif	/* ! __CONFIG_AM335X_MBA335X_H */
