/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright 2020 - 2021 TQ-Systems GmbH
 */

#if !defined(__TQMA8MXML_MBA8MX_H)
#define __TQMA8MXML_MBA8MX_H

/* Serial */
#define CONFIG_MXC_UART_BASE		UART3_BASE_ADDR

#if (CONFIG_MXC_UART_BASE == UART1_BASE_ADDR)
#define CONSOLE_DEV			"ttymxc0"
#elif (CONFIG_MXC_UART_BASE == UART2_BASE_ADDR)
#define CONSOLE_DEV			"ttymxc1"
#elif (CONFIG_MXC_UART_BASE == UART3_BASE_ADDR)
#define CONSOLE_DEV			"ttymxc2"
#else
#error
#endif

#define BB_ENV_SETTINGS \
	"console=" CONSOLE_DEV "\0" \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0"

#endif /* __TQMA8MXML_MBA8MX_H */
