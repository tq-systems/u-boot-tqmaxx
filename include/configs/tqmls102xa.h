/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Copyright 2015 TQ Systems GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

#define CONFIG_ARMV7_PSCI_1_0

#define CONFIG_ARMV7_SECURE_BASE	OCRAM_BASE_S_ADDR

#define CONFIG_SYS_FSL_CLK

#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_DEEP_SLEEP

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 16 * 1024 * 1024)

#define CONFIG_SYS_INIT_RAM_ADDR	OCRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	OCRAM_SIZE

#ifdef CONFIG_SPL_BUILD
#undef CONFIG_SPL_HUSH_PARSER
#undef CONFIG_HUSH_PARSER
#undef CONFIG_CMDLINE
#undef CONFIG_DM_MMC
#endif

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
#define CONFIG_USB_MAX_CONTROLLER_COUNT		1
#define CONFIG_SYS_USB_XHCI_MAX_ROOT_PORTS	2
#endif

/*
 * Generic Timer Definitions
 */
#define COUNTER_FREQUENCY		12500000

#define CONFIG_SYS_CLK_FREQ		100000000
#define CONFIG_DDR_CLK_FREQ		100000000

#if defined(CONFIG_DDR_ECC)
#define DDR_SDRAM_CFG			0x670c0004
#else
#define DDR_SDRAM_CFG			0x470c0000
#endif
#define DDR_CS0_BNDS			0x008000bf
#define DDR_CS0_CONFIG			0x80014302
#define DDR_CS1_BNDS			0x00c000ff
#define DDR_CS1_CONFIG			0x80014302
#define DDR_TIMING_CFG_0		0x0066000c
#define DDR_TIMING_CFG_1		0xbcb40c66
#define DDR_TIMING_CFG_2		0x0040c120
#define DDR_TIMING_CFG_3		0x010C1000
#define DDR_TIMING_CFG_4		0x00000001
#define DDR_TIMING_CFG_5		0x00002400
#define DDR_TIMING_CFG_7		0x13300000
#define DDR_SDRAM_CFG_2			0x00401010
#define DDR_SDRAM_MODE			0x00041c70
#define DDR_SDRAM_MODE_2		0x00980000
#define DDR_SDRAM_INTERVAL		0x0c300618
#define DDR_DDR_WRLVL_CNTL		0x8645c606
#define DDR_DDR_WRLVL_CNTL_2		0x05060607
#define DDR_DDR_WRLVL_CNTL_3		0x05050505
#define DDR_DDR_CDR1			0x80080000
#define DDR_DDR_CDR2			0x00000001
#define DDR_SDRAM_CLK_CNTL		0x02000000
#define DDR_DDR_ZQ_CNTL			0x89080600
#define DDR_CS0_CONFIG_2		0
#define DDR_SDRAM_CFG_MEM_EN		0x80000000
#define SDRAM_CFG2_D_INIT		0x00000010
#define DDR_CDR2_VREF_TRAIN_EN		0x00000080
#define SDRAM_CFG2_FRC_SR		0x80000000
#define SDRAM_CFG_BI			0x00000001

#ifdef CONFIG_SD_BOOT
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_LDSCRIPT	"arch/$(ARCH)/cpu/u-boot-spl.lds"
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
#define CONFIG_SYS_MONITOR_LEN		0x100000
#endif

#ifdef CONFIG_QSPI_BOOT
#define CONFIG_SYS_TEXT_BASE		0x40010000
#define CONFIG_SPL_PAD_TO		0x10000
#endif

#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE		0x60100000
#endif

#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM			0x80000000
#ifdef CONFIG_SYS_1G_CS0
  #define PHYS_SDRAM_SIZE		(1u * 1024 * 1024 * 1024)
#elif CONFIG_SYS_2G_CS0CS1
  #define PHYS_SDRAM_SIZE		(2u * 1024 * 1024 * 1024)
#endif

#define CONFIG_SYS_DDR_SDRAM_BASE      0x80000000UL
#define CONFIG_SYS_SDRAM_BASE          CONFIG_SYS_DDR_SDRAM_BASE

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
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_MXC_I2C3		/* enable I2C bus 3 */

/* I2C EEPROM (M24C64) */
#define CONFIG_SYS_EEPROM_BUS_NUM		0
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x54
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		2
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	5 /* 32 Bytes */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	20

/*
 * MMC
 */
#define CONFIG_FSL_ESDHC
#define CONFIG_SYS_FSL_ERRATUM_ESDHC111
#define ESDHCI_QUIRK_BROKEN_TIMEOUT_VALUE

/* QSPI */
#if defined(CONFIG_QSPI_BOOT)
#if defined(CONFIG_RAMBOOT_PBL) || defined(CONFIG_RAMBOOT_PBL_BIN)
  #define CONFIG_SYS_FSL_PBL_PBI	board/tqc/tqmls102xa/ls102xa_pbi_qspi.cfg
  #if defined(CONFIG_SYS_CPU_1200)
    #define CONFIG_SYS_FSL_PBL_RCW	board/tqc/tqmls102xa/ls102xa_rcw_qspi_1200.cfg
  #else
    #define CONFIG_SYS_FSL_PBL_RCW	board/tqc/tqmls102xa/ls102xa_rcw_qspi_1000.cfg
  #endif
