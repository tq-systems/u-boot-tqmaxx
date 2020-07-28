/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 TQ Systems GmbH
 */

#if !defined(__TQMA8MXML_MBA8MX_H)
#define __TQMA8MXML_MBA8MX_H

/* ENET Config */
/* ENET1 */
#if defined(CONFIG_FEC_MXC)

#define CONFIG_ETHPRIME			"FEC"
#define FEC_QUIRK_ENET_MAC

#endif

#if !defined(CONFIG_SPL_BUILD)
#define CONFIG_DM_PCA953X
#endif

#define CONFIG_BAUDRATE			115200
#define CONFIG_MXC_UART_BASE		UART3_BASE_ADDR

#if (CONFIG_MXC_UART_BASE == UART1_BASE_ADDR)
#define CONSOLE_DEV			"ttymxc0"
#elif (CONFIG_MXC_UART_BASE == UART3_BASE_ADDR)
#define CONSOLE_DEV			"ttymxc2"
#elif (CONFIG_MXC_UART_BASE == UART2_BASE_ADDR)
#define CONSOLE_DEV			"ttymxc1"
#else
#error
#endif

#define BB_ENV_SETTINGS \
	"console=" CONSOLE_DEV "," __stringify(CONFIG_BAUDRATE) \
		" earlycon=ec_imx6q," __stringify(CONFIG_MXC_UART_BASE) "," \
		__stringify(CONFIG_BAUDRATE) "\0" \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0"

#endif /* __TQMA8MXML_MBA8MX_H */
