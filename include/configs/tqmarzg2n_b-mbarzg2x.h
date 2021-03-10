/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * include/configs/tqmrzg2n_b-mbarzg2x.h
 *     This file is TQMaRZG2N-B (2GiB) module on MBaRZG2x board configuration.
 *
 * Copyright (C) 2019 Renesas Electronics Corporation
 * Copyright (C) 2021 TQ-Systems GmbH
 */

#ifndef __TQMARZG2N_B_MBARZG2X_H
#define __TQMARZG2N_B_MBARZG2X_H

#include "rcar-gen3-common.h"

/* Ethernet RAVB */
#define CONFIG_BITBANGMII
#define CONFIG_BITBANGMII_MULTI

/* Generic Timer Definitions (use in assembler source) */
#define COUNTER_FREQUENCY	0x17D7840	/* 25.00MHz from CPclk */

/* Environment in eMMC, at the end of 2nd "boot sector" */
#define CONFIG_ENV_OFFSET		(-CONFIG_ENV_SIZE)
#define CONFIG_SYS_MMC_ENV_DEV		1
#define CONFIG_SYS_MMC_ENV_PART		2

#define CONFIG_BOARD_LATE_INIT

/* override bootcmd from rcar-gen3-common.h */

#undef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND	\
	"fatload mmc 0:1 0x48080000 Image; " \
	"fatload mmc 0:1 0x48000000 " CONFIG_DEFAULT_FDT_FILE "; " \
	"booti 0x48080000 - 0x48000000"

#endif /*__TQMARZG2N_B_MBARZG2X_H */
