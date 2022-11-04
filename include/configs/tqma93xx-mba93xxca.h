/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2022-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#if !defined(__TQMA93XX_MBA93XXCA_H)
#define __TQMA93XX_MBA93XXCA_H

#ifdef CONFIG_DWC_ETH_QOS

#define PHY_ANEG_TIMEOUT			20000

#endif /* CONFIG_DWC_ETH_QOS */

#define CFG_SYS_FSL_USDHC_NUM			2

#define CONSOLE_DEV				"ttyLP0"

#define BB_ENV_SETTINGS \
	"console=" CONSOLE_DEV "\0" \

#endif /* __TQMA93XX_MBA93XXCA_H */
