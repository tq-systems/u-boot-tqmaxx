/*
 * (C) Copyright 2014
 * Texas Instruments Incorporated.
 * Felipe Balbi <balbi@ti.com>
 *
 * Copyright (C) 2017 TQ Systems (ported AM57xx IDK to TQMa57xx)
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
#define CONFIG_NR_DRAM_BANKS		2
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
#define CONFIG_ENV_EEPROM_IS_ON_I2C
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50	/* Main EEPROM */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2

/* PMIC I2C bus number */
#define CONFIG_SYS_SPD_BUS_NUM		0

/* PCA9555 GPIO expander support */
#define CONFIG_PCA953X

/* SD/eMMC */
#define CONFIG_SUPPORT_EMMC_BOOT
#define CONFIG_HSMMC2_8BIT

/* CPSW Ethernet */
#define CONFIG_BOOTP_DNS		/* Configurable parts of CMD_DHCP */
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_NET_RETRY_COUNT		10
#define CONFIG_DRIVER_TI_CPSW		/* Driver for IP block */
#define CONFIG_MII			/* Required in net/eth.c */
#define PHY_ANEG_TIMEOUT	8000	/* PHY needs longer aneg time at 1G */

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
#define CONFIG_TI_SPI_MMAP
#define CONFIG_SF_DEFAULT_SPEED                15360000
#define CONFIG_SF_DEFAULT_MODE                 SPI_MODE_0
#define CONFIG_QSPI_QUAD_SUPPORT
#define TQMA57XX_SPI_FLASH_SECTOR_SIZE		SZ_64K

/* SPI SPL */
#define CONFIG_TI_EDMA3
#define CONFIG_SPL_SPI_LOAD

/*
 * QSPI flash map
 *
 * Default to using SPI for environment, etc.
 * 0x000000 - 0x040000 : QSPI.SPL (256KiB)
 * 0x040000 - 0x140000 : QSPI.u-boot (1MiB)
 * 0x140000 - 0x1C0000 : QSPI.u-boot-spl-os (512KiB)
 * 0x1C0000 - 0x1D0000 : QSPI.u-boot-env (64KiB)
 * 0x1D0000 - 0x1E0000 : QSPI.u-boot-env.backup1 (64KiB)
 * 0x1E0000 - 0x9E0000 : QSPI.kernel (8MiB)
 * 0x9E0000 - 0x2000000 : USERLAND
 */
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x40000
#define TQMA57XX_SPI_ENV_OFFS		0x1C0000
#define CONFIG_SYS_SPI_KERNEL_OFFS      0x1E0000
#define CONFIG_SYS_SPI_ARGS_OFFS        0x140000
#define CONFIG_SYS_SPI_ARGS_SIZE        0x80000

/* SPI ENV */
#define CONFIG_ENV_SPI_BUS              (CONFIG_SF_DEFAULT_BUS)
#define CONFIG_ENV_SPI_CS               (CONFIG_SF_DEFAULT_CS)
#define CONFIG_ENV_SPI_MAX_HZ           (CONFIG_SF_DEFAULT_SPEED)
#define CONFIG_ENV_SPI_MODE             (CONFIG_SF_DEFAULT_MODE)

#define CONFIG_ENV_OFFSET		(TQMA57XX_SPI_ENV_OFFS)
#define CONFIG_ENV_SIZE                 SZ_64K
#define CONFIG_ENV_SECT_SIZE            TQMA57XX_SPI_FLASH_SECTOR_SIZE
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_OFFSET_REDUND        (CONFIG_ENV_OFFSET + \
					 CONFIG_ENV_SECT_SIZE)

/* Default environment */
#include <environment/ti/boot.h>
#include <environment/ti/mmc.h>

#ifdef CONFIG_CMD_NET
#define NETARGS_TQMA57XX \
	"static_ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}" \
		"::off\0" \
	"nfsopts=nolock\0" \
	"rootpath=/srv/nfs/rootfs\0" \
	"netloadimage=nfs ${loadaddr} $serverip:$rootpath$bootdir/$bootfile\0" \
	"netloadfdt=nfs ${fdtaddr} $serverip:$rootpath$bootdir/$fdtfile\0" \
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

#define CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
#define CONFIG_EXTRA_ENV_SETTINGS \
	DEFAULT_LINUX_BOOT_ENV \
	DEFAULT_MMC_TI_ARGS \
	DEFAULT_COMMON_BOOT_TI_ARGS \
	DEFAULT_FIT_TI_ARGS \
	NETARGS_TQMA57XX \
	"bootfile=zImage_tqma572x-mba57xx.bin \0" \
	"fdtfile=tqma572x-mba57xx.dtb \0" \
	"devtype=mmc \0" \
	"upd_sd=if tftp u-boot/u-boot_tqma572x.img; " \
		"then echo updating u-boot on sd card...; " \
		"fatwrite mmc 0:1 $loadaddr u-boot.img $filesize; " \
		"tftp u-boot/MLO_tqma572x; " \
		"fatwrite mmc 0:1 $loadaddr MLO $filesize; " \
		" else echo u-boot file not found!; fi \0" \
	"upd_spi=if tftp u-boot/u-boot_tqma572x.img; " \
		"then echo updating u-boot on spi flash...; " \
		"sf probe 0; sf erase 0x00000 0x100000; " \
		"sf write ${loadaddr} 0x40000 ${filesize}; " \
		"tftp u-boot/MLO_tqma572x; " \
		"sf write ${loadaddr} 0x00000 ${filesize}; " \
		"else echo u-boot file not found!; fi \0" \
	"bootcmd=if test ${dofastboot} -eq 1; then echo " \
		"Boot fastboot requested, resetting dofastboot ...; " \
		"setenv dofastboot 0; saveenv; " \
		"echo Booting into fastboot ...; fastboot 1; fi; " \
		"if test ${boot_fit} -eq 1; then run update_to_fit;fi; " \
		"run envboot; run mmcboot; \0 " \
	"boot_fit=0\0"
#endif /* __CONFIG_TQMA57XX_H */
