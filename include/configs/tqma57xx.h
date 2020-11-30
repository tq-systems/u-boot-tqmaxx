/*
 * (C) Copyright 2014
 * Texas Instruments Incorporated.
 * Felipe Balbi <balbi@ti.com>
 *
 * Copyright (C) 2017 TQ-Systems GmbH (ported AM57xx IDK to TQMa57xx)
 * Author: Stefan Lange <s.lange@gateware.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_TQMA57XX_H
#define __CONFIG_TQMA57XX_H

#include <environment/ti/dfu.h>
#include <linux/sizes.h>

#ifdef CONFIG_SPL_BUILD
#define CONFIG_IODELAY_RECALIBRATION
#endif

/* SDRAM */
#define CONFIG_SYS_DDR_DATA_INIT	0xdeadbeef
#define CONFIG_SYS_MEMTEST_START	0x81000000
#define CONFIG_SYS_MEMTEST_END		0x810fffff

#define CONFIG_SYS_OMAP_ABE_SYSCK

/* console */
#define CONSOLEDEV			"ttyO2"
#define CONFIG_SYS_NS16550_COM1                UART1_BASE      /* UART0 */
#define CONFIG_SYS_NS16550_COM2                UART2_BASE      /* UART2 */
#define CONFIG_SYS_NS16550_COM3                UART3_BASE      /* UART3 */

/* Use General purpose timer 1 */
#define CONFIG_SYS_TIMERBASE            GPT2_BASE

/*
 * For the DDR timing information we can either dynamically determine
 * the timings to use or use pre-determined timings (based on using the
 * dynamic method.  Default to the static timing information.
 */
#define CONFIG_SYS_EMIF_PRECALCULATED_TIMING_REGS
#ifndef CONFIG_SYS_EMIF_PRECALCULATED_TIMING_REGS
#define CONFIG_SYS_AUTOMATIC_SDRAM_DETECTION
#define CONFIG_SYS_DEFAULT_LPDDR2_TIMINGS
#endif

#define CONFIG_PALMAS_POWER

#include <asm/arch/cpu.h>
#include <asm/arch/omap.h>

#include <configs/ti_armv7_omap.h>

/*
 * Hardware drivers
 */
#define CONFIG_SYS_NS16550_CLK          48000000
#if !defined(CONFIG_DM_SERIAL)
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE     (-4)
#endif

/*
 * SPL related defines.  The Public RAM memory map the ROM defines the
 * area between 0x40300000 and 0x4031E000 as a download area for OMAP5.
 * On DRA7xx/AM57XX the download area is between 0x40300000 and 0x4037E000.
 * We set CONFIG_SPL_DISPLAY_PRINT to have omap_rev_string() called and
 * print some information.
 */
#ifdef CONFIG_TI_SECURE_DEVICE
/*
 * For memory booting on HS parts, the first 4KB of the internal RAM is
 * reserved for secure world use and the flash loader image is
 * preceded by a secure certificate. The SPL will therefore run in internal
 * RAM from address 0x40301350 (0x40300000+0x1000(reserved)+0x350(cert)).
 */
#define TI_OMAP5_SECURE_BOOT_RESV_SRAM_SZ       0x1000
#define CONFIG_SPL_TEXT_BASE    0x40301350
/* If no specific start address is specified then the secure EMIF
 * region will be placed at the end of the DDR space. In order to prevent
 * the main u-boot relocation from clobbering that memory and causing a
 * firewall violation, we tell u-boot that memory is protected RAM (PRAM)
 */
#if (CONFIG_TI_SECURE_EMIF_REGION_START == 0)
#define CONFIG_PRAM (CONFIG_TI_SECURE_EMIF_TOTAL_REGION_SIZE) >> 10
#endif
#else
/*
 * For all booting on GP parts, the flash loader image is
 * downloaded into internal RAM at address 0x40300000.
 */
#define CONFIG_SPL_TEXT_BASE    0x40300000
#endif

#define CONFIG_SYS_SPL_ARGS_ADDR        (CONFIG_SYS_SDRAM_BASE + \
					 (128 << 20))

