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

#include <configs/ti_omap5_common.h>

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

#endif /* __CONFIG_TQMA57XX_H */
