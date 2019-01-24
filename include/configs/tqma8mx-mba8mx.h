/*
 * Copyright 2019 TQ Systems GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#if !defined(__TQMA8MX_MBA8MX_H)
#define __TQMA8MX_MBA8MX_H

/* ENET Config */
/* ENET1 */
#if defined(CONFIG_CMD_NET)

#define CONFIG_MII
#define CONFIG_ETHPRIME			"FEC"

#define CONFIG_FEC_XCV_TYPE		RGMII
#define CONFIG_FEC_MXC_PHYADDR		0
#define FEC_QUIRK_ENET_MAC

/* #define CONFIG_PHY_GIGE */
#endif

#if !defined(CONFIG_SPL_BUILD)
/* #define CONFIG_DM_GPIO */
#define CONFIG_DM_PCA953X
#endif

/* Framebuffer */
#ifdef CONFIG_VIDEO
#define CONFIG_VIDEO_IMXDCSS
#define CONFIG_VIDEO_BMP_RLE8
#define CONFIG_SPLASH_SCREEN
#define CONFIG_SPLASH_SCREEN_ALIGN
#define CONFIG_BMP_16BPP
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_BMP_LOGO
#define CONFIG_IMX_VIDEO_SKIP
#endif

#endif /* __TQMA8MX_MBA8MX_H */