#ifdef CONFIG_SPL_BUILD
#undef CONFIG_TIMER
#endif

/* i2c eeprom */
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x54	/* Main EEPROM */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2
/* variant and revision detection
 * module-specific definitions
 * memsize: 1G or 2G known
 * memtype: only known type used, ecc optional for TQMa574x
 * features: '0' defined as 'feature present'
 */
#define TQC_VARD_BUS			0	/* System EEPROM bus */
#define TQC_VARD_ADDR			0x57
#define TQC_VARD_MEMSIZE_1G		0
#define TQC_VARD_MEMSIZE_2G		1
#define TQC_VARD_MEMSIZE_EMPTY		0xff
#define TQC_VARD_MEMTYPE_ECC_MASK	0x80	/* Bit for ECC-enabled memory */
#define TQC_VARD_MEMTYPE_RAM1		1
#define TQC_VARD_MEMTYPE_EMPTY		0xff
#define TQC_VARD_FEATURES1_EMMC		~BIT(31)
#define TQC_VARD_FEATURES1_EEPROM	~BIT(30)	/* i2c1 0x54 */
#define TQC_VARD_FEATURES1_QSPI		~BIT(29)
#define TQC_VARD_FEATURES2_RTC		~BIT(31)

/* PMIC I2C bus number */
#define CONFIG_SYS_SPD_BUS_NUM		0

/* PCA9555 GPIO expander support */
#define CONFIG_PCA953X

/* SD/eMMC */
#define CONFIG_SUPPORT_EMMC_BOOT
#define CONFIG_HSMMC2_8BIT

/* CPSW Ethernet */
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_NET_RETRY_COUNT		10
#define CONFIG_DRIVER_TI_CPSW		/* Driver for IP block */
#define CONFIG_MII			/* Required in net/eth.c */
#define PHY_ANEG_TIMEOUT	8000	/* PHY needs longer aneg time at 1G */
#define FDT_SEQ_MACADDR_FROM_ENV

/* USB xHCI HOST */
#define CONFIG_USB_XHCI_OMAP
#define CONFIG_OMAP_USB_PHY
#define CONFIG_OMAP_USB3PHY1_HOST

/* SATA */
#define CONFIG_LIBATA
#define CONFIG_SCSI_AHCI
#define CONFIG_SCSI_AHCI_PLAT
#define CONFIG_SYS_SCSI_MAX_SCSI_ID	1
#define CONFIG_SYS_SCSI_MAX_LUN		1
#define CONFIG_SYS_SCSI_MAX_DEVICE	(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
						CONFIG_SYS_SCSI_MAX_LUN)

/* SPI */
#define TQMA57XX_SPI_FLASH_SECTOR_SIZE		SZ_64K
#define CONFIG_SYS_MAX_FLASH_BANKS	1

/*
 * QSPI flash map
 *
 * Default to using SPI for environment, etc.
 * 0x000000 - 0x03FFFF : QSPI.SPL (256KiB)
 * 0x040000 - 0x13FFFF : QSPI.u-boot (1MiB)
 * 0x140000 - 0x14FFFF : QSPI.u-boot-env (64KiB)
 * 0x150000 - 0x15FFFF : QSPI.u-boot-env.backup1 (64KiB)
 * 0x160000 - 0x1DFFFF : QSPI.dtb (512KiB)
 * 0x1E0000 - 0x9DFFFF : QSPI.kernel (8MiB)
 * 0x9E0000 - 0x2000000 : USERLAND
 *
 * ENV_OFFSET, ENV_SIZE, ENV_OFFSET_REDUND identical to MMC
 */
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x40000
#define TQMA57XX_SPI_ENV_OFFS		0x140000
#define CONFIG_SYS_SPI_KERNEL_OFFS	0x1E0000
#define CONFIG_SYS_SPI_ARGS_OFFS	0x140000
#define CONFIG_SYS_SPI_ARGS_SIZE	0x80000

