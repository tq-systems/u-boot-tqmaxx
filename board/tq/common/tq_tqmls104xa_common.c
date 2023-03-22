// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Gregor Herburger
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/io.h>
#include <common.h>
#include <fsl_immap.h>

#include "../common/tq_bb.h"

#define SCFG_QSPI_CLKSEL_DIV_24            0x30100000

#if defined(CONFIG_TFABOOT) && defined(CONFIG_ENV_IS_IN_SPI_FLASH)
void *env_sf_get_env_addr(void)
{
	return (void *)(CONFIG_SYS_FSL_QSPI_BASE + CONFIG_ENV_OFFSET);
}
#endif

int board_early_init_f(void)
{
	struct ccsr_scfg *scfg = (struct ccsr_scfg *)CONFIG_SYS_FSL_SCFG_ADDR;

	fsl_lsch2_early_init_f();
#ifdef CONFIG_FSL_QSPI
	/* divide CGA1/CGA2 PLL by 24 to get QSPI interface clock */
	out_be32(&scfg->qspi_cfg, SCFG_QSPI_CLKSEL_DIV_24);
#endif

	return tq_bb_board_early_init_f();
}

#if IS_ENABLED(CONFIG_OF_BOARD_FIXUP)
int board_fix_fdt(void *blob)
{
	return tq_bb_board_fix_fdt(blob);
}
#endif

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	return tq_bb_board_misc_init_r();
}
#endif
