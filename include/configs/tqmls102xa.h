/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Copyright 2015 TQ Systems GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

#define CONFIG_LS102XA

#define CONFIG_ARMV7_PSCI

#define CONFIG_SYS_FSL_CLK

#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_BOARD_EARLY_INIT_F

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 16 * 1024 * 1024)

#define CONFIG_SYS_INIT_RAM_ADDR	OCRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	OCRAM_SIZE

/*
 * USB
 */

/*
 * EHCI Support - disbaled by default as
 * there is no signal coming out of soc on
 * this board for this controller. However,
 * the silicon still has this controller,
 * and anyone can use this controller by
 * taking signals out on their board.
 */

/*#define CONFIG_HAS_FSL_DR_USB*/

#ifdef CONFIG_HAS_FSL_DR_USB
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_FSL
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#endif

/* XHCI Support - enabled by default */
#define CONFIG_HAS_FSL_XHCI_USB

#ifdef CONFIG_HAS_FSL_XHCI_USB
#define CONFIG_USB_XHCI_FSL
#define CONFIG_USB_XHCI_DWC3
#define CONFIG_USB_XHCI
#define CONFIG_USB_MAX_CONTROLLER_COUNT        1
#define CONFIG_SYS_USB_XHCI_MAX_ROOT_PORTS     2
#endif

#if defined(CONFIG_HAS_FSL_DR_USB) || defined(CONFIG_HAS_FSL_XHCI_USB)
#define CONFIG_CMD_USB
#define CONFIG_USB_STORAGE
#define CONFIG_CMD_EXT2
#endif

/*
 * Generic Timer Definitions
 */
#define GENERIC_TIMER_CLK		12500000

#define CONFIG_SYS_CLK_FREQ		100000000
#define CONFIG_DDR_CLK_FREQ		100000000

#define DDR_SDRAM_CFG			0x470c0008
#define DDR_CS0_BNDS			0x008000bf
#define DDR_CS0_CONFIG			0x80014302
#define DDR_TIMING_CFG_0		0x50550004
#define DDR_TIMING_CFG_1		0xbcb38c56
#define DDR_TIMING_CFG_2		0x0040d120
#define DDR_TIMING_CFG_3		0x010e1000
#define DDR_TIMING_CFG_4		0x00000001
#define DDR_TIMING_CFG_5		0x03401400
#define DDR_SDRAM_CFG_2			0x00401010
#define DDR_SDRAM_MODE			0x00061c60
#define DDR_SDRAM_MODE_2		0x00180000
#define DDR_SDRAM_INTERVAL		0x18600618
#define DDR_DDR_WRLVL_CNTL		0x8655f605
#define DDR_DDR_WRLVL_CNTL_2	0x05060607
#define DDR_DDR_WRLVL_CNTL_3	0x05050505
#define DDR_DDR_CDR1			0x80040000
#define DDR_DDR_CDR2			0x00000001
#define DDR_SDRAM_CLK_CNTL		0x02000000
#define DDR_DDR_ZQ_CNTL			0x89080600
#define DDR_CS0_CONFIG_2		0
#define DDR_SDRAM_CFG_MEM_EN	0x80000000
#define SDRAM_CFG2_D_INIT		0x00000010
#define DDR_CDR2_VREF_TRAIN_EN	0x00000080
#define SDRAM_CFG2_FRC_SR		0x80000000
#define SDRAM_CFG_BI			0x00000001

#ifdef CONFIG_SD_BOOT

#define CONFIG_SYS_FSL_PBL_RCW	board/tqc/tqmls102xa/ls102xa_rcw_sd.cfg

#ifdef CONFIG_RAMBOOT_PBL
#define CONFIG_SYS_FSL_PBL_PBI	board/tqc/tqmls102xa/ls102xa_pbi_sd.cfg
#endif

#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_LDSCRIPT	"arch/$(ARCH)/cpu/u-boot-spl.lds"
#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_ENV_SUPPORT
#define CONFIG_SPL_MPC8XXX_INIT_DDR_SUPPORT
#define CONFIG_SPL_I2C_SUPPORT
#define CONFIG_SPL_WATCHDOG_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR		0xe8
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS		0x400

#define CONFIG_SPL_TEXT_BASE		0x10000000
#define CONFIG_SPL_MAX_SIZE		0x1a000
#define CONFIG_SPL_STACK		0x1001d000
#define CONFIG_SPL_PAD_TO		0x1c000
#define CONFIG_SYS_TEXT_BASE		0x82000000

