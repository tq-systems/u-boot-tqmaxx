/*
 * Copyright (C) 2013, 2014, 2017 Markus Niebel <Markus.Niebel@tq-group.com>
 *
 * Configuration settings for the TQ Systems TQMa6<Q,D,DL,S> module.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/kconfig.h>
/* SPL */
/* common IMX6 SPL configuration */
#include "imx6_spl.h"

#undef CONFIG_SPL_MAX_SIZE
/*
 * define CONFIG_SPL_MAX_SIZE to fill at max one SPI NOR flash sector (64k)
 * substracted by image offset (1k)
 */
#define CONFIG_SPL_MAX_SIZE		0xfc00

#undef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR
/*
 * force offset im eMMC/SD the same as in SPI NOR. if using SPI NOR sector size,
 * we can have combinded and splitted images as well
 */
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	(TQMA6_SPI_FLASH_SECTOR_SIZE) / \
						0x200

#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x10000

/*
 * place code in last 4 MiB of RAM, for combined image use 512 MiB as common
 * value
 */
#if defined(CONFIG_TQMA6QDL) || defined(CONFIG_TQMA6S)
#undef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE		0x2fc00000
#elif defined(CONFIG_TQMA6QP) ||defined(CONFIG_TQMA6Q) || defined(CONFIG_TQMA6DL)
#undef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE		0x4fc00000
#endif

#include "mx6_common.h"

#define CONFIG_IMX_THERMAL
#define CONFIG_MXC_UART

/* SPI */
#define CONFIG_MXC_SPI

/* SPI Flash */

#define TQMA6_SPI_FLASH_SECTOR_SIZE	SZ_64K

#define CONFIG_SF_DEFAULT_BUS	0
#define CONFIG_SF_DEFAULT_CS	0
#define CONFIG_SF_DEFAULT_SPEED	50000000
#define CONFIG_SF_DEFAULT_MODE	(SPI_MODE_0)

/* I2C Configs */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_MXC_I2C3		/* enable I2C bus 3 */
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_SYS_I2C_SPEED		100000

/* I2C EEPROM (M24C64) */
#define CONFIG_SYS_I2C_EEPROM_ADDR			0x50
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN			2
#define CONFIG_SYS_I2C_EEPROM_PAGE_WRITE_BITS		5 /* 32 Bytes */
#define CONFIG_SYS_I2C_EEPROM_PAGE_WRITE_DELAY_MS	20

#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_POWER_PFUZE100
#define CONFIG_POWER_PFUZE100_I2C_ADDR	0x08

/* MMC Configs */
#define CONFIG_SYS_FSL_ESDHC_ADDR	0
#define CONFIG_SUPPORT_EMMC_BOOT
#define CONFIG_SUPPORT_EMMC_RPMB

/* USB Configs */
#define CONFIG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_USB_MAX_CONTROLLER_COUNT	2
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET	/* For OTG port */

#define CONFIG_FEC_MXC
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_MII

#define CONFIG_ARP_TIMEOUT		200UL

#define CONFIG_ENV_SIZE			(SZ_8K)
/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 2 * SZ_1M)

#if defined(CONFIG_TQMA6X_MMC_BOOT)

#define TQMA6_UBOOT_OFFSET		SZ_1K
#define TQMA6_UBOOT_SECTOR_START	0x2
#define TQMA6_UBOOT_SECTOR_COUNT	0x7fe

#define CONFIG_ENV_OFFSET		SZ_1M
#define CONFIG_SYS_MMC_ENV_DEV		0

