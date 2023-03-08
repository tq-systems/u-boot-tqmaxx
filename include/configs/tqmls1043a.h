/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2019-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Timo Herbrecher, Gregor Herburger
 */

#ifndef __TQMLS1043A_H__
#define __TQMLS1043A_H__

#include "ls1043a_common.h"
#include "tqmls10xxa_common.h"

#define CONFIG_LAYERSCAPE_NS_ACCESS

#undef CONFIG_EXTRA_ENV_SETTINGS
/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"fmucode=fsl_fman_ucode_ls1043_r1.1_106_4_18.bin\0"	\
	TQMLS10XXA_COMMON_ENV

#include <asm/fsl_secure_boot.h>

#include "tqmls1043a_baseboard.h"

#endif /* __TQMLS1043A_H__ */