#define CONFIG_SYS_SPL_MALLOC_START	(CONFIG_SYS_TEXT_BASE + \
		CONFIG_SYS_MONITOR_LEN)
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x100000
#define CONFIG_SPL_BSS_START_ADDR	0x80100000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000
#define CONFIG_SYS_MONITOR_LEN		0x80000
#endif

#ifdef CONFIG_QSPI_BOOT
#define CONFIG_SYS_TEXT_BASE		0x40010000
#endif

#if defined(CONFIG_QSPI_BOOT) || defined(CONFIG_SD_BOOT)
#define CONFIG_SYS_NO_FLASH
#endif

#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE		0x60100000
#endif

#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM			0x80000000
#define PHYS_SDRAM_SIZE			(1u * 1024 * 1024 * 1024)

#define CONFIG_SYS_DDR_SDRAM_BASE      0x80000000UL
#define CONFIG_SYS_SDRAM_BASE          CONFIG_SYS_DDR_SDRAM_BASE

#define CONFIG_SYS_HAS_SERDES

#define CONFIG_FSL_CAAM			/* Enable CAAM */

#if !defined(CONFIG_SD_BOOT) && !defined(CONFIG_NAND_BOOT) && \
	!defined(CONFIG_QSPI_BOOT)
#define CONFIG_U_QE
#endif

/*
 * Serial Port
 */
#ifdef CONFIG_LPUART
#define CONFIG_LPUART_32B_REG
#else
#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_NS16550_SERIAL
#ifndef CONFIG_DM_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#endif
#define CONFIG_SYS_NS16550_CLK		get_serial_clock()
#endif

#define CONFIG_BAUDRATE			115200

/*
 * I2C
 */
#define CONFIG_CMD_I2C
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_MXC_I2C3		/* enable I2C bus 3 */

/* I2C EEPROM (M24C64) */
#define CONFIG_SYS_EEPROM_BUS_NUM			0
#define CONFIG_SYS_I2C_EEPROM_ADDR			0x50
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN			2
#define CONFIG_SYS_I2C_EEPROM_PAGE_WRITE_BITS		5 /* 32 Bytes */
#define CONFIG_SYS_I2C_EEPROM_PAGE_WRITE_DELAY_MS	20
#define CONFIG_CMD_EEPROM

/*
 * MMC
 */
#define CONFIG_MMC
#define CONFIG_CMD_MMC
#define CONFIG_FSL_ESDHC
#define CONFIG_GENERIC_MMC
#define CONFIG_SYS_FSL_ERRATUM_ESDHC111
#define ESDHCI_QUIRK_BROKEN_TIMEOUT_VALUE

/* QSPI */
#if defined(CONFIG_QSPI_BOOT) || defined(CONFIG_SD_BOOT)

#define CONFIG_FSL_QSPI
#define QSPI0_AMBA_BASE			0x40000000
#define FSL_QSPI_FLASH_SIZE		SZ_64M
#define FSL_QSPI_FLASH_NUM		2
#define FSL_QSPI_FLASH_DUALDIE

#if defined(CONFIG_SD_BOOT)
#define FSL_QSPI_QUAD_MODE
#endif

#define CONFIG_CMD_SF
#define CONFIG_SPI_FLASH
/* Banked address mode - TODO implement 4 Byte adressing */
#define CONFIG_SPI_FLASH_BAR
#define CONFIG_SPI_FLASH_STMICRO

#define CONFIG_CMD_TIME

#endif /* QSPI */

/*
 * Video -> TODO
 */
/* #define CONFIG_FSL_DCU_FB */

#ifdef CONFIG_FSL_DCU_FB

#define CONFIG_VIDEO
#define CONFIG_CMD_BMP
#define CONFIG_CFB_CONSOLE
#define CONFIG_VGA_AS_SINGLE_DEVICE
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_BMP_LOGO

#define CONFIG_FSL_DCU_SII9022A
#define CONFIG_SYS_I2C_DVI_BUS_NUM	1
#define CONFIG_SYS_I2C_DVI_ADDR		0x39

#endif /* CONFIG_FSL_DCU_FB */

/*
 * eTSEC
 */
#define CONFIG_TSEC_ENET

