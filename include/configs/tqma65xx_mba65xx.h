/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Configuration header file for TQ-Systems TQMa65xx on MBa65xx baseboard
 *
 * Copyright (C) 2017-2018 Texas Instruments Incorporated - http://www.ti.com/
 * Copyright (C) 2020-2022 TQ-Systems GmbH
 */

#ifndef __CONFIG_TQMA65XX_MBA65XX_H
#define __CONFIG_TQMA65XX_MBA65XX_H

#define EXTRA_ENV_TQMA65XX_SETTINGS_BOARD \
	"args_board=setenv bootargs ${bootargs} " \
		"earlycon=ns16550a,mmio32,0x02800000 " \
		"console=ttyS2,115200n8\0" \
	""

#include "tqma65xx.h"

#endif /* __CONFIG_TQMA65XX_MBA65XX_H */
