/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2013, 2014, 2016 - 2022 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#ifndef __TQC_BB__
#define __TQC_BB__

struct mmc;
struct bd_info;

int tq_bb_board_mmc_getwp(struct mmc *mmc);
int tq_bb_board_mmc_getcd(struct mmc *mmc);
int tq_bb_board_mmc_init(struct bd_info *bis);

void tq_bb_board_init_f(ulong dummy);
int tq_bb_board_early_init_f(void);
int tq_bb_board_init(void);
int tq_bb_board_late_init(void);
int tq_bb_checkboard(void);
void tq_bb_board_quiesce_devices(void);

const char *tq_bb_get_boardname(void);

#if defined(CONFIG_SPL_BUILD)
void tq_bb_spl_board_init(void);
#endif

/*
 * Device Tree Support
 */
#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
struct bd_info;

int tq_bb_ft_board_setup(void *blob, struct bd_info *bis);
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */

#endif /* __TQC_BB_H */
