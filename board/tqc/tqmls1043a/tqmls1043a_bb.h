/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019 TQ Systems
 */

#ifndef __TQMLS1043A_BB__
#define __TQMLS1043A_BB__

#include <common.h>
#include <mmc.h>

/*
 * baseboard specific initialization functions
 */
int tqmls1043a_bb_board_early_init_f(void);
int tqmls1043a_bb_board_init(void);
int tqmls1043a_bb_misc_init_r(void);
int tqmls1043a_bb_checkboard(void);
const char *tqmls1043a_bb_get_boardname(void);
#ifdef CONFIG_NET
int tqmls1043a_bb_board_eth_init(bd_t *bis);
#endif
int tqmls1043a_bb_board_mmc_getcd(struct mmc *mmc);
int tqmls1043a_bb_board_mmc_getwp(struct mmc *mmc);

/*
 * Device Tree Support
 */
#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
int tqmls1043a_bb_ft_board_setup(void *blob, bd_t *bd);
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */

#endif /* __TQMLS1043A_BB__ */
