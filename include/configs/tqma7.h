/*
 * Copyright (C) 2015 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Freescale i.MX7D SABRESD board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __TQMA7_CONFIG_H
#define __TQMA7_CONFIG_H

#include <linux/kconfig.h>

#include <asm/arch/imx-regs.h>
#include <asm/imx-common/gpio.h>
#include <linux/sizes.h>
#include "mx7_common.h"

#define CONFIG_ROM_UNIFIED_SECTIONS
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO
#define CONFIG_SYS_GENERIC_BOARD

#define CONFIG_DBG_MONITOR

/* uncomment for SECURE mode support */
/* #define CONFIG_SECURE_BOOT */

#ifdef CONFIG_SECURE_BOOT
#ifndef CONFIG_CSF_SIZE
#define CONFIG_CSF_SIZE 0x4000
#endif
#endif

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_BOARD_LATE_INIT

#define CONFIG_MXC_GPIO
#define CONFIG_MXC_UART

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX		1
#define CONFIG_BAUDRATE			115200

/* I2C Configs */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
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

/* RTC and time support */
#define CONFIG_CMD_DATE
#define CONFIG_RTC_DS1339
#define CONFIG_SYS_I2C_RTC_ADDR 0x68

/* PMIC */
#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_POWER_PFUZE3000
#define CONFIG_PFUZE3000_PMIC_I2C
#define CONFIG_POWER_PFUZE3000_I2C_ADDR		0x08

/* MMC Configs */
#define CONFIG_FSL_ESDHC
#define CONFIG_FSL_USDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR	0
#define CONFIG_SUPPORT_EMMC_BOOT

#define CONFIG_MMC
#define CONFIG_CMD_MMC
#define CONFIG_GENERIC_MMC
/* TODO: check if needed */
#define CONFIG_BOUNCE_BUFFER

/* USB Configs */
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_MX7
#define CONFIG_USB_STORAGE
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX
#define CONFIG_USB_ETHER_SMSC95XX
#define CONFIG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS   0
#define CONFIG_USB_MAX_CONTROLLER_COUNT	2
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET	/* For OTG port */

/* Fuses */
#define CONFIG_MXC_OCOTP
#define CONFIG_CMD_FUSE

#define CONFIG_CMD_EXT2
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_EXT4_WRITE
#define CONFIG_CMD_FAT
#define CONFIG_CMD_FS_GENERIC
#define CONFIG_DOS_PARTITION


#define PHYS_SDRAM_SIZE			SZ_1G

#define CONFIG_FEC_MXC
#define CONFIG_PHYLIB
#define CONFIG_MII
#define CONFIG_CMD_MII
#define CONFIG_FEC_DMA_MINALIGN		64


/* Command definition */
#include <config_cmd_default.h>

#define CONFIG_CMD_BMODE
#define CONFIG_CMD_BOOTZ
#define CONFIG_CMD_ITEST
#define CONFIG_CMD_SETEXPR
#undef CONFIG_CMD_IMLS

#define CONFIG_BOOTDELAY		3

#define CONFIG_LOADADDR			0x82000000

/* place code in last 4 MiB of RAM of 512 MiB RAM (minimal variant) */
#define CONFIG_SYS_TEXT_BASE		0x9fc00000

#define CONFIG_ENV_SIZE			(SZ_8K)
/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(32 * SZ_1M)

#if defined(CONFIG_TQMA7_MMC_BOOT)

#define CONFIG_ENV_IS_IN_MMC
#define TQMA7_UBOOT_OFFSET		SZ_1K
#define TQMA7_UBOOT_SECTOR_START	0x2
#define TQMA7_UBOOT_SECTOR_COUNT	0x7fe

#define CONFIG_ENV_OFFSET		SZ_1M
#define CONFIG_SYS_MMC_ENV_DEV		0

#define TQMA7_FDT_OFFSET		(2 * SZ_1M)
#define TQMA7_FDT_SECTOR_START		0x1000
#define TQMA7_FDT_SECTOR_COUNT		0x800

#define TQMA7_KERNEL_SECTOR_START	0x2000
#define TQMA7_KERNEL_SECTOR_COUNT	0x2000