#define TQMA6_EXTRA_BOOTDEV_ENV_SETTINGS                                       \
	"uboot_start="__stringify(TQMA6_UBOOT_SECTOR_START)"\0"                \
	"uboot_size="__stringify(TQMA6_UBOOT_SECTOR_COUNT)"\0"                 \
	"mmcdev=-1\0"                                                          \
	"firmwarepart=1\0"                                                     \
	"loadimage=run kernel_name; "                                          \
		"load mmc ${mmcdev}:${firmwarepart} ${loadaddr} ${kernel} \0"  \
	"loadfdt=load mmc ${mmcdev}:${firmwarepart} ${fdt_addr} ${fdt_file}\0" \
	"update_uboot=run set_getcmd; "                                        \
		"if ${getcmd} ${uboot}; then "                                 \
			"if itest ${filesize} > 0; then "                      \
				"mmc dev ${mmcdev}; mmc rescan; "              \
				"setexpr blkc ${filesize} + 0x1ff; "           \
				"setexpr blkc ${blkc} / 0x200; "               \
				"if itest ${blkc} <= ${uboot_size}; then "     \
					"mmc write ${loadaddr} ${uboot_start} "\
						"${blkc}; "                    \
				"fi; "                                         \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv blkc; setenv getcmd \0"               \
	"update_kernel=run kernel_name; run set_getcmd; "                      \
		"if ${getcmd} ${kernel}; then "                                \
			"if itest ${filesize} > 0; then "                      \
				"echo Write kernel image to mmc ${mmcdev}:${firmwarepart}...; " \
				"save mmc ${mmcdev}:${firmwarepart} ${loadaddr} " \
					"${kernel} ${filesize}; "              \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv getcmd \0"                            \
	"update_fdt=run set_getcmd; "                                          \
		"if ${getcmd} ${fdt_file}; then "                              \
			"if itest ${filesize} > 0; then "                      \
				"echo Write fdt image to mmc ${mmcdev}:${firmwarepart}...; " \
				"save mmc ${mmcdev}:${firmwarepart} ${loadaddr} " \
					"${fdt_file} ${filesize}; "            \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv getcmd \0"                            \

#define CONFIG_BOOTCOMMAND \
	"run mmcboot; run netboot; run panicboot"

#elif defined(CONFIG_TQMA6X_SPI_BOOT)

#define TQMA6_UBOOT_OFFSET		SZ_1K
#define TQMA6_UBOOT_SECTOR_START	0x0
/* max u-boot size: 512k */
#define TQMA6_UBOOT_SECTOR_SIZE		TQMA6_SPI_FLASH_SECTOR_SIZE
#define TQMA6_UBOOT_SECTOR_COUNT	0x8
#define TQMA6_UBOOT_SIZE		(TQMA6_UBOOT_SECTOR_SIZE * \
					 TQMA6_UBOOT_SECTOR_COUNT)

#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_OFFSET		(TQMA6_UBOOT_SIZE)
#define CONFIG_ENV_SECT_SIZE		TQMA6_SPI_FLASH_SECTOR_SIZE
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + \
					 CONFIG_ENV_SECT_SIZE)

#define CONFIG_ENV_SPI_BUS		(CONFIG_SF_DEFAULT_BUS)
#define CONFIG_ENV_SPI_CS		(CONFIG_SF_DEFAULT_CS)
#define CONFIG_ENV_SPI_MAX_HZ		(CONFIG_SF_DEFAULT_SPEED)
#define CONFIG_ENV_SPI_MODE		(CONFIG_SF_DEFAULT_MODE)

#define TQMA6_FDT_OFFSET		(CONFIG_ENV_OFFSET_REDUND + \
					 CONFIG_ENV_SECT_SIZE)
#define TQMA6_FDT_SECT_SIZE		(TQMA6_SPI_FLASH_SECTOR_SIZE)

#define TQMA6_FDT_SECTOR_START		0x0a /* 8 Sector u-boot, 2 Sector env */
#define TQMA6_FDT_SECTOR_COUNT		0x01

#define TQMA6_KERNEL_SECTOR_START	0x10 /* offset 1 MB */
#define TQMA6_KERNEL_SECTOR_COUNT	0xa0 /* 10 MB */