#ifdef CONFIG_TSEC_ENET
#define CONFIG_MII
#define CONFIG_MII_DEFAULT_TSEC		1
#define CONFIG_TSEC1			1
#define CONFIG_TSEC1_NAME		"eTSEC1"
#define CONFIG_TSEC2			1
#define CONFIG_TSEC2_NAME		"eTSEC2"
#define CONFIG_TSEC3			1
#define CONFIG_TSEC3_NAME		"eTSEC3"

#define TSEC1_FLAGS			(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC2_FLAGS			(TSEC_GIGABIT | TSEC_REDUCED | TSEC_SGMII)
#define TSEC3_FLAGS			(TSEC_GIGABIT | TSEC_REDUCED)

#define TSEC1_PHY_ADDR			0x0C
#define TSEC2_PHY_ADDR			0x03
#define TSEC3_PHY_ADDR			0x04

#define CONFIG_SYS_TBIPA_VALUE		0x08

#define CONFIG_ETHPRIME			"eTSEC3"

#define CONFIG_PHY_GIGE
#define CONFIG_PHYLIB
#define CONFIG_PHY_TI

#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
#define CONFIG_HAS_ETH2

#endif /* eTSEC */

/* PCIe */
#define CONFIG_PCI		/* Enable PCI/PCIE */
#define CONFIG_PCIE1		/* PCIE controler 1 */
#define CONFIG_PCIE2		/* PCIE controler 2 */
#define CONFIG_PCIE_LAYERSCAPE	/* Use common FSL Layerscape PCIe code */
#define FSL_PCIE_COMPAT "fsl,ls1021a-pcie"

#define CONFIG_SYS_PCI_64BIT

#define CONFIG_SYS_PCIE_CFG0_PHYS_OFF	0x00000000
#define CONFIG_SYS_PCIE_CFG0_SIZE	0x00001000	/* 4k */
#define CONFIG_SYS_PCIE_CFG1_PHYS_OFF	0x00001000
#define CONFIG_SYS_PCIE_CFG1_SIZE	0x00001000	/* 4k */

#define CONFIG_SYS_PCIE_IO_BUS		0x00000000
#define CONFIG_SYS_PCIE_IO_PHYS_OFF	0x00010000
#define CONFIG_SYS_PCIE_IO_SIZE		0x00010000	/* 64k */

#define CONFIG_SYS_PCIE_MEM_BUS		0x08000000
#define CONFIG_SYS_PCIE_MEM_PHYS_OFF	0x04000000
#define CONFIG_SYS_PCIE_MEM_SIZE	0x08000000	/* 128M */

#ifdef CONFIG_PCI
#define CONFIG_NET_MULTI
#define CONFIG_PCI_PNP
#define CONFIG_E1000
#define CONFIG_PCI_SCAN_SHOW
#define CONFIG_CMD_PCI
#endif

#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_MII
#define CONFIG_CMD_MDIO

#define CONFIG_CMDLINE_TAG
#define CONFIG_CMDLINE_EDITING

#if defined(CONFIG_SYS_NO_FLASH)
#undef CONFIG_CMD_IMLS
#endif

#define CONFIG_ARMV7_NONSEC
#define CONFIG_ARMV7_VIRT
#define CONFIG_PEN_ADDR_BIG_ENDIAN
#define CONFIG_LAYERSCAPE_NS_ACCESS
#define CONFIG_SMP_PEN_ADDR		0x01ee0200
#define CONFIG_TIMER_CLK_FREQ		12500000
#define CONFIG_ARMV7_SECURE_BASE	OCRAM_BASE_S_ADDR

#define CONFIG_HWCONFIG
#define HWCONFIG_BUFFER_SIZE		128

#define CONFIG_BOOTDELAY		3

#ifdef CONFIG_LPUART
#define TQMLS201X_CONSOLE_DEV		"ttyLP0"
#else
#define TQMLS201X_CONSOLE_DEV		"ttyS0"
#endif

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser */
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE		\
		(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		16	/* max number of command args */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#define CONFIG_CMD_GREPENV
#define CONFIG_CMD_MEMINFO
#define CONFIG_CMD_MEMTEST
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		0x9fffffff

/* set to RAMBASE + 32 MiB -> advice from Documentation/arm/booting */
#define CONFIG_SYS_LOAD_ADDR		0x82000000

#define CONFIG_LS102XA_STREAM_ID

/*
 * Stack sizes
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE		(30 * 1024)

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_MONITOR_BASE CONFIG_SPL_TEXT_BASE
#else
#define CONFIG_SYS_MONITOR_BASE CONFIG_SYS_TEXT_BASE    /* start of monitor */
#endif

#define CONFIG_SYS_QE_FW_ADDR		0x67f40000

/*
 * Environment
 */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_ENV_SIZE			(SZ_32K)
#define CONFIG_LOADADDR			CONFIG_SYS_LOAD_ADDR

#if defined(CONFIG_SD_BOOT)

#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_ENV_OFFSET		SZ_1M

#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_SIZE_REDUND		CONFIG_ENV_SIZE
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)

