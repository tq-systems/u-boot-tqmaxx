/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2024 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#if !defined(__TQMA95XXSA_MB_SMARC_2_H)
#define __TQMA8XXS_MB_SMARC_2_H

#define CFG_SYS_FSL_USDHC_NUM			2

#define CONSOLE_DEV				"ttyLP6"

#define BB_ENV_SETTINGS \
	"console=" CONSOLE_DEV ",115200\0" \

#endif /* __TQMA8XXS_MB_SMARC_2_H */
