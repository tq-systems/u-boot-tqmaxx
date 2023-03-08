// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <common.h>

#include "tq_bb.h"

struct mmc;

int __weak tq_bb_board_mmc_getwp(struct mmc *mmc)
{
	return 0;
}

int __weak tq_bb_board_mmc_getcd(struct mmc *mmc)
{
	return 0;
}

int __weak tq_bb_board_mmc_init(struct bd_info *bis)
{
	return 0;
}

void __weak tq_bb_board_init_f(ulong dummy)
{
	;
}

int __weak tq_bb_board_early_init_f(void)
{
	return 0;
}

int __weak tq_bb_board_fix_fdt(void *rw_fdt_blob)
{
	return 0;
}

int __weak tq_bb_board_init(void)
{
	return 0;
}

int __weak tq_bb_board_late_init(void)
{
	return 0;
}

int __weak tq_bb_board_misc_init_r(void)
{
	return 0;
}

int __weak tq_bb_checkboard(void)
{
	return 0;
}

void __weak tq_bb_board_quiesce_devices(void)
{
	;
}

const char * __weak tq_bb_get_boardname(void)
{
	return "INVALID";
}

#if defined(CONFIG_SPL_BUILD)
void __weak tq_bb_spl_board_init(void)
{
	;
}

#endif

/*
 * Device Tree Support
 */
#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
int __weak tq_bb_ft_board_setup(void *blob, struct bd_info *bis)
{
	return 0;
}

#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */
