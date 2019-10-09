/*
 * Copyright 2019 TQ Systems GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#if !defined(__TQMA8X_MBA8X_H)
#define __TQMA8X_MBA8X_H

#define CONFIG_FEC_MXC
#define CONFIG_FEC_XCV_TYPE		RGMII
#define FEC_QUIRK_ENET_MAC

/* #define CONFIG_PHY_GIGE */ /* Support for 1000BASE-X */
/* #define CONFIG_PHYLIB */
/* #define CONFIG_PHY_TI */
#define CONFIG_MII

#define MX8QM_FEC1_BASE			0x5B040000U
#define MX8QM_FEC2_BASE			0x5B050000U
#define CONFIG_FEC_MXC_MDIO_BASE	MX8QM_FEC1_BASE

#define CONFIG_FEC_MXC_PHYADDR		0x1F

#define BB_ENV_SETTINGS \
	"console=ttyLP0,"__stringify(CONFIG_BAUDRATE)"\0" \
	"earlycon=lpuart32,5a060000,"__stringify(CONFIG_BAUDRATE)"\0" \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0"

#endif /* __TQMA8X_MBA8X_H */
