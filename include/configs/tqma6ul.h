/*
 * Copyright (C) 2016 TQ Systems
 * Author: Marco Felsch <Marco.Felsch@tq-group.com>
 * Configuration settings for the TQ Systems TQMa6UL SOM.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __TQMA6UL_CONFIG_H
#define __TQMA6UL_CONFIG_H

#include <linux/kconfig.h>

/* SPL */
#define CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SPL_SPI_SUPPORT
#define CONFIG_SPL_FAT_SUPPORT
#define CONFIG_SPL_EXT_SUPPORT

/* common IMX6 SPL configuration */
#include "imx6_spl.h"

#include <asm/arch/imx-regs.h>
#include <asm/imx-common/gpio.h>
#include <linux/sizes.h>
#include "mx6_common.h"

#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

/* support fused HW units */
#define CONFIG_MODULE_FUSE
#define CONFIG_OF_SYSTEM_SETUP

/* uncomment for SECURE mode support */
/* #define CONFIG_SECURE_BOOT */

#ifdef CONFIG_SECURE_BOOT
#ifndef CONFIG_CSF_SIZE
#define CONFIG_CSF_SIZE 0x4000
#endif
#endif

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_BOARD_LATE_INIT

#define CONFIG_MXC_GPIO
#define CONFIG_MXC_UART
#define CONFIG_IMX_THERMAL

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX		1
#define CONFIG_BAUDRATE			115200

/* I2C Configs */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_MXC_I2C3		/* enable I2C bus 3 */
#define CONFIG_SYS_I2C_MXC_I2C4		/* enable I2C bus 4 */

/* I2C EEPROM (M24C64 + SE97B) */
#define CONFIG_SYS_I2C_EEPROM_ADDR			0x50
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN			2
/* 16 Bytes for SE97B */
#define CONFIG_SYS_I2C_EEPROM_PAGE_WRITE_BITS		4
#define CONFIG_SYS_I2C_EEPROM_PAGE_WRITE_DELAY_MS	20
#define CONFIG_CMD_EEPROM

/* RTC and time support */
#define CONFIG_CMD_DATE
#define CONFIG_RTC_DS1339
#define CONFIG_SYS_I2C_RTC_ADDR 0x68

/* Quad-SPI */
#define CONFIG_FSL_QSPI
#define CONFIG_QSPI_BASE		QSPI0_BASE_ADDR
#define CONFIG_QSPI_MEMMAP_BASE		QSPI0_AMBA_BASE

/* Quad-SPI Flash */
#define	CONFIG_SPI_FLASH
#define	CONFIG_SPI_FLASH_STMICRO
#define	CONFIG_SPI_FLASH_BAR

#define CONFIG_CMD_SF
#define	CONFIG_SF_DEFAULT_BUS		0
#define	CONFIG_SF_DEFAULT_CS		0
#define	CONFIG_SF_DEFAULT_SPEED		40000000
#define	CONFIG_SF_DEFAULT_MODE		SPI_MODE_0

/* PMIC */
#undef CONFIG_LDO_BYPASS_CHECK
#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_POWER_PFUZE3000
#define CONFIG_PFUZE3000_PMIC_I2C
#define CONFIG_POWER_PFUZE3000_I2C_ADDR	0x08

/* MMC Configs, other options from mx6_common */
#define CONFIG_SYS_FSL_ESDHC_ADDR	0
#define CONFIG_SUPPORT_EMMC_RPMB

/* USB Configs */
#ifdef CONFIG_CMD_USB
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_MX6
#define CONFIG_USB_STORAGE
#define CONFIG_MXC_USB_PORTSC  (PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS   0
#define CONFIG_USB_MAX_CONTROLLER_COUNT 2
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#endif

/* Fuses */
#define CONFIG_MXC_OCOTP
#define CONFIG_CMD_FUSE

#define CONFIG_CMD_EXT2
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_EXT4_WRITE
#define CONFIG_CMD_FAT
#define CONFIG_FAT_WRITE
#define CONFIG_CMD_FS_GENERIC
#define CONFIG_DOS_PARTITION


/* FEC */
#define CONFIG_FEC_MXC
#define CONFIG_PHYLIB
#define CONFIG_MII
#define CONFIG_CMD_MII
#define CONFIG_FEC_DMA_MINALIGN		64


#define CONFIG_CMD_BMODE
#define CONFIG_CMD_BOOTZ

#undef CONFIG_CMD_IMLS
#undef CONFIG_BOOTM_NETBSD
#undef CONFIG_BOOTM_PLAN9
#undef CONFIG_BOOTM_RTEMS

#define CONFIG_BOOTDELAY		3
#define CONFIG_ZERO_BOOTDELAY_CHECK

#undef CONFIG_LOADADDR
#define CONFIG_LOADADDR			0x82000000

