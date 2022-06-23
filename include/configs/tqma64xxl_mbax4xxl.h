/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Configuration header file for TQ-Systems TQMa64xxL on MBaX4XxL baseboard
 *
 * Copyright (C) 2017-2018 Texas Instruments Incorporated - http://www.ti.com/
 * Copyright (c) 2020-2022 TQ-Systems GmbH <license@tq-group.com>, D-82229 Seefeld, Germany.
 */

#ifndef __CONFIG_TQMA64XXL_MBAX4XXL_H
#define __CONFIG_TQMA64XXL_MBAX4XXL_H

#define EXTRA_ENV_TQMA64XXL_SETTINGS_BOARD \
	"args_board=setenv bootargs ${bootargs} " \
		"earlycon=ns16550a,mmio32,0x02800000 " \
		"console=ttyS2,115200n8\0" \
	"name_overlay=k3-am64-tqma64xxl-mbax4xxl-sdcard.dtbo\0" \
	""

#include "tqma64xxl.h"

#endif /* __CONFIG_TQMA64XXL_MBAX4XXL_H */
