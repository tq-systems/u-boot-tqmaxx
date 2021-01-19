/*
 * Copyright (C) 2013, 2014 Markus Niebel <Markus.Niebel@tq-group.com>
 *
 * Configuration settings for the TQ Systems TQMa6<Q,S> module.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/kconfig.h>
/* SPL */
/* #if defined(CONFIG_SPL_BUILD) */

#define CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SPL_SPI_SUPPORT
#define CONFIG_SPL_FAT_SUPPORT
#define CONFIG_SPL_EXT_SUPPORT

/* common IMX6 SPL configuration */
#include "imx6_spl.h"

/* #endif */

/* place code in last 4 MiB of RAM */
#if defined(CONFIG_TQMA6S)
#define CONFIG_SYS_TEXT_BASE		0x2fc00000
#elif defined(CONFIG_TQMA6Q) || defined(CONFIG_TQMA6DL)
#define CONFIG_SYS_TEXT_BASE		0x4fc00000
#elif defined(CONFIG_TQMA6Q_2GB)
#define CONFIG_SYS_TEXT_BASE		0x8fc00000
#endif

#include "mx6_common.h"

#undef CONFIG_LDO_BYPASS_CHECK

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_BOARD_LATE_INIT

#define CONFIG_MXC_UART

/* SPI */
#define CONFIG_CMD_SPI
#define CONFIG_MXC_SPI

/* SPI Flash */
#define TQMA6_SPI_FLASH_SECTOR_SIZE	SZ_64K

#define CONFIG_CMD_SF
#define CONFIG_SF_DEFAULT_BUS	0
#define CONFIG_SF_DEFAULT_CS	0
#define CONFIG_SF_DEFAULT_SPEED	50000000
#define CONFIG_SF_DEFAULT_MODE	(SPI_MODE_0)

/* I2C Configs */
#define CONFIG_CMD_I2C
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_MXC_I2C3		/* enable I2C bus 3 */
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_SYS_I2C_SPEED		100000

/* I2C SYSMON (LM75) */
#define CONFIG_DTT_LM75
#define CONFIG_DTT_MAX_TEMP		70
#define CONFIG_DTT_MIN_TEMP		-30
#define CONFIG_DTT_HYSTERESIS	3
#define CONFIG_CMD_DTT

/* I2C EEPROM (M24C64) */
#define CONFIG_SYS_I2C_EEPROM_ADDR			0x50
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN			2
#define CONFIG_SYS_I2C_EEPROM_PAGE_WRITE_BITS		5 /* 32 Bytes */
#define CONFIG_SYS_I2C_EEPROM_PAGE_WRITE_DELAY_MS	20
#define CONFIG_CMD_EEPROM

#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_POWER_PFUZE100
#define CONFIG_POWER_PFUZE100_I2C_ADDR	0x08

#if defined(CONFIG_FSL_CAAM)

/* TODO: this is part of Kconfig in Mainline */
#define CONFIG_SYS_FSL_SEC_COMPAT	4
#define CONFIG_SYS_FSL_SEC_LE

/*
 * TODO: hashing support not per Kconfig to prevent warnings for
 * redefining in image.h
 */
#if !defined(CONFIG_SHA1)
#define CONFIG_SHA1		/* and SHA1 */
#endif
#if !defined(CONFIG_SHA1)
#define CONFIG_SHA256		/* and SHA256 */
#endif

#define CONFIG_CMD_HASH
#define CONFIG_HASH_VERIFY

#endif

/* MMC Configs, other options from mx6_common */
#define CONFIG_SYS_FSL_ESDHC_ADDR	0
#define CONFIG_SUPPORT_EMMC_RPMB

/* USB Configs */
#define CONFIG_CMD_USB
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_MX6
#define CONFIG_USB_STORAGE
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_SMSC95XX
#define CONFIG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_USB_MAX_CONTROLLER_COUNT	2
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET	/* For OTG port */

#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_MII

#define CONFIG_FEC_MXC
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_PHYLIB
#define CONFIG_MII

#define CONFIG_ARP_TIMEOUT		200UL

/* Command definition */
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_EXT4_WRITE
#define CONFIG_CMD_FAT
#define CONFIG_FAT_WRITE
#define CONFIG_CMD_FS_GENERIC
#define CONFIG_DOS_PARTITION

#define CONFIG_CMD_BMODE

#define CONFIG_ENV_SIZE			(SZ_8K)
/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 2 * SZ_1M)

/* above 128 MiB offset as in ARM related docu for linux suggested
 * DTB is loaded at 128 MiB, so use just 16 MiB more
 */
#define TQMA6_INITRD_ADDRESS		0x19000000

