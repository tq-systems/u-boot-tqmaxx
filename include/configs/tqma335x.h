/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2021 - 2022 TQ-Systems GmbH <license@tq-group.com>, D-82229 Seefeld, Germany.
 * Author: Gregor Herburger
 *
 * Based on:
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 */

#ifndef __CONFIG_AM335X_MBA335X_H
#define __CONFIG_AM335X_MBA335X_H

#include <configs/ti_am335x_common.h>
#include <linux/sizes.h>

#ifndef CONFIG_SPL_BUILD
#define CONFIG_TIMESTAMP
#endif

#define CONFIG_SYS_DDR_DATA_INIT	0xdeadbeef
#define CONFIG_SYS_MEMTEST_START	(CONFIG_SYS_SDRAM_BASE + SZ_16M)

#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + \
					 (gd->ram_size / 4 * 3))
#define CONFIG_SYS_MEMTEST_SCRATCH	(CONFIG_SYS_MEMTEST_END)

#define CONFIG_SYS_BOOTM_LEN		SZ_16M

/* Clock Defines */
#define V_OSCK				24000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

/* Custom script for NOR */
#define CONFIG_SYS_LDSCRIPT		"board/tqc/tqma335x/u-boot.lds"

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
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"console=ttyS4,115200n8\0" \
	"partitions=" \
		"uuid_disk=${uuid_gpt_disk};" \
		"name=bootloader,start=384K,size=1792K," \
			"uuid=${uuid_gpt_bootloader};" \
		"name=rootfs,start=2688K,size=-,uuid=${uuid_gpt_rootfs}\0" \
	"optargs=\0" \
	"uboot=u-boot.img\0" \
	"mlo=MLO\0" \
	"uboot_spi=u-boot.img\0" \
	"mlo_spi=MLO.byteswap\0" \
	"init_console=setenv console ttyS4,115200n8\0" \
	"update_uboot_mmc=if mmc dev ${mmcdev} && mmc rescan; then " \
			"run set_getcmd; " \
			"setenv mlo_load_addr $loadaddr; " \
			"setexpr uboot_load_addr ${loadaddr} + " __stringify(SZ_256K) "; " \
			"if ${getcmd} ${mlo_load_addr} ${mlo}; then " \
				"setenv mlo_size $filesize; " \
				"if ${getcmd} ${uboot_load_addr} ${uboot}; then " \
					"fatwrite mmc ${mmcdev}:1 ${mlo_load_addr} ${mlo} ${mlo_size} && " \
					"fatwrite mmc ${mmcdev}:1 ${uboot_load_addr} ${uboot} ${filesize}; " \
				"else " \
					"echo ERROR: {getcmd} ${uboot} failed; " \
				"fi; " \
			"else " \
				"echo ERROR: {getcmd} ${mlo} failed; " \
			"fi; " \
		"fi; " \
		"setenv ${filesize}; setenv getcmd; setenv mlo_load_addr; " \
		"setenv mlo_size; setenv uboot_load_addr \0" \
	"update_uboot_spi=if sf probe; then " \
			"run set_getcmd; " \
			"setenv mlo_load_addr $loadaddr; " \
			"setexpr uboot_load_addr $loadaddr + " __stringify(CONFIG_SYS_SPI_U_BOOT_OFFS) "; " \
			"if ${getcmd} ${mlo_load_addr} ${mlo_spi}; then " \
				"setenv mlo_size $filesize; " \
				"if ${getcmd} ${uboot_load_addr} ${uboot_spi}; then " \
					"if itest ${mlo_size} <= " __stringify(CONFIG_SYS_SPI_U_BOOT_OFFS) "; then " \
						"sf update ${mlo_load_addr} 0x0 ${mlo_size} && " \
						"sf update ${uboot_load_addr} " __stringify(CONFIG_SYS_SPI_U_BOOT_OFFS) " ${filesize}; " \
					"else " \
						"echo ERROR: ${mlo_spi} to large; " \
					"fi; " \
				"else " \
					"echo ERROR: {getcmd} ${uboot_spi} failed; " \
				"fi; " \
			"else " \
				"echo ERROR: {getcmd} ${mlo_spi} failed; " \
			"fi; " \
		"fi; " \
		"setenv ${filesize}; setenv getcmd; setenv mlo_load_addr; " \
		"setenv mlo_size; setenv uboot_load_addr \0" \
	"upd_uboot_spi_net=run update_uboot_spi\0" \
	"upd_uboot_sd_net=setenv mmcdev 0 && run update_uboot_mmc\0" \
	"upd_uboot_emmc_net=setenv mmcdev 1 && run update_uboot_mmc\0" \
	"addtty=setenv bootargs ${bootargs} console=${console}\0"              \
	"netdev=eth0\0"                                                        \
	"rootpath=/srv/nfs/exports\0"                                          \
	"ipmode=static\0"                                                      \
	"netargs=run addnfs addip addtty\0"                                    \
	"addnfs=setenv bootargs ${bootargs} "                                  \
		"root=/dev/nfs rw "                                            \
		"nfsroot=${serverip}:${rootpath},v3,tcp;\0"                    \
	"addip_static=setenv bootargs ${bootargs} "                            \
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:"            \
		"${hostname}:${netdev}:off\0"                                  \
	"addip_dynamic=setenv bootargs ${bootargs} ip=dhcp\0"                  \
	"addip=if test \"${ipmode}\" != static; then "                         \
		"run addip_dynamic; else run addip_static; fi\0"               \
	"set_getcmd=if test \"${ipmode}\" != static; then "                    \
		"setenv getcmd dhcp; setenv autoload yes; "                    \
		"else setenv getcmd tftp; setenv autoload no; fi\0"            \
	"netboot=echo Booting from net ...; "                                  \
		"run set_getcmd; "                                             \
		"setenv bootargs; "                                            \
		"run netargs; "                                                \
		"if ${getcmd} ${bootfile}; then "                              \
			"if ${getcmd} ${fdtaddr} ${fdtfile}; then "            \
				"bootz ${loadaddr} - ${fdtaddr}; "             \
			"fi; "                                                 \
		"fi; "                                                         \
		"echo ... failed\0"                                            \
	DFUARGS \
	BOOTENV \
	"boot_targets=legacy_mmc0\0"
