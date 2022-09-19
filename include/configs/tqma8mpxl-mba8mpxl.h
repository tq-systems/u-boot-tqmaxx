/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2021 - 2022 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#if !defined(__TQMA8MPXL_MBA8MPXL_H)
#define __TQMA8MPXL_MBA8MPXL_H

#if defined(CONFIG_FEC_MXC)

#define FEC_QUIRK_ENET_MAC

#endif /* CONFIG_FEC_MXC */

#ifdef CONFIG_DWC_ETH_QOS

#define CONFIG_SYS_NONCACHED_MEMORY	(1 * SZ_1M)
#define PHY_ANEG_TIMEOUT 20000

#endif /* CONFIG_DWC_ETH_QOS */

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