/* place code in last 4 MiB of RAM of 256 MiB RAM */
#undef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE		0x8fc00000

#define CONFIG_ENV_SIZE			SZ_8K
/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		SZ_16M

/*
 * above 128 MiB offset as in ARM related docu for linux suggested
 * DTB is loaded at 128 MiB, so use just 16 MiB more
 */
#define TQMA6UL_INITRD_ADDRESS		0x89000000

#define TQMA6UL_MFG_ENV_SETTINGS \
	"mfgtool_args=setenv bootargs console=" CONFIG_CONSOLE_DEV ",115200 "    \
		"rdinit=/linuxrc "                                               \
		"g_mass_storage.stall=0 g_mass_storage.removable=1 "             \
		"g_mass_storage.file=/fat g_mass_storage.ro=1 "                  \
		"g_mass_storage.idVendor=0x066F g_mass_storage.idProduct=0x37FF "\
		"g_mass_storage.iSerialNumber=\"\" "                             \
		"enable_wait_mode=off "                                          \
		"\0"                                                             \
		"initrd_addr="__stringify(TQMA6UL_INITRD_ADDRESS)"\0"            \
		"initrd_high=0xffffffff\0"                                       \
		"bootcmd_mfg=run mfgtool_args;bootz ${loadaddr} ${initrd_addr} ${fdt_addr};\0" \

#if defined(CONFIG_TQMA6UL_MMC_BOOT)

#define CONFIG_ENV_IS_IN_MMC
#define TQMA6UL_UBOOT_OFFSET		SZ_1K
#define TQMA6UL_UBOOT_SECTOR_START	0x2
#define TQMA6UL_UBOOT_SECTOR_COUNT	0x7fe

#define CONFIG_ENV_OFFSET		SZ_1M
#define CONFIG_SYS_MMC_ENV_DEV		0

#define TQMA6UL_EXTRA_BOOTDEV_ENV_SETTINGS                                     \
	"uboot_start="__stringify(TQMA6UL_UBOOT_SECTOR_START)"\0"              \
	"uboot_size="__stringify(TQMA6UL_UBOOT_SECTOR_COUNT)"\0"               \
	"mmcdev="__stringify(CONFIG_SYS_MMC_ENV_DEV)"\0"                       \
	"firmwarepart=1\0"                                                     \
	"loadimage=run kernel_name; "                                          \
		"load mmc ${mmcdev}:${firmwarepart} ${loadaddr} ${kernel} \0"  \
	"loadfdtsingle="                                                       \
		"load mmc ${mmcdev}:${firmwarepart} ${fdt_addr} ${fdt_file} \0"\
	"loadfdtfit="                                                          \
		"load mmc ${mmcdev}:${firmwarepart} ${loadaddr} ${fdt_file}; " \
		"imxtract ${loadaddr} ${fitfdt_part} ${fdt_addr} \0"           \
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
	"update_fdt=run fdt_name; run set_getcmd; "                            \
		"if ${getcmd} ${fdtimg}; then "                                \
			"if itest ${filesize} > 0; then "                      \
				"echo Write fdt image to mmc ${mmcdev}:${firmwarepart}...; " \
				"save mmc ${mmcdev}:${firmwarepart} ${loadaddr} " \
					"${fdt_file} ${filesize}; "            \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv getcmd; setenv fdtimg \0"             \

#define CONFIG_BOOTCOMMAND                                                     \
	"run mmcboot; run netboot; run panicboot"

#elif defined(CONFIG_TQMA6UL_QSPI_BOOT)

#define TQMA6UL_SPI_FLASH_SECTOR_SIZE	SZ_64K

#define TQMA6UL_UBOOT_OFFSET		SZ_4K
#define TQMA6UL_UBOOT_SECTOR_START	0x0
/* max u-boot size: 512k */
#define TQMA6UL_UBOOT_SECTOR_SIZE	TQMA6UL_SPI_FLASH_SECTOR_SIZE
#define TQMA6UL_UBOOT_SECTOR_COUNT	0x8
#define TQMA6UL_UBOOT_SIZE		(TQMA6UL_UBOOT_SECTOR_SIZE * \
					 TQMA6UL_UBOOT_SECTOR_COUNT)

#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_OFFSET		(TQMA6UL_UBOOT_SIZE)
#define CONFIG_ENV_SECT_SIZE		TQMA6UL_SPI_FLASH_SECTOR_SIZE
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + \
					 CONFIG_ENV_SECT_SIZE)

#define CONFIG_ENV_SPI_BUS		(CONFIG_SF_DEFAULT_BUS)
#define CONFIG_ENV_SPI_CS		(CONFIG_SF_DEFAULT_CS)
#define CONFIG_ENV_SPI_MAX_HZ		(CONFIG_SF_DEFAULT_SPEED)
#define CONFIG_ENV_SPI_MODE		(CONFIG_SF_DEFAULT_MODE)