#define TQMA6_EXTRA_BOOTDEV_ENV_SETTINGS                                       \
	"mmcblkdev=0\0"                                                        \
	"uboot_offset="__stringify(TQMA6_UBOOT_OFFSET)"\0"                     \
	"uboot_sectors="__stringify(TQMA6_UBOOT_SECTOR_COUNT)"\0"              \
	"fdt_start="__stringify(TQMA6_FDT_SECTOR_START)"\0"                    \
	"fdt_sectors="__stringify(TQMA6_FDT_SECTOR_COUNT)"\0"                  \
	"kernel_start="__stringify(TQMA6_KERNEL_SECTOR_START)"\0"              \
	"kernel_sectors="__stringify(TQMA6_KERNEL_SECTOR_COUNT)"\0"            \
	"update_uboot=run set_getcmd; "                                        \
		"if ${getcmd} ${uboot}; then "                                 \
			"if itest ${filesize} > 0; then "                      \
				"setexpr blkc ${filesize} + "                  \
					__stringify(TQMA6_UBOOT_OFFSET)"; "    \
				"setexpr size ${uboot_sectors} * "             \
					__stringify(TQMA6_SPI_FLASH_SECTOR_SIZE)"; " \
				"if itest ${blkc} <= ${size}; then "           \
					"sf probe; "                           \
					"sf erase 0 ${size}; "                 \
					"sf write ${loadaddr} ${uboot_offset} "\
						"${filesize}; "                \
				"fi; "                                         \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize 0; setenv blkc; setenv size; setenv getcmd \0"\
	"update_kernel=run kernel_name; run set_getcmd; "                      \
		"if ${getcmd} ${kernel}; then "                                \
			"if itest ${filesize} > 0; then "                      \
				"setexpr size ${kernel_sectors} * "            \
					__stringify(TQMA6_SPI_FLASH_SECTOR_SIZE)"; " \
				"setexpr offset ${kernel_start} * "            \
					__stringify(TQMA6_SPI_FLASH_SECTOR_SIZE)"; " \
				"if itest ${filesize} <= ${size}; then "       \
					"sf probe; "                           \
					"sf erase ${offset} ${size}; "         \
					"sf write ${loadaddr} ${offset} "      \
						"${filesize}; "                \
				"fi; "                                         \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize 0; setenv size ; setenv offset; "             \
		"setenv getcmd \0"                                             \
	"update_fdt=run set_getcmd; "                                          \
		"if ${getcmd} ${fdt_file}; then "                              \
			"if itest ${filesize} > 0; then "                      \
				"setexpr size ${fdt_sectors} * "               \
					__stringify(TQMA6_SPI_FLASH_SECTOR_SIZE)"; " \
				"setexpr offset ${fdt_start} * "               \
					__stringify(TQMA6_SPI_FLASH_SECTOR_SIZE)"; " \
				"if itest ${filesize} <= ${size}; then "       \
					"sf probe; "                           \
					"sf erase ${offset} ${size}; "         \
					"sf write ${loadaddr} ${offset} "      \
						"${filesize}; "                \
				"fi; "                                         \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv getcmd; "                                              \
		"setenv filesize 0; setenv size; setenv offset\0"              \
	"loadimage=sf probe; "                                                 \
		"setexpr size ${kernel_sectors} * "                            \
			__stringify(TQMA6_SPI_FLASH_SECTOR_SIZE)"; "           \
		"setexpr offset ${kernel_start} * "                            \
			__stringify(TQMA6_SPI_FLASH_SECTOR_SIZE)"; "           \
		"sf read ${loadaddr} ${offset} ${size}; "                      \
		"setenv size ; setenv offset\0"                                \
	"loadfdt=sf probe; "                                                   \
		"setexpr size ${fdt_sectors} * "                               \
			__stringify(TQMA6_SPI_FLASH_SECTOR_SIZE)"; "           \
		"setexpr offset ${fdt_start} * "                               \
			__stringify(TQMA6_SPI_FLASH_SECTOR_SIZE)"; "           \
		"sf read ${fdt_addr} ${offset} ${size}; "                      \
		"setenv size ; setenv offset\0"                                \

#define CONFIG_BOOTCOMMAND                                                     \
	"sf probe; run mmcboot; run netboot; run panicboot"                    \

#else

#error "need to define boot source"

#endif

/* 128 MiB offset as in ARM related docu for linux suggested */
#define TQMA6_FDT_ADDRESS		0x18000000

/* set to a resonable value, changeable by user */
#define TQMA6_CMA_SIZE                 160M

#if defined(CONFIG_DEFAULT_FDT_FILE)
#define TQMA6_FDT_FILE_ENV	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0"
#else
#define TQMA6_FDT_FILE_ENV
#endif