#define TQMA7_EXTRA_BOOTDEV_ENV_SETTINGS                                       \
	"uboot_start="__stringify(TQMA7_UBOOT_SECTOR_START)"\0"                \
	"uboot_size="__stringify(TQMA7_UBOOT_SECTOR_COUNT)"\0"                 \
	"fdt_start="__stringify(TQMA7_FDT_SECTOR_START)"\0"                    \
	"fdt_size="__stringify(TQMA7_FDT_SECTOR_COUNT)"\0"                     \
	"kernel_start="__stringify(TQMA7_KERNEL_SECTOR_START)"\0"              \
	"kernel_size="__stringify(TQMA7_KERNEL_SECTOR_COUNT)"\0"               \
	"mmcdev="__stringify(CONFIG_SYS_MMC_ENV_DEV)"\0"                       \
	"loadimage=mmc dev ${mmcdev}; "                                        \
		"mmc read ${loadaddr} ${kernel_start} ${kernel_size};\0"       \
	"loadfdtsingle=mmc dev ${mmcdev}; "                                    \
		"mmc read ${fdt_addr} ${fdt_start} ${fdt_size};\0"             \
	"loadfdtfit=mmc dev ${mmcdev}; "                                       \
		"mmc read ${loadaddr} ${fdt_start} ${fdt_size}; "              \
		"imxtract ${loadaddr} ${fitfdt_part} ${fdt_addr}\0"            \
	"update_uboot=if tftp ${uboot}; then "                                 \
		"if itest ${filesize} > 0; then "                              \
			"mmc dev ${mmcdev}; mmc rescan; "                      \
			"setexpr blkc ${filesize} + 0x1ff; "                   \
			"setexpr blkc ${blkc} / 0x200; "                           \
			"if itest ${blkc} <= ${uboot_size}; then "             \
				"mmc write ${loadaddr} ${uboot_start} "        \
					"${blkc}; "                            \
			"fi; "                                                 \
		"fi; fi; "                                                     \
		"setenv filesize; setenv blkc \0"                              \
	"update_kernel=run kernel_name; "                                      \
		"if tftp ${kernel}; then "                                     \
			"if itest ${filesize} > 0; then "                      \
				"mmc dev ${mmcdev}; mmc rescan; "              \
				"setexpr blkc ${filesize} + 0x1ff; "                   \
				"setexpr blkc ${blkc} / 0x200; "                           \
				"if itest ${blkc} <= ${kernel_size}; then "    \
					"mmc write ${loadaddr} "               \
						"${kernel_start} ${blkc}; "    \
				"fi; "                                         \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv blkc \0"                              \
	"update_fdt=run fdt_name; if tftp ${fdtimg}; then "                    \
		"if itest ${filesize} > 0; then "                              \
			"mmc dev ${mmcdev}; mmc rescan; "                      \
			"setexpr blkc ${filesize} + 0x1ff; "                   \
			"setexpr blkc ${blkc} / 0x200; "                           \
			"if itest ${blkc} <= ${fdt_size}; then "               \
				"mmc write ${loadaddr} ${fdt_start} ${blkc}; " \
			"fi; "                                                 \
		"fi; fi; "                                                     \
		"setenv filesize; setenv blkc \0"                              \

#define CONFIG_BOOTCOMMAND \
	"run mmcboot; run netboot; run panicboot"

#else

#error "need to define boot source"

#endif

/* 128 MiB offset as in ARM related docu for linux suggested */
#define TQMA_FDT_ADDRESS		0x18000000

#undef CONFIG_BOOTM_NETBSD
#undef CONFIG_BOOTM_PLAN9
#undef CONFIG_BOOTM_RTEMS

#ifdef CONFIG_SYS_BOOT_QSPI
#define CONFIG_SYS_USE_QSPI
#define CONFIG_ENV_IS_IN_SPI_FLASH
#elif defined CONFIG_SYS_BOOT_NAND
#define CONFIG_SYS_USE_NAND
#define CONFIG_ENV_IS_IN_NAND
#else
#define CONFIG_ENV_IS_IN_MMC
#endif

#ifdef CONFIG_SYS_USE_QSPI
#define CONFIG_SYS_AUXCORE_BOOTDATA 0x60100000 /* Set to QSPI1 A flash, offset 1M */
#else
#define CONFIG_SYS_AUXCORE_BOOTDATA 0x7F8000 /* Set to TCML address */
#endif
#define CONFIG_CMD_BOOTAUX /* Boot M4 */
#define CONFIG_CMD_SETEXPR

#ifdef CONFIG_CMD_BOOTAUX
#define CONFIG_MXC_RDC /* Enable RDC to isolate the peripherals for A7 and M4 */

#ifdef CONFIG_SYS_USE_QSPI
#define UPDATE_M4_ENV \
	"m4image=m4_qspi.bin\0" \
	"loadm4image=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${m4image}\0" \
	"update_m4_from_sd=" \
		"if sf probe 0:0; then " \
			"if run loadm4image; then " \
				"setexpr fw_sz ${filesize} + 0xffff; " \
				"setexpr fw_sz ${fw_sz} / 0x10000; "	\
				"setexpr fw_sz ${fw_sz} * 0x10000; "	\
				"sf erase 0x100000 ${fw_sz}; " \
				"sf write ${loadaddr} 0x100000 ${filesize}; " \
			"fi; " \
		"fi\0" \
	"m4boot=sf probe 0:0; bootaux "__stringify(CONFIG_SYS_AUXCORE_BOOTDATA)"\0"
#else
#define UPDATE_M4_ENV \
	"m4image=m4_qspi.bin\0" \
	"loadm4image=fatload mmc ${mmcdev}:${mmcpart} "__stringify(CONFIG_SYS_AUXCORE_BOOTDATA)" ${m4image}\0" \
	"m4boot=run loadm4image; bootaux "__stringify(CONFIG_SYS_AUXCORE_BOOTDATA)"\0"