/* SPI ENV */
#define CONFIG_ENV_SPI_BUS		(CONFIG_SF_DEFAULT_BUS)
#define CONFIG_ENV_SPI_CS		(CONFIG_SF_DEFAULT_CS)
#define CONFIG_ENV_SPI_MAX_HZ		(CONFIG_SF_DEFAULT_SPEED)
#define CONFIG_ENV_SPI_MODE		(CONFIG_SF_DEFAULT_MODE)

#define CONFIG_ENV_OFFSET		(TQMA57XX_SPI_ENV_OFFS)
#define CONFIG_ENV_SIZE			SZ_64K
#define CONFIG_ENV_SECT_SIZE		TQMA57XX_SPI_FLASH_SECTOR_SIZE
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_OFFSET_REDUND        (CONFIG_ENV_OFFSET + \
					 CONFIG_ENV_SECT_SIZE)
#define CONFIG_SYS_MMC_ENV_DEV		1

/* Default environment */
#include <environment/ti/boot.h>
#include <environment/ti/mmc.h>

#ifdef CONFIG_CMD_NET
#define NETARGS_TQMA57XX \
	"static_ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}" \
		"::off\0" \
	"nfsopts=nolock\0" \
	"rootpath=/srv/nfs/rootfs\0" \
	"netloadimage=nfs ${loadaddr} $serverip:$rootpath/$bootfile\0" \
	"netloadfdt=nfs ${fdtaddr} $serverip:$rootpath/$fdtfile\0" \
	"netargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"root=/dev/nfs " \
		"nfsroot=${serverip}:${rootpath},${nfsopts} rw " \
		"ip=dhcp\0" \
	"netboot=echo Booting from network ...; " \
		"setenv autoload no; " \
		"dhcp; " \
		"run netloadimage; " \
		"run netloadfdt; " \
		"run netargs; " \
		"bootz ${loadaddr} - ${fdtaddr}\0"
#else
#define NETARGS_TQMA57XX ""
#endif

#define COMMON_BOOT_ARGS \
	"console=" CONSOLEDEV ",115200n8\0" \
	"bootpart=${mmcdev}:1\0" \
	"usbtty=cdc_acm\0" \
	"vram=16M\0" \
	AVB_VERIFY_CMD \
	"partitions=" PARTS_DEFAULT "\0" \
	"optargs=\0" \
	"dofastboot=0\0" \
	"emmc_linux_boot=" \
		"echo Trying to boot Linux from eMMC ...; " \
		"setenv mmcdev 1; " \
		"setenv bootpart 1:2; " \
		"setenv mmcroot /dev/mmcblk1p2 rw; " \
		"run mmcboot;\0" \

