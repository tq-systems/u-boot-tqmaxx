/*
 * Copyright 2019 TQ Systems GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#if !defined(__TQMA8XX_MBPA8XX_H)
#define __TQMA8XX_MBPA8XX_H

#define CONFIG_FEC_MXC
#define CONFIG_FEC_XCV_TYPE		RGMII
#define FEC_QUIRK_ENET_MAC

#define CONFIG_MII

#define MX8QX_FEC1_BASE			0x5B040000U
#define MX8QX_FEC2_BASE			0x5B050000U
#define CONFIG_FEC_MXC_MDIO_BASE	MX8QX_FEC1_BASE

#define CONFIG_FEC_MXC_PHYADDR		0x1F

#define BB_ENV_SETTINGS \
	"console=ttyLP1,115200 earlycon=lpuart32,0x5a070000\0" \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0"

#endif /* __TQMA8XX_MBPA8XX_H */