#define CONFIG_EXTRA_ENV_SETTINGS                                              \
	"board=tqma6\0"                                                        \
	"uimage=uImage\0"                                                      \
	"zimage=linuximage\0"                                                  \
	"boot_type=bootz\0"                                                    \
	"kernel_name=if test \"${boot_type}\" != bootz; then "                 \
		"setenv kernel ${uimage}; "                                    \
		"else setenv kernel ${zimage}; fi\0"                           \
	"uboot=u-boot.imx\0"                                                   \
	TQMA6_FDT_FILE_ENV                                                     \
	"fdt_addr="__stringify(TQMA6_FDT_ADDRESS)"\0"                          \
	"console=" CONSOLE_DEV "\0"                                            \
	"cma_size="__stringify(TQMA6_CMA_SIZE)"\0"                             \
	"fdt_high=0xffffffff\0"                                                \
	"initrd_high=0xffffffff\0"                                             \
	"rootfsmode=ro\0"                                                      \
	"addcma=setenv bootargs ${bootargs} cma=${cma_size}\0"                 \
	"addtty=setenv bootargs ${bootargs} console=${console},${baudrate}\0"  \
	"fbdepth=32\0"                                                         \
	"fbdrv=imxdrm\0"                                                       \
	"addfb=setenv bootargs ${bootargs} "                                   \
		"${fbdrv}.legacyfb_depth=${fbdepth} consoleblank=0\0"          \
	"mmcpart=2\0"                                                          \
	"mmcblkdev=0\0"                                                        \
	"mmcargs=run addmmc addtty addfb addcma\0"                             \
	"addmmc=setenv bootargs ${bootargs} "                                  \
		"root=/dev/mmcblk${mmcblkdev}p${mmcpart} ${rootfsmode} "       \
		"rootwait\0"                                                   \
	"mmcboot=echo Booting from mmc ...; "                                  \
		"setenv bootargs; "                                            \
		"run mmcargs; "                                                \
		"if run loadfdt; then "                                        \
			"echo boot device tree kernel ...; "                   \
			"if run loadimage; then "                              \
				"${boot_type} ${loadaddr} - ${fdt_addr}; "     \
			"fi; "                                                 \
		"else "                                                        \
			"if run loadimage; then "                              \
				"${boot_type}; "                               \
			"fi; "                                                 \
		"fi;\0"                                                        \
		"setenv bootargs \0"                                           \
	"netdev=eth0\0"                                                        \
	"rootpath=/srv/nfs/tqma6\0"                                            \
	"ipmode=static\0"                                                      \
	"netargs=run addnfs addip addtty addfb addcma\0"                       \
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
		"run kernel_name; "                                            \
		"run set_getcmd; "                                             \
		"setenv bootargs; "                                            \
		"run netargs; "                                                \
		"if ${getcmd} ${fdt_addr} ${fdt_file}; then "                  \
			"if ${getcmd} ${loadaddr} ${kernel}; then "            \
				"${boot_type} ${loadaddr} - ${fdt_addr}; "     \
			"fi; "                                                 \
		"fi; "                                                         \
		"echo ... failed\0"                                            \
	"panicboot=echo No boot device !!! reset\0"                            \
	TQMA6_EXTRA_BOOTDEV_ENV_SETTINGS                                       \

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#if !defined(CONFIG_SYS_BOOTM_LEN) || (CONFIG_SYS_BOOTM_LEN < SZ_16M)
#undef CONFIG_SYS_BOOTM_LEN
#define CONFIG_SYS_BOOTM_LEN 		SZ_16M
#endif

/*
 * MEMTEST configuration
 */

#if defined(CONFIG_CMD_MEMTEST)

/*
 * config alternate mtest:
 * enable 3/4 of RAM to test
 * U-Boot is relocated to the end of RAM
 */

#if !defined(__ASSEMBLY__)
unsigned imx_ddr_size(void);
#endif

#define CONFIG_SYS_ALT_MEMTEST

#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE

#ifdef CONFIG_TQMA6QDL
#define CONFIG_SYS_MEMTEST_END	(CONFIG_SYS_MEMTEST_START + \
				((SZ_512M / 4) * 3))
#else
#define CONFIG_SYS_MEMTEST_END	(CONFIG_SYS_MEMTEST_START + \
				((imx_ddr_size() / 4) * 3))
#endif

#define CONFIG_SYS_MEMTEST_SCRATCH	CONFIG_SYS_MEMTEST_END

#endif /* CONFIG_CMD_MEMTEST */

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

/* Support at least the sensor on TQMa6 SOM */

#endif /* __CONFIG_H */
