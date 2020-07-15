/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013, 2014, 2016 - 2020 TQ Systems
 * Author: Markus Niebel <markus.niebel@tq-group.com>
 */

#ifndef __TQC_BB__
#define __TQC_BB__

#include <common.h>

int tqc_bb_board_mmc_getwp(struct mmc *mmc);
int tqc_bb_board_mmc_getcd(struct mmc *mmc);
int tqc_bb_board_mmc_init(bd_t *bis);

void tqc_bb_board_init_f(ulong dummy);
int tqc_bb_board_early_init_f(void);
int tqc_bb_board_init(void);
int tqc_bb_board_late_init(void);
int tqc_bb_checkboard(void);
void tqc_bb_board_quiesce_devices(void);

const char *tqc_bb_get_boardname(void);

#if defined(CONFIG_SPL_BUILD)
void tqc_bb_spl_board_init(void);
#endif

/*
 * Device Tree Support
 */
#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
int tqc_bb_ft_board_setup(void *blob, bd_t *bd);
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */

#if defined(CONFIG_TQC_RTC) && (CONFIG_DM_I2C)
int tqc_pcf85063_adjust_capacity(int bus, int address, int quartz_load);
#endif /* CONFIG_TQC_RTC */

#endif
