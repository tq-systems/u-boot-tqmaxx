/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2019-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Timo Herbrecher, Gregor Herburger
 */

#ifndef __TQMLS1043A_BASEBOARD_H__
#define __TQMLS1043A_BASEBOARD_H__

#if defined(CONFIG_TQMLS1043A_BB_MBLS10XXA)
	#include "tqc_bb_mbls10xxa.h"
#else
	#error "No baseboard selected for TQMLS1043A SoC module!"
#endif

#endif /* __TQMLS1043A_BASEBOARD_H__ */