#endif
#elif defined(CONFIG_SD_BOOT)
#define CONFIG_SYS_FSL_PBL_PBI	board/tqc/tqmls102xa/ls102xa_pbi_sd.cfg
#define CONFIG_SYS_FSL_PBL_RCW	board/tqc/tqmls102xa/ls102xa_rcw_sd.cfg
#endif

#define QSPI0_AMBA_BASE			0x40000000
#define FSL_QSPI_FLASH_SIZE		SZ_64M
#define FSL_QSPI_FLASH_NUM		2
#define FSL_QSPI_FLASH_DUALDIE
#define CONFIG_SYS_FSL_QSPI_AHB
#define FSL_QSPI_QUAD_MODE

/* filesystem on flash */
#define CONFIG_SYS_MAX_FLASH_BANKS_DETECT	2
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_RBTREE
#define CONFIG_MTD_UBI_WL_THRESHOLD	4096
#define CONFIG_LZO
#define MTDIDS_DEFAULT \
	"nor0=nor0\0"

#define MTDPARTS_DEFAULT \
	"mtdparts=nor0:"                                                       \
		"896k@0k(U-Boot-PBL),"                                         \
		 "64k@896k(ENV),"                                              \
		 "64k@960k(DTB),"                                              \
		  "7M@1M(Linux),"                                              \
		 "56M@8M(RootFS)\0"                                            \

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

#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
#define CONFIG_HAS_ETH2

#endif /* eTSEC */

/* PCIe */
#define CONFIG_PCIE1		/* PCIE controler 1 */
#define CONFIG_PCIE2		/* PCIE controler 2 */

#ifdef CONFIG_PCI
#define CONFIG_PCI_SCAN_SHOW
#define CONFIG_CMD_PCI
#endif

#define CONFIG_CMDLINE_TAG
#define CONFIG_CMDLINE_EDITING

#if defined(CONFIG_SYS_NO_FLASH)
#undef CONFIG_CMD_IMLS
#endif

#define CONFIG_PEN_ADDR_BIG_ENDIAN
#define CONFIG_LAYERSCAPE_NS_ACCESS
#define CONFIG_SMP_PEN_ADDR		0x01ee0200

#define CONFIG_HWCONFIG
#define HWCONFIG_BUFFER_SIZE		256

