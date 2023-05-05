/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2019-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#if !defined(__TQMA8MX_MBA8MX_H)
#define __TQMA8MX_MBA8MX_H

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

#define CONFIG_MXC_UART_BASE		UART3_BASE_ADDR
#define CONSOLE_DEV			"ttymxc2"

#define BB_ENV_SETTINGS \
	"console=" CONSOLE_DEV "\0" \
	"addearlycon=setenv bootargs ${bootargs} " \
		"earlycon=ec_imx6q," __stringify(CONFIG_MXC_UART_BASE) \
		",${baudrate}\0" \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0"

#endif /* __TQMA8MX_MBA8MX_H */
