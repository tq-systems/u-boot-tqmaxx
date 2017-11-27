/*
 * Copyright (C) 2017 Markus Niebel <Markus.Niebel@tq-group.com>
 *
 * Configuration settings for the TQ Systems TQMa6D module on NAV baseboard.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_TQMA6_NAV_H
#define __CONFIG_TQMA6_NAV_H

#if defined(CONFIG_TQMA6S) || defined(CONFIG_TQMA6DL)
#error
#elif defined(CONFIG_TQMA6Q)
#define CONFIG_DEFAULT_FDT_FILE		"imx6q-nav-hdcp.dtb"
#endif

#define CONFIG_FEC_XCV_TYPE		RMII
#define CONFIG_ETHPRIME			"FEC"

#define CONFIG_FEC_MXC_PHYADDR		0x0
#define CONFIG_PHY_SMSC

#define CONFIG_MXC_UART_BASE		UART2_BASE
#define CONFIG_CONSOLE_DEV		"ttymxc1"

/*
 * Remove all unused interfaces / commands that are defined in
 * the common header tqma6.h
 */
#undef CONFIG_CMD_SF
#undef CONFIG_CMD_SPI
#undef CONFIG_MXC_SPI

#if defined(CONFIG_SECURE_BOOT)
#undef CONFIG_BMODE
#undef CONFIG_FIT

#undef CONFIG_SUPPORT_EMMC_RPMB
#undef CONFIG_CMD_NET
#undef CONFIG_FEC_MXC
#undef CONFIG_CMD_PING
#undef CONFIG_CMD_DHCP
#undef CONFIG_CMD_MII
#undef CONFIG_PHYLIB
#undef CONFIG_MII
#undef CONFIG_CMD_I2C
#undef CONFIG_CMD_DTT
#undef CONFIG_CMD_EEPROM
#undef CONFIG_CMD_PMIC
#undef CONFIG_CMD_FUSE

#undef CONFIG_USB_EHCI
#undef CONFIG_CMD_USB
#undef CONFIG_USB
#undef CONFIG_USB_EHCI_MX6
#undef CONFIG_USB_ETHER_SMSC95XX
#undef CONFIG_USB_HOST_ETHER

#define CONFIG_SILENT_CONSOLE
#define CONFIG_SYS_DEVICE_NULLDEV
/* TODO: set to -1 to force no wait at all */
#if defined(CONFIG_BOOTDELAY)
#undef CONFIG_BOOTDELAY
#endif
#define CONFIG_BOOTDELAY 0

#endif

/* Watchdog */
#define CONFIG_HW_WATCHDOG
#define CONFIG_IMX_WATCHDOG
#define CONFIG_WATCHDOG_TIMEOUT_MSECS 60000

#define	TQMA6_BASEBOARD_ENV_SETTINGS                                           \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0"                               \
	"firmwarepath=/boot\0"                                                 \
	"firmwarepart=2\0"                                                     \
	"zimage=zImage\0"                                                      \

#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + \
					 CONFIG_ENV_SIZE)

/* TODO: is this the right check? */
#if defined(CONFIG_TQMA6_SUPPORT_MENDERIO)

#define CONFIG_DISABLE_CONSOLE

/*
 * Define the variables for mender integration
 * see: https://docs.mender.io/1.2/devices/integrating-with-u-boot#integration-points
 */
#define CONFIG_BOOTCOUNT_LIMIT
#define CONFIG_BOOTCOUNT_ENV

/*
 * Mender: error CONFIG_SYS_MMC_ENV_DEV should not be defined explicitly
 * (will be auto-configured).
 */
#undef CONFIG_SYS_MMC_ENV_DEV
/*
 * Mender: error CONFIG_ENV_OFFSET should not be defined explicitly
 * (will be auto-configured).
 */
#undef CONFIG_ENV_OFFSET

#undef CONFIG_SYS_REDUNDAND_ENVIRONMENT
#undef CONFIG_ENV_OFFSET_REDUND

#undef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND \
        "run mender_setup; run mmcboot; run mender_try_to_recover; run panicboot"

#undef TQMA6_BASEBOARD_ENV_SETTINGS
#define	TQMA6_BASEBOARD_ENV_SETTINGS                                           \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0"                               \
	"firmwarepath=/boot\0"                                                 \
	"firmwarepart=2\0"                                                     \
	"zimage=zImage\0"                                                      \
	"mender_uboot_root=${mmcdev}:${firmwarepart}\0"                        \
	"mender_kernel_root=/dev/mmcblk${mmcblkdev}p${mmcpart}\0"              \
	"loadimage=load ${mender_uboot_root} ${loadaddr} "                     \
		"${firmwarepath}/${zimage}\0"                                  \
	"loadfdt=load ${mender_uboot_root} ${fdt_addr} "                       \
		"${firmwarepath}/${fdt_file}\0"                                \
	"addmmc=setenv bootargs ${bootargs} "                                  \
		"root=${mender_kernel_root} ${rootfsmode} "                    \
		"rootwait\0"                                                   \

#endif

#endif /* __CONFIG_TQMA6_NAV_H */