#define TQMA6UL_FDT_OFFSET		(CONFIG_ENV_OFFSET_REDUND + \
					 CONFIG_ENV_SECT_SIZE)
#define TQMA6UL_FDT_SECT_SIZE		(TQMA6UL_SPI_FLASH_SECTOR_SIZE)

#define TQMA6UL_FDT_SECTOR_START	0x0a /* 8 Sector u-boot, 2 Sector env */
#define TQMA6UL_FDT_SECTOR_COUNT	0x01

#define TQMA6UL_KERNEL_SECTOR_START	0x10
#define TQMA6UL_KERNEL_SECTOR_COUNT	0x60

#define TQMA6UL_EXTRA_BOOTDEV_ENV_SETTINGS                                     \
	"mmcblkdev=0\0"                                                        \
	"uboot_offset="__stringify(TQMA6UL_UBOOT_OFFSET)"\0"                   \
	"uboot_sectors="__stringify(TQMA6UL_UBOOT_SECTOR_COUNT)"\0"            \
	"fdt_start="__stringify(TQMA6UL_FDT_SECTOR_START)"\0"                  \
	"fdt_sectors="__stringify(TQMA6UL_FDT_SECTOR_COUNT)"\0"                \
	"kernel_start="__stringify(TQMA6UL_KERNEL_SECTOR_START)"\0"            \
	"kernel_sectors="__stringify(TQMA6UL_KERNEL_SECTOR_COUNT)"\0"          \
	"update_uboot=run set_getcmd; "                                        \
		"if ${getcmd} ${uboot}; then "                                 \
			"if itest ${filesize} > 0; then "                      \
				"setexpr blkc ${filesize} + "                  \
					__stringify(TQMA6UL_UBOOT_OFFSET) "; " \
				"setexpr size ${uboot_sectors} * "             \
					__stringify(TQMA6UL_SPI_FLASH_SECTOR_SIZE)"; " \
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
					__stringify(TQMA6UL_SPI_FLASH_SECTOR_SIZE)"; " \
				"setexpr offset ${kernel_start} * "            \
					__stringify(TQMA6UL_SPI_FLASH_SECTOR_SIZE)"; " \
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
	"update_fdt=run fdt_name; run set_getcmd; "                            \
		"if ${getcmd} ${fdtimg}; then "                                \
			"if itest ${filesize} > 0; then "                      \
				"setexpr size ${fdt_sectors} * "               \
					__stringify(TQMA6UL_SPI_FLASH_SECTOR_SIZE)"; " \
				"setexpr offset ${fdt_start} * "               \
					__stringify(TQMA6UL_SPI_FLASH_SECTOR_SIZE)"; " \
				"if itest ${filesize} <= ${size}; then "       \
					"sf probe; "                           \
					"sf erase ${offset} ${size}; "         \
					"sf write ${loadaddr} ${offset} "      \
						"${filesize}; "                \
				"fi; "                                         \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv fdtimg; setenv getcmd; "                               \
		"setenv filesize 0; setenv size; setenv offset\0"              \
	"loadimage=sf probe; "                                                 \
		"setexpr size ${kernel_sectors} * "                            \
			__stringify(TQMA6UL_SPI_FLASH_SECTOR_SIZE)"; "         \
		"setexpr offset ${kernel_start} * "                            \
			__stringify(TQMA6UL_SPI_FLASH_SECTOR_SIZE)"; "         \
		"sf read ${loadaddr} ${offset} ${size}; "                      \
		"setenv size ; setenv offset\0"                                \
	"loadfdtsingle=sf probe; "                                             \
		"setexpr size ${fdt_sectors} * "                               \
			__stringify(TQMA6UL_SPI_FLASH_SECTOR_SIZE)"; "         \
		"setexpr offset ${fdt_start} * "                               \
			__stringify(TQMA6UL_SPI_FLASH_SECTOR_SIZE)"; "         \
		"sf read ${fdt_addr} ${offset} ${size}; "                      \
		"setenv size ; setenv offset\0"                                \
	"loadfdtfit=sf probe; "                                                \
		"setexpr size ${fdt_sectors} * "                               \
			__stringify(TQMA6UL_SPI_FLASH_SECTOR_SIZE)"; "         \
		"setexpr offset ${fdt_start} * "                               \
			__stringify(TQMA6UL_SPI_FLASH_SECTOR_SIZE)"; "         \
		"sf read ${loadaddr} ${offset} ${size}; "                      \
		"setenv size ; setenv offset "                                 \
		"imxtract ${loadaddr} ${fitfdt_part} ${fdt_addr}\0"            \

#define CONFIG_BOOTCOMMAND                                                     \
	"sf probe; run mmcboot; run netboot; run panicboot"                    \

#else

#error "need to define boot source"

