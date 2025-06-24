/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2025 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Paul Gerber
 */

#if !defined(__TQMA8MPXS_MBA8MPXS_H)
#define __TQMA8MPXS_MBA8MPXS_H

#if defined(CONFIG_FEC_MXC)

#define FEC_QUIRK_ENET_MAC

#endif /* CONFIG_FEC_MXC */

#ifdef CONFIG_DWC_ETH_QOS

#define PHY_ANEG_TIMEOUT 20000

#endif /* CONFIG_DWC_ETH_QOS */

#define CFG_MXC_UART_BASE		UART3_BASE_ADDR
#define CONSOLE_DEV			"ttymxc2"

#define BB_ENV_SETTINGS \
	"console=" CONSOLE_DEV "\0" \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0"

#endif /* __TQMA8MPXS_MBA8MPXS_H */