#define TQMA6_MFG_ENV_SETTINGS \
	"mfgtool_args=setenv bootargs console=" CONFIG_CONSOLE_DEV ",115200 "    \
		"rdinit=/linuxrc "                                               \
		"g_mass_storage.stall=0 g_mass_storage.removable=1 "             \
		"g_mass_storage.file=/fat g_mass_storage.ro=1 "                  \
		"g_mass_storage.idVendor=0x066F g_mass_storage.idProduct=0x37FF "\
		"g_mass_storage.iSerialNumber=\"\" "                             \
		"enable_wait_mode=off "                                          \
		"\0"                                                             \
		"initrd_addr="__stringify(TQMA6_INITRD_ADDRESS)"\0"              \
		"initrd_high=0xffffffff\0"                                       \
		"bootcmd_mfg=run mfgtool_args;bootz ${loadaddr} ${initrd_addr} ${fdt_addr};\0" \

#if defined(CONFIG_TQMA6X_MMC_BOOT)

#define CONFIG_ENV_IS_IN_MMC
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
				"mmc dev ${mmcdev}; mmc rescan; "              \
				"echo Write kernel image to mmc ${mmcdev}:${firmwarepart}...; " \
				"save mmc ${mmcdev}:${firmwarepart} ${loadaddr} " \
					"${kernel} ${filesize}; "              \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv getcmd \0"                            \
	"update_fdt=run set_getcmd; "                                          \
		"if ${getcmd} ${fdt_file}; then "                              \
			"if itest ${filesize} > 0; then "                      \
				"mmc dev ${mmcdev}; mmc rescan; "              \
				"echo Write fdt image to mmc ${mmcdev}:${firmwarepart}...; " \
				"save mmc ${mmcdev}:${firmwarepart} ${loadaddr} " \
					"${fdt_file} ${filesize}; "            \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv getcmd \0"                            \

#define CONFIG_BOOTCOMMAND \
	"run mmcboot; run netboot; run panicboot"

#elif defined(CONFIG_TQMA6X_SPI_BOOT)

#define TQMA6_UBOOT_OFFSET		0x400
#define TQMA6_UBOOT_SECTOR_START	0x0
/* max u-boot size: 512k */
#define TQMA6_UBOOT_SECTOR_SIZE		TQMA6_SPI_FLASH_SECTOR_SIZE
#define TQMA6_UBOOT_SECTOR_COUNT	0x8
#define TQMA6_UBOOT_SIZE		(TQMA6_UBOOT_SECTOR_SIZE * \
					 TQMA6_UBOOT_SECTOR_COUNT)

#define CONFIG_ENV_IS_IN_SPI_FLASH
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
#define TQMA6_KERNEL_SECTOR_COUNT	0xa0 /* size 10 MB */

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
	"console=" CONFIG_CONSOLE_DEV "\0"                                     \
	"cma_size="__stringify(TQMA6_CMA_SIZE)"\0"                             \
	"fdt_high=0xffffffff\0"                                                \
	"initrd_high=0xffffffff\0"                                             \
	"rootfsmode=ro\0"                                                      \
	"addcma=setenv bootargs ${bootargs} cma=${cma_size}\0"                 \
	"addtty=setenv bootargs ${bootargs} console=${console},${baudrate}\0"  \
	"addfb=setenv bootargs ${bootargs} consoleblank=0\0"                   \
	"mmcpart=2\0"                                                          \
	"mmcblkdev=0\0"                                                        \
	"mmcargs=run addmmc addtty addfb addcma\0"                             \
	"addmmc=setenv bootargs ${bootargs} "                                  \
		"root=/dev/mmcblk${mmcblkdev}p${mmcpart} ${rootfsmode} "       \
		"rootwait\0"                                                   \
	"mmcboot=echo Booting from mmc ...; "                                  \
		"setenv bootargs; "                                            \
		"run mmcargs; "                                                \
		"if run loadimage; then "                                      \
			"if run loadfdt; then "                                \
				"echo boot device tree kernel ...; "           \
				"${boot_type} ${loadaddr} - ${fdt_addr}; "     \
			"fi; "                                                 \
		"else "                                                        \
			"${boot_type}; "                                       \
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
	TQMA6_MFG_ENV_SETTINGS                                                 \

#define CONFIG_STACKSIZE		(128u * SZ_1K)

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

#define CONFIG_ZERO_BOOTDELAY_CHECK

#if !defined(CONFIG_SYS_BOOTM_LEN) || (CONFIG_SYS_BOOTM_LEN < SZ_16M)
#undef CONFIG_SYS_BOOTM_LEN
#define CONFIG_SYS_BOOTM_LEN		SZ_16M
#endif

#define CONFIG_OF_LIBFDT
#define CONFIG_OF_BOARD_SETUP
#define CONFIG_FIT
#define CONFIG_FIT_VERBOSE

#define CONFIG_IMX_THERMAL

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
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + \
					((imx_ddr_size() / 4) * 3))

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
#if !defined(CONFIG_DTT_SENSORS)
#define CONFIG_DTT_SENSORS		{ 0 }
#endif

#if defined(CONFIG_SECURE_BOOT)
#if !defined(CONFIG_CSF_SIZE)
/* TODO: this is 0x2000 in Mainline */
#define CONFIG_CSF_SIZE		0x4000
#endif
#endif

#endif /* __CONFIG_H */