#elif defined(CONFIG_QSPI_BOOT)

#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_OFFSET		0x100000
#define CONFIG_ENV_SECT_SIZE		0x10000

#define TQMLS102X_UBOOT_SPI_OFFSET	SZ_64K

#else
#error
#endif

/* Partitioning */
#define TQMLS102X_UBOOT_OFFSET		SZ_4K
#define TQMLS102X_UBOOT_SECTOR_START	0x8
#define TQMLS102X_UBOOT_SECTOR_COUNT	0x7f8
#define TQMLS102X_UBOOT_SPI_OFFSET	SZ_64K

#define TQMLS102X_FDT_OFFSET		(2 * SZ_1M)
#define TQMLS102X_FDT_SECTOR_START	0x1000
#define TQMLS102X_FDT_SECTOR_COUNT	0x800
#define CONFIG_DEFAULT_FDT_FILE		"ls1021a-tqmls1021a-mbls102x.dtb"

#define TQMLS102X_KERNEL_SECTOR_START	0x2000
#define TQMLS102X_KERNEL_SECTOR_COUNT	0x4000

/* 128 MiB offset as in ARM related docu for linux suggested */
#define TQMLS102X_FDT_ADDRESS		0x88000000

#define TQMLS102X_EXTRA_BOOTDEV_ENV_SETTINGS                                   \
	"uboot_start="__stringify(TQMLS102X_UBOOT_SECTOR_START)"\0"            \
	"uboot_size="__stringify(TQMLS102X_UBOOT_SECTOR_COUNT)"\0"             \
	"fdt_start="__stringify(TQMLS102X_FDT_SECTOR_START)"\0"                \
	"fdt_size="__stringify(TQMLS102X_FDT_SECTOR_COUNT)"\0"                 \
	"kernel_start="__stringify(TQMLS102X_KERNEL_SECTOR_START)"\0"          \
	"kernel_size="__stringify(TQMLS102X_KERNEL_SECTOR_COUNT)"\0"           \
	"mmcdev="__stringify(CONFIG_SYS_MMC_ENV_DEV)"\0"                       \
	"loadimage=mmc dev ${mmcdev}; "                                        \
		"mmc read ${loadaddr} ${kernel_start} ${kernel_size};\0"       \
	"loadfdt=mmc dev ${mmcdev}; "                                          \
		"mmc read ${fdt_addr} ${fdt_start} ${fdt_size};\0"             \
	"update_uboot=if tftp ${uboot}; then "                                 \
		"if itest ${filesize} > 0; then "                              \
			"mmc dev ${mmcdev}; mmc rescan; "                      \
			"setexpr blkc ${filesize} / 0x200; "                   \
			"setexpr blkc ${blkc} + 1; "                           \
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
				"setexpr blkc ${filesize} / 0x200; "           \
				"setexpr blkc ${blkc} + 1; "                   \
				"if itest ${blkc} <= ${kernel_size}; then "    \
					"mmc write ${loadaddr} "               \
						"${kernel_start} ${blkc}; "    \
				"fi; "                                         \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv blkc \0"                              \
	"update_fdt=if tftp ${fdt_file}; then "                                \
		"if itest ${filesize} > 0; then "                              \
			"mmc dev ${mmcdev}; mmc rescan; "                      \
			"setexpr blkc ${filesize} / 0x200; "                   \
			"setexpr blkc ${blkc} + 1; "                           \
			"if itest ${blkc} <= ${fdt_size}; then "               \
				"mmc write ${loadaddr} ${fdt_start} ${blkc}; " \
			"fi; "                                                 \
		"fi; fi; "                                                     \
		"setenv filesize; setenv blkc \0"                              \


#define CONFIG_BOOTCOMMAND \
	"run mmcboot; run netboot; run panicboot"

