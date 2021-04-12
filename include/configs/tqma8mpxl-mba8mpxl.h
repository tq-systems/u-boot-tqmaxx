/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 TQ-Systems GmbH
 */

#if !defined(__TQMA8MPXL_MBA8MPXL_H)
#define __TQMA8MPXL_MBA8MPXL_H

#define CONFIG_BAUDRATE			115200
#define CONFIG_MXC_UART_BASE		UART4_BASE_ADDR
#define CONSOLE_DEV			"ttymxc3"

#define BB_ENV_SETTINGS \
	"console=" CONSOLE_DEV "\0" \
	"addearlycon=setenv bootargs ${bootargs} " \
		"earlycon=ec_imx6q," __stringify(CONFIG_MXC_UART_BASE) \
		",${baudrate}\0" \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0"

#endif /* __TQMA8MPXL_MBA8MPXL_H */