#endif

/* NS16550 Configuration */
#define CONFIG_SYS_NS16550_COM1		0x44e09000	/* Base EVM has UART0 */
#define CONFIG_SYS_NS16550_COM2		0x48022000	/* UART1 */
#define CONFIG_SYS_NS16550_COM3		0x48024000	/* UART2 */
#define CONFIG_SYS_NS16550_COM4		0x481a6000	/* UART3 */
#define CONFIG_SYS_NS16550_COM5		0x481a8000	/* UART4 */
#define CONFIG_SYS_NS16550_COM6		0x481aa000	/* UART5 */

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

/* SPL related */
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x20000

/* Always 128 KiB env size */
#define CONFIG_ENV_SIZE			SZ_128K

#ifdef CONFIG_ENV_IS_IN_SPI_FLASH
/* 4 KB sectors should be disabled */
#if defined(CONFIG_SPI_FLASH_USE_4K_SECTORS)
#error "settings are only valid with large SPI NOR erase sectors"
#endif

#define CONFIG_ENV_SECT_SIZE		SZ_64K
/* 768 KiB */
#define CONFIG_ENV_OFFSET		(768 * SZ_1K)
 /* 896 KiB */
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#endif

#ifdef CONFIG_ENV_IS_IN_MMC
/*
 * This is intentionally set to an invalid value and shall be set using a
 * project / mainboard specific mmc_get_env_dev()
 * CONFIG_ENV_OFFSET_REDUND and CONFIG_ENV_OFFSET shall be configured via
 * project / mainboard specific -uboot-dtsi
 */
#define CONFIG_SYS_MMC_ENV_DEV		-1
#define CONFIG_SYS_MMC_ENV_PART		0
#endif

#define CONFIG_SYS_MMC_MAX_DEVICE	2

#ifdef CONFIG_DRIVER_TI_CPSW
#define CONFIG_CLOCK_SYNTHESIZER
#define CLK_SYNTHESIZER_I2C_ADDR 0x65
#endif

#endif	/* ! __CONFIG_AM335X_MBA335X_H */
