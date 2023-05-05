/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2020-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#if !defined(__TQMA8MXML_MBA8MX_H)
#define __TQMA8MXML_MBA8MX_H

/* Serial, SERIAL_MXC does not support DM yet */
#define CONFIG_MXC_UART_BASE		UART3_BASE_ADDR
#define CONSOLE_DEV			"ttymxc2"

#define BB_ENV_SETTINGS \
	"console=" CONSOLE_DEV "\0" \
	"addearlycon=setenv bootargs ${bootargs} " \
		"earlycon=ec_imx6q," __stringify(CONFIG_MXC_UART_BASE) \
		",${baudrate}\0" \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0"

#endif /* __TQMA8MXML_MBA8MX_H */