#define CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
#define CONFIG_EXTRA_ENV_SETTINGS \
	DEFAULT_LINUX_BOOT_ENV \
	COMMON_BOOT_ARGS \
	DEFAULT_FIT_TI_ARGS \
	NETARGS_TQMA57XX \
	"bootfile=linuximage\0" \
	"devtype=mmc \0" \
	"u-boot=u-boot.img\0" \
	"uboot_size=0x800\0" \
	"MLO=MLO\0" \
	"mlo_size=0x100\0" \
	"update_uboot=setenv bootpart ${mmcdev}:1; " \
		"if tftp ${MLO}; then " \
		"echo updating MLO on mmc${mmcdev}...; " \
		"mmc dev ${mmcdev}; mmc rescan; " \
		"fatwrite ${devtype} ${bootpart} ${loadaddr} ${MLO} ${filesize}; " \
		"fi; if tftp ${u-boot}; then " \
		"echo updating u-boot on mmc${mmcdev}...; " \
		"fatwrite ${devtype} ${bootpart} ${loadaddr} ${u-boot} ${filesize}; " \
		"fi; setenv filesize; \0" \
	"update_kernel=setenv bootpart ${mmcdev}:1; " \
		"if tftp ${bootfile}; then " \
		"echo updating ${bootfile} on mmc${bootpart}...; " \
		"mmc dev ${mmcdev}; mmc rescan; " \
		"fatwrite ${devtype} ${bootpart} ${loadaddr} ${bootfile} ${filesize}; " \
		"fi; setenv filesize; \0" \
	"update_fdt=setenv bootpart ${mmcdev}:1; " \
		"if tftp ${fdtfile}; then " \
		"echo updating {fdtfile} on mmc${bootpart}...; " \
		"mmc dev ${mmcdev}; mmc rescan; " \
		"fatwrite ${devtype} ${bootpart} ${loadaddr} ${fdtfile} ${filesize}; " \
		"fi; setenv filesize; \0" \
	"upd_spi=if tftp u-boot/${u-boot}; " \
		"then echo updating u-boot on spi flash...; " \
		"sf probe 0; sf erase 0x00000 0x100000; " \
		"sf write ${loadaddr} 0x40000 ${filesize}; " \
		"tftp u-boot/${MLO}; " \
		"sf write ${loadaddr} 0x00000 ${filesize}; " \
		"else echo u-boot file not found!; fi \0" \
	"bootcmd=if test ${dofastboot} -eq 1; then echo " \
		"Boot fastboot requested, resetting dofastboot ...; " \
		"setenv dofastboot 0; saveenv; " \
		"echo Booting into fastboot ...; fastboot 1; fi; " \
		"if test ${boot_fit} -eq 1; then run update_to_fit;fi; " \
		"run envboot; run mmcboot; \0 " \
	"boot_fit=0\0" \
	"mmcrootfstype=ext4 rootwait\0" \
	"finduuid=setenv bootpart ${mmcdev}:1; part uuid mmc ${bootpart} uuid\0" \
	"args_mmc=setenv bootargs console=${console} " \
		"${optargs} " \
		"root=/dev/mmcblk${mmcblkdev}p2 rw " \
		"rootfstype=${mmcrootfstype}\0" \
	"loadbootscript=setenv bootpart ${mmcdev}:1; load ${devtype} ${bootpart} ${loadaddr} boot.scr\0" \
	"bootscript=echo Running bootscript from mmc${mmcdev} ...; " \
		"source ${loadaddr}\0" \
	"bootenvfile=uEnv.txt\0" \
	"importbootenv=echo Importing environment from mmc${mmcdev} ...; " \
		"env import -t ${loadaddr} ${filesize}\0" \
	"loadbootenv=setenv bootpart ${mmcdev}:1; load ${devtype} ${bootpart} ${loadaddr} ${bootenvfile}\0" \
	"loadimage=setenv bootpart ${mmcdev}:1; load ${devtype} ${bootpart} ${loadaddr} ${bootfile}\0" \
	"loadfdt=setenv bootpart ${mmcdev}:1; load ${devtype} ${bootpart} ${fdtaddr} ${fdtfile}\0" \
	"envboot=mmc dev ${mmcdev}; " \
		"if mmc rescan; then " \
			"echo SD/MMC found on device ${mmcdev};" \
			"if run loadbootscript; then " \
				"run bootscript;" \
			"else " \
				"if run loadbootenv; then " \
					"echo Loaded env from ${bootenvfile};" \
					"run importbootenv;" \
				"fi;" \
				"if test -n $uenvcmd; then " \
					"echo Running uenvcmd ...;" \
					"run uenvcmd;" \
				"fi;" \
			"fi;" \
		"fi;\0" \
	"mmcloados=run args_mmc; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if run loadfdt; then " \
				"bootz ${loadaddr} - ${fdtaddr}; " \
			"else " \
				"if test ${boot_fdt} = try; then " \
					"bootz; " \
				"else " \
					"echo WARN: Cannot load the DT; " \
				"fi; " \
			"fi; " \
		"else " \
			"bootz; " \
		"fi;\0" \
	"mmcboot=mmc dev ${mmcdev}; " \
		"setenv devnum ${mmcdev}; " \
		"setenv devtype mmc; " \
		"if mmc rescan; then " \
			"echo SD/MMC found on device ${mmcdev};" \
			"if run loadimage; then " \
				"if test ${boot_fit} -eq 1; then " \
					"run loadfit; " \
				"else " \
					"run mmcloados;" \
				"fi;" \
			"fi;" \
		"fi;\0"
#endif /* __CONFIG_TQMA57XX_H */