#endif

/* 128 MiB offset as in ARM related docu for linux suggested */
#define TQMA6UL_FDT_ADDRESS		0x88000000

/* set to a resonable value, changeable by user */
#define TQMA6UL_CMA_SIZE		32M

#define CONFIG_EXTRA_ENV_SETTINGS \
	"board=tqma6ul\0"                                                      \
	"uimage=uImage\0"                                                      \
	"zimage=linuximage\0"                                                  \
	"boot_type=bootz\0"                                                    \
	"kernel_name=if test \"${boot_type}\" != bootz; then "                 \
		"setenv kernel ${uimage}; "                                    \
		"else setenv kernel ${zimage}; fi\0"                           \
	"uboot=u-boot.imx\0"                                                   \
	"fdt_type=single\0"                                                    \
	"fitfdt_file=" CONFIG_DEFAULT_FDT_FILE ".fit\0"                        \
	"fitfdt_part=fdt@0\0"                                                  \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0"                               \
	"fdt_name=if test \"${fdt_type}\" != single; then "                    \
		"setenv fdtimg ${fitfdt_file}; "                               \
		"else setenv fdtimg ${fdt_file}; fi\0"                         \
	"fdt_addr="__stringify(TQMA6UL_FDT_ADDRESS)"\0"                        \
	"console=" CONFIG_CONSOLE_DEV "\0"                                     \
	"cma_size="__stringify(TQMA6UL_CMA_SIZE)"\0"                           \
	"fdt_high=0xffffffff\0"                                                \
	"initrd_high=0xffffffff\0"                                             \
	"rootfsmode=ro\0"                                                      \
	"addcma=setenv bootargs ${bootargs} cma=${cma_size}\0"                 \
	"addtty=setenv bootargs ${bootargs} console=${console},${baudrate} "   \
		"consoleblank=0\0"                                             \
	"mmcpart=2\0"                                                          \
	"mmcblkdev=0\0"                                                        \
	"mmcargs=run addmmc addtty addcma\0"                                   \
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
	"netdev=eth1\0"                                                        \
	"rootpath=/srv/nfs/tqma6\0"                                            \
	"ipmode=static\0"                                                      \
	"netargs=run addnfs addip addtty addcma\0"                             \
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
	"get_fdt=run fdt_name; "                                               \
		"if test \"${fdt_type}\" != single; then "                     \
			"echo use dtb from fit; "                              \
			"if ${getcmd} ${loadaddr} ${fdtimg}; then "            \
				"imxtract ${loadaddr} ${fitfdt_part} "         \
					"${fdt_addr}; "                        \
			"fi; "                                                 \
		"else "                                                        \
			"echo use dtb; "                                       \
			"${getcmd} ${fdt_addr} ${fdtimg}; "                    \
		"fi; "                                                         \
		"setenv fdtimg\0"                                              \
	"netboot=echo Booting from net ...; "                                  \
		"run kernel_name; "                                            \
		"run set_getcmd; "                                             \
		"setenv bootargs; "                                            \
		"run netargs; "                                                \
		"if run get_fdt; then "                                        \
			"if ${getcmd} ${loadaddr} ${kernel}; then "            \
				"${boot_type} ${loadaddr} - ${fdt_addr}; "     \
			"fi; "                                                 \
		"fi; "                                                         \
		"echo ... failed\0"                                            \
	"panicboot=echo No boot device !!! reset\0"                            \
	"loadfdt=if test \"${fdt_type}\" != single; then "                     \
		"run loadfdtfit; "                                             \
		"else run loadfdtsingle; fi\0"                                 \
	TQMA6UL_EXTRA_BOOTDEV_ENV_SETTINGS                                     \
	TQMA6UL_MFG_ENV_SETTINGS                                               \

/* Miscellaneous configurable options */
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#define CONFIG_SYS_LONGHELP

#define CONFIG_AUTO_COMPLETE

#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

#define CONFIG_CMDLINE_EDITING
#define CONFIG_STACKSIZE		SZ_128K

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
#define CONFIG_SYS_BOOTM_LEN		SZ_16M
#endif

/* FLASH and environment organization */
#define CONFIG_SYS_NO_FLASH

#define CONFIG_OF_LIBFDT
#define CONFIG_OF_BOARD_SETUP
#define CONFIG_FIT
#define CONFIG_FIT_VERBOSE

#ifndef CONFIG_SYS_DCACHE_OFF
#define CONFIG_CMD_CACHE
#endif

/*
 * All the defines above are for the TQMa6UL SoM
 *
 * Now include the baseboard specific configuration
 */
#ifdef CONFIG_MBA6UL
#include "tqma6ul_mba6ul.h"
#else
#error "No baseboard for the TQMa6UL SOM defined!"
#endif

#endif /* __TQMA6UL_CONFIG_H */
