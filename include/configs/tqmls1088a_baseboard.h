/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 TQ-Systems GmbH
 */

#ifndef __TQMLS1088A_BASEBOARD_H__
#define __TQMLS1088A_BASEBOARD_H__

#if defined(CONFIG_TQMLS1088A_BB_MBLS10XXA)
	#include "tqc_bb_mbls10xxa.h"
#else
	#error "No baseboard selected for TQMLS1088A SoC module!"
#endif

#endif /* __TQMLS1088A_BASEBOARD_H__ */
