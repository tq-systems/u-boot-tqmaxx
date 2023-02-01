/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2021 - 2022 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#if !defined(__TQMA93XX_MBA93XXCA_H)
#define __TQMA93XX_MBA93XXCA_H

#if defined(CONFIG_FEC_MXC)

#define FEC_QUIRK_ENET_MAC
#define CONFIG_FEC_XCV_TYPE			RGMII

#endif /* CONFIG_FEC_MXC */

#ifdef CONFIG_DWC_ETH_QOS

#define CONFIG_SYS_NONCACHED_MEMORY		(1 * SZ_1M)
#define PHY_ANEG_TIMEOUT			20000

#endif /* CONFIG_DWC_ETH_QOS */

#define CONFIG_SYS_FSL_USDHC_NUM		2

#define CONFIG_BAUDRATE				115200
#define CONSOLE_DEV				"ttyLP0"

#define CONFIG_SERIAL_TAG

#define BB_ENV_SETTINGS \
	"console=" CONSOLE_DEV "\0" \
	"addearlycon=setenv bootargs ${bootargs} \0" \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0"

#define CONFIG_SYS_I2C_SPEED			100000

/* USB configs */
#define CONFIG_USB_MAX_CONTROLLER_COUNT	2

#if defined(CONFIG_CMD_NET)
#define CONFIG_ETHPRIME				"eth1"
#endif

#endif /* __TQMA93XX_MBA93XXCA_H */
