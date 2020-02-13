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

/* must be defined but not used for DM */
#define CONFIG_FEC_MXC_PHYADDR		0x1F

#define BB_ENV_SETTINGS \
	"console=ttyLP0,"__stringify(CONFIG_BAUDRATE)"\0" \
	"earlycon=lpuart32,0x5a060000\0" \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0"

#endif /* __TQMA8X_MBA8X_H */
