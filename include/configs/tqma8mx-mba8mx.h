/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 - 2020 TQ Systems GmbH
 */

#if !defined(__TQMA8MX_MBA8MX_H)
#define __TQMA8MX_MBA8MX_H

/* ENET Config */
/* ENET1 */
#if defined(CONFIG_FEC_MXC)

#define CONFIG_ETHPRIME			"FEC"
#define FEC_QUIRK_ENET_MAC

#endif

#if !defined(CONFIG_SPL_BUILD)
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

#define CONFIG_BAUDRATE			115200
#define CONFIG_MXC_UART_BASE		UART3_BASE_ADDR

#if (CONFIG_MXC_UART_BASE == UART1_BASE_ADDR)
#define CONSOLE_DEV			"ttymxc0"
#elif (CONFIG_MXC_UART_BASE == UART3_BASE_ADDR)
#define CONSOLE_DEV			"ttymxc2"
#else
#error
#endif

#define BB_ENV_SETTINGS \
	"console=" CONSOLE_DEV "," __stringify(CONFIG_BAUDRATE) \
		" earlycon=ec_imx6q," __stringify(CONFIG_MXC_UART_BASE) "," \
		__stringify(CONFIG_BAUDRATE) "\0" \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0"

#endif /* __TQMA8MX_MBA8MX_H */
