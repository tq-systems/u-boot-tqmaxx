/*
 * Copyright 2018 TQ Systems GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#if !defined(__TQMA8QXS_MB_SMARC_2_H)
#define __TQMA8QXS_MB_SMARC_2_H

/* TODO: move to module ... */

#define CONFIG_FEC_MXC
#define CONFIG_FEC_XCV_TYPE		RGMII
#define FEC_QUIRK_ENET_MAC

/*
#define CONFIG_PHY_GIGE
#define CONFIG_PHYLIB
#define CONFIG_PHY_TI
*/

#define CONFIG_MII

#define MX8QX_FEC1_BASE			0x5B040000U
#define MX8QX_FEC2_BASE			0x5B050000U
#define CONFIG_FEC_MXC_MDIO_BASE	MX8QX_FEC1_BASE

#define CONFIG_FEC_MXC_PHYADDR		0x1F

#define BB_ENV_SETTINGS \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0"

#endif /* __TQMA8QXS_MB_SMARC_2_H */