#ifdef CONFIG_LPUART
#define TQMLS102X_CONSOLE_DEV		"ttyLP0"
#else
#define TQMLS102X_CONSOLE_DEV		"ttyS0"
#endif

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE		\
		(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		16	/* max number of command args */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		0x9fffffff

/* set to RAMBASE + 32 MiB -> advice from Documentation/arm/booting */
#define CONFIG_SYS_LOAD_ADDR		0x82000000

#define CONFIG_LS102XA_STREAM_ID

/*
 * Stack sizes
 * The stack sizes are set up in start.S using the settings below
 */
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

#elif defined(CONFIG_QSPI_BOOT)

#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_OFFSET		0xE0000
#define CONFIG_ENV_SECT_SIZE		0x10000
#define CONFIG_SYS_MMC_ENV_DEV		0	/* for mmcdev */

#else
#error
#endif

/* Partitioning */
#define TQMLS102X_UBOOT_OFFSET		SZ_4K
#define TQMLS102X_UBOOT_SECTOR_START	0x8
#define TQMLS102X_UBOOT_SECTOR_COUNT	0x7f8

#if defined(CONFIG_DDR_ECC)
#define TQMLS102X_UBOOT_MMCSD_IMAGE_FILE	"u-boot-with-spl-pbl-ecc-mmcsd-2017.07.bin"
#define TQMLS102X_UBOOT_QSPI_IMAGE_FILE		"u-boot-pbl.bin.ecc.bswap"
#else
#define TQMLS102X_UBOOT_MMCSD_IMAGE_FILE	"u-boot-with-spl-pbl-mmcsd-2017.07.bin"
#define TQMLS102X_UBOOT_QSPI_IMAGE_FILE		"u-boot-pbl.bin.bswap"
#endif

/* 128 MiB offset as in ARM related docu for linux suggested */
#define TQMLS102X_FDT_ADDRESS		0x88000000

#if defined(CONFIG_TQMLS102XA_MBLS102XA)
#define BASEBOARD_EXTRA_ENV_SETTINGS	"addplatform=cpld_mux\0"
#else
#define BASEBOARD_EXTRA_ENV_SETTINGS	"addplatform=\0"
#endif

#define TQMLS102X_EXTRA_BOOTDEV_ENV_SETTINGS                                   \
	"uboot_start="__stringify(TQMLS102X_UBOOT_SECTOR_START)"\0"            \
	"uboot_size="__stringify(TQMLS102X_UBOOT_SECTOR_COUNT)"\0"             \
	"mmcdev="__stringify(CONFIG_SYS_MMC_ENV_DEV)"\0"                       \
	"mtdparts=" MTDPARTS_DEFAULT                                           \
	"firmwarepart=1\0"                                                     \
	"loadimage=run kernel_name; "                                          \
		"load mmc ${mmcdev}:${firmwarepart} ${loadaddr} ${kernel} \0"  \
	"loadfdt="                                                       \
		"load mmc ${mmcdev}:${firmwarepart} ${fdt_addr} ${fdt_file} \0"\
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
	BASEBOARD_EXTRA_ENV_SETTINGS                                           \

#define CONFIG_BOOTCOMMAND \
	"run mmcboot; run spiboot; run netboot; run panicboot"

#define CONFIG_EXTRA_ENV_SETTINGS                                              \
	"addmisc=setenv bootargs ${bootargs} hdmi\0"                           \
	"board=tqmls102x\0"                                                    \
	"uimage=uImage\0"                                                      \
	"zimage=linuximage\0"                                                  \
	"boot_type=bootz\0"                                                    \
	"kernel_name=if test \"${boot_type}\" != bootz; then "                 \
		"setenv kernel ${uimage}; "                                    \
		"else setenv kernel ${zimage}; fi\0"                           \
	"uboot=" TQMLS102X_UBOOT_MMCSD_IMAGE_FILE "\0"                            \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0"                               \
	"fdt_addr="__stringify(TQMLS102X_FDT_ADDRESS)"\0"                      \
	"console=" TQMLS102X_CONSOLE_DEV "\0"                                  \
	"fdt_high=0xffffffff\0"                                                \
	"initrd_high=0xffffffff\0"                                             \
	"addtty=setenv bootargs ${bootargs} console=tty0 "                     \
		"console=${console},${baudrate}\0"                             \
	"mmcpart=2\0"                                                          \
	"mmcblkdev=0\0"                                                        \
	"mmcargs=run addmmc addtty addmisc addplatform\0"                      \
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
	"netargs=run addnfs addip addtty addmisc\0"                            \
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
	"addspi=setenv bootargs ${bootargs} root=ubi0:root rw "                \
		"rootfstype=ubifs ubi.mtd=4\0"                                 \
	"spiargs=run addspi addtty addmisc\0"                                  \
	"loadspiimage=sf probe 0; sf read ${loadaddr} Linux\0"                 \
	"loadspifdt=sf probe 0; sf read ${fdt_addr} DTB\0"                     \
	"spiboot=echo Booting from SPI NOR flash...; setenv bootargs; "        \
		"run spiargs; run loadspiimage; "                              \
		"if run loadspifdt; then "                                     \
			"${boot_type} ${loadaddr} - ${fdt_addr}; "             \
		"else "                                                        \
			"${boot_type}; "                                       \
		"fi;\0"                                                        \
	"uboot-qspi=" TQMLS102X_UBOOT_QSPI_IMAGE_FILE "\0"                        \
	"rootfs-qspi=root.ubi\0"                                               \
	"update_rcw=run update_uboot-qspi\0"                                   \
	"update_uboot-qspi=if tftp ${uboot-qspi}; then "                       \
		"if itest ${filesize} > 0; then "                              \
			"sf probe 0; "                                         \
			"sf update ${loadaddr} U-Boot-PBL ${filesize}; "       \
		"fi; fi; "                                                     \
		"setenv filesize;\0"                                           \
	"update_kernel-qspi=run kernel_name; if tftp ${kernel}; then "         \
		"if itest ${filesize} > 0; then "                              \
			"sf probe 0; sf update ${loadaddr} Linux ${filesize};" \
		"fi; fi; "                                                     \
		"setenv filesize;\0"                                           \
	"update_fdt-qspi=if tftp ${fdt_file}; then "                           \
		"if itest ${filesize} > 0; then "                              \
			"sf probe 0; sf update ${loadaddr} DTB ${filesize}; "  \
		"fi; fi; "                                                     \
		"setenv filesize;\0"                                           \
	"init_rootfs-qspi=if tftp ${rootfs-qspi}; then "                       \
		"if itest ${filesize} > 0; then "                              \
			"sf probe 0; sf erase RootFS 3800000; "                \
			"sf write ${loadaddr} RootFS ${filesize}; "            \
		"fi; fi; "                                                     \
		"setenv filesize;\0"                                           \
	TQMLS102X_EXTRA_BOOTDEV_ENV_SETTINGS                                   \

#define CONFIG_MISC_INIT_R

#ifdef CONFIG_SECURE_BOOT
#define CONFIG_CMD_BLOB
#endif

#define CONFIG_SYS_BOOTM_LEN (64 << 20)

#endif /* __CONFIG_H */
