/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * include/configs/hihope-rzg2n.h
 *     This file is HOPERUN HiHope-RZG2M board configuration.
 *
 * Copyright (C) 2019 Renesas Electronics Corporation
 */

#ifndef __HIHOPE_RZG2N_H
#define __HIHOPE_RZG2N_H

#include "rcar-gen3-common.h"

/* Ethernet RAVB */
#define CONFIG_BITBANGMII
#define CONFIG_BITBANGMII_MULTI

/* Generic Timer Definitions (use in assembler source) */
#define COUNTER_FREQUENCY	0xFE502A	/* 16.66MHz from CPclk */

/* Environment in eMMC, at the end of 2nd "boot sector" */
#define CONFIG_ENV_OFFSET		(-CONFIG_ENV_SIZE)
#define CONFIG_SYS_MMC_ENV_DEV		1
#define CONFIG_SYS_MMC_ENV_PART		2

#define CONFIG_BOARD_LATE_INIT

#endif /* __HIHOPE_RZG2N_H */