#define CONFIG_EXTRA_ENV_SETTINGS                                              \
	"board=tqmls102x\0"                                                    \
	"uimage=uImage\0"                                                      \
	"zimage=zImage\0"                                                      \
	"boot_type=bootz\0"                                                    \
	"kernel_name=if test \"${boot_type}\" != bootz; then "                 \
		"setenv kernel ${uimage}; "                                    \
		"else setenv kernel ${zimage}; fi\0"                           \
	"uboot=u-boot-with-spl-pbl-mmcsd-2016.05-rc1.bin\0"                    \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0"                               \
	"fdt_addr="__stringify(TQMLS102X_FDT_ADDRESS)"\0"                      \
	"console=" TQMLS201X_CONSOLE_DEV "\0"                                  \
	"fdt_high=0xffffffff\0"                                                \
	"initrd_high=0xffffffff\0"                                             \
	"addtty=setenv bootargs ${bootargs} console=${console},${baudrate}\0"  \
	"mmcpart=2\0"                                                          \
	"mmcblkdev=0\0"                                                        \
	"mmcargs=run addmmc addtty\0"                                          \
	"addmmc=setenv bootargs ${bootargs} "                                  \
		"root=/dev/mmcblk${mmcblkdev}p${mmcpart} rw rootwait\0"        \
	"mmcboot=echo Booting from mmc ...; "                                  \
		"setenv bootargs; "                                            \
		"run mmcargs; "                                                \
		"run loadimage; "                                              \
		"if run loadfdt; then "                                        \
			"echo boot device tree kernel ...; "                   \
			"${boot_type} ${loadaddr} - ${fdt_addr}; "             \
		"else "                                                        \
			"${boot_type}; "                                       \
		"fi;\0"                                                        \
		"setenv bootargs \0"                                           \
	"netdev=eth0\0"                                                        \
	"rootpath=/srv/nfs/tqmls1021a\0"                                       \
	"ipmode=static\0"                                                      \
	"netargs=run addnfs addip addtty addfb\0"                              \
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
		"if ${getcmd} ${kernel}; then "                                \
			"if ${getcmd} ${fdt_addr} ${fdt_file}; then "          \
				"${boot_type} ${loadaddr} - ${fdt_addr}; "     \
			"fi; "                                                 \
		"fi; "                                                         \
		"echo ... failed\0"                                            \
	"panicboot=echo No boot device !!! reset\0"                            \
	"uboot-qspi=u-boot.bin-u-boot-qspi\0"                                  \
	"update_uboot-qspi=if tftp ${uboot-qspi}; then "                       \
		"if itest ${filesize} > 0; then "                              \
			"setexpr erasesz ${filesize} / 0x10000; "              \
			"setexpr erasesz ${erasesz} + 1; "                     \
			"setexpr erasesz ${erasesz} * 0x10000; "               \
			"sf probe; sf erase "__stringify(TQMLS102X_UBOOT_SPI_OFFSET)" ${erasesz}; " \
			"sf write ${loadaddr} "__stringify(TQMLS102X_UBOOT_SPI_OFFSET)" ${filesize}; " \
		"fi; fi; "                                                     \
		"setenv filesize; setenv erasesz\0"                            \
	"rcw-qspi=ls102xa-rcw-tqmls-qspi-0100.bin.bswap\0"                     \
	"update_rcw=if tftp ${rcw-qspi}; then "                                \
		"if itest ${filesize} > 0; then "                              \
			"setexpr erasesz ${filesize} / 0x10000; "              \
			"setexpr erasesz ${erasesz} + 1; "                     \
			"setexpr erasesz ${erasesz} * 0x10000; "               \
			"sf probe; sf erase 0 ${erasesz}; "                    \
			"sf write ${loadaddr} 0 ${filesize}; "                 \
		"fi; fi; "                                                     \
		"setenv filesize; setenv erasesz\0"                            \
	TQMLS102X_EXTRA_BOOTDEV_ENV_SETTINGS                                   \

#define CONFIG_CMD_BOOTZ

#define CONFIG_MISC_INIT_R

/* Hash command with SHA acceleration supported in hardware */
#ifdef CONFIG_FSL_CAAM
#define CONFIG_CMD_HASH
#define CONFIG_SHA_HW_ACCEL
#endif

#ifdef CONFIG_SECURE_BOOT
#define CONFIG_CMD_BLOB
#endif

#endif /* __CONFIG_H */
