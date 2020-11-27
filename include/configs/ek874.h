/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * include/configs/ek874.h
 *     This file is ek874 board configuration.
 *
 * Copyright (C) 2019 Renesas Electronics Corporation
 */

#ifndef __EK874_H
#define __EK874_H

#undef DEBUG

#include "rcar-gen3-common.h"

/* Ethernet RAVB */
#define CONFIG_NET_MULTI
#define CONFIG_BITBANGMII
#define CONFIG_BITBANGMII_MULTI

/* Generic Timer Definitions (use in assembler source) */
#define COUNTER_FREQUENCY	0xFE502A	/* 16.66MHz from CPclk */

/* Environment in eMMC, at the end of 2nd "boot sector" */
#define CONFIG_SYS_MMC_ENV_DEV		2
#define CONFIG_SYS_MMC_ENV_PART		2
#undef CONFIG_ENV_SIZE_REDUND
#undef CONFIG_ENV_SECT_SIZE
#define CONFIG_ENV_SECT_SIZE	(64 * 1024)
#define CONFIG_ENV_OFFSET	0x3F0000
#define CONFIG_BOARD_LATE_INIT

#endif /* __EK874_H */