#endif
#else
#define UPDATE_M4_ENV ""
#endif

/* 128 MiB offset as in ARM related docu for linux suggested */
#define TQMA7_FDT_ADDRESS		0x88000000

/* set to a resonable value, changeable by user */
#define TQMA7_CMA_SIZE                 64M

#define CONFIG_EXTRA_ENV_SETTINGS \
	UPDATE_M4_ENV \
	"board=tqma7\0"                                                        \
	"uimage=uImage\0"                                                      \
	"zimage=zImage\0"                                                      \
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
	"fdt_addr="__stringify(TQMA7_FDT_ADDRESS)"\0"                          \
	"console=" CONFIG_CONSOLE_DEV "\0"                                     \
	"cma_size="__stringify(TQMA6_CMA_SIZE)"\0"                             \
	"fdt_high=0xffffffff\0"                                                \
	"initrd_high=0xffffffff\0"                                             \
	"rootfsmode=ro\0"                                                      \
	"addcma=setenv bootargs ${bootargs} cma=${cma_size}\0"                 \
	"addtty=setenv bootargs ${bootargs} console=${console},${baudrate}\0"  \
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
	TQMA7_EXTRA_BOOTDEV_ENV_SETTINGS                                       \

/* Miscellaneous configurable options */
#define CONFIG_SYS_LONGHELP
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_CBSIZE		1024

#define CONFIG_SYS_MAXARGS		256
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

#define CONFIG_CMD_MEMTEST
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 0x20000000)

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

#define CONFIG_ZERO_BOOTDELAY_CHECK

#if !defined(CONFIG_SYS_BOOTM_LEN) || (CONFIG_SYS_BOOTM_LEN < SZ_16M)
#undef CONFIG_SYS_BOOTM_LEN
#define CONFIG_SYS_BOOTM_LEN		SZ_16M
#endif

/* FLASH and environment organization */
#define CONFIG_SYS_NO_FLASH

#ifdef CONFIG_SYS_USE_QSPI
#define CONFIG_FSL_QSPI    /* enable the QUADSPI driver */
#define CONFIG_QSPI_BASE		QSPI1_IPS_BASE_ADDR
#define CONFIG_QSPI_MEMMAP_BASE		QSPI0_ARB_BASE_ADDR

#define CONFIG_CMD_SF
#define	CONFIG_SPI_FLASH
#define	CONFIG_SPI_FLASH_MACRONIX
#define	CONFIG_SPI_FLASH_BAR
#define	CONFIG_SF_DEFAULT_BUS		0
#define	CONFIG_SF_DEFAULT_CS		0
#define	CONFIG_SF_DEFAULT_SPEED		40000000
#define	CONFIG_SF_DEFAULT_MODE		SPI_MODE_0
#endif

#if defined(CONFIG_ENV_IS_IN_MMC)
#
#elif defined(CONFIG_ENV_IS_IN_SPI_FLASH)
#define CONFIG_ENV_OFFSET		(768 * 1024)
#define CONFIG_ENV_SECT_SIZE		(64 * 1024)
#define CONFIG_ENV_SPI_BUS		CONFIG_SF_DEFAULT_BUS
#define CONFIG_ENV_SPI_CS		CONFIG_SF_DEFAULT_CS
#define CONFIG_ENV_SPI_MODE		CONFIG_SF_DEFAULT_MODE
#define CONFIG_ENV_SPI_MAX_HZ		CONFIG_SF_DEFAULT_SPEED
#endif

#ifdef CONFIG_SYS_USE_NAND
#define CONFIG_SYS_FSL_USDHC_NUM	1
#else
#define CONFIG_SYS_FSL_USDHC_NUM	2
#endif
#define CONFIG_SYS_MMC_ENV_DEV		0   /* USDHC1 */
#define CONFIG_SYS_MMC_ENV_PART		0	/* user area */
#define CONFIG_MMCROOT			"/dev/mmcblk0p2"  /* USDHC1 */

#define CONFIG_ZERO_BOOTDELAY_CHECK

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

#define CONFIG_SYS_CONSOLE_IS_IN_ENV


#define CONFIG_IMX_THERMAL

#if defined(CONFIG_MXC_EPDC) && defined(CONFIG_SYS_USE_QSPI)
#error "EPDC Pins conflicts QSPI, Either EPDC or QSPI can be enabled!"
#endif

/*
 * All the defines above are for the TQMa7 SoM
 *
 * Now include the baseboard specific configuration
 */
#ifdef CONFIG_MBA7
#include "tqma7_mba7.h"
#else
#error "No baseboard for the TQMa7 SOM defined!"
#endif

/* Support at least the sensor on TQMa SOM */
#if !defined(CONFIG_DTT_SENSORS)
#define CONFIG_DTT_SENSORS		{ 0 }
#endif

#endif /* __TQMA7_CONFIG_H */
