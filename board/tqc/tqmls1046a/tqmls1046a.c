// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 TQ-Systems GmbH
 */

#include <common.h>
#include <i2c.h>
#include <fdt_support.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/ppa.h>
#include <asm/arch/soc.h>
#include <hwconfig.h>
#include <ahci.h>
#include <mmc.h>
#include <scsi.h>
#include <fm_eth.h>
#include <fsl_csu.h>
#include <fsl_esdhc.h>
#include <fsl_sec.h>
#include "tqmls1046a_bb.h"

DECLARE_GLOBAL_DATA_PTR;


int board_early_init_f(void)
{
	fsl_lsch2_early_init_f();

	return tqmls1046a_bb_board_early_init_f();
}

#ifndef CONFIG_SPL_BUILD
int checkboard(void)
{
	printf("Board: TQMLS1046A on a %s\n", tqmls1046a_bb_get_boardname());

	/*
	 * TODO: add further information (e.g. boot source, CPLD & SysC firmware
	 * version, ...)
	 */
 
	return tqmls1046a_bb_checkboard();
}

int board_init(void)
{
#ifdef CONFIG_SECURE_BOOT
	/*
	 * In case of Secure Boot, the IBR configures the SMMU
	 * to allow only Secure transactions.
	 * SMMU must be reset in bypass mode.
	 * Set the ClientPD bit and Clear the USFCFG Bit
	 */
	u32 val;
	val = (in_le32(SMMU_SCR0) | SCR0_CLIENTPD_MASK) & ~(SCR0_USFCFG_MASK);
	out_le32(SMMU_SCR0, val);
	val = (in_le32(SMMU_NSCR0) | SCR0_CLIENTPD_MASK) & ~(SCR0_USFCFG_MASK);
	out_le32(SMMU_NSCR0, val);
#endif

#ifdef CONFIG_FSL_CAAM
	sec_init();
#endif

#ifdef CONFIG_FSL_LS_PPA
	ppa_init();
#endif

	return tqmls1046a_bb_board_init();
}

int board_setup_core_volt(u32 vdd)
{
	/* TODO: core voltage could be changed from SysC on TQMLS10xxA */
	return 0;
}

int get_serdes_volt(void)
{
	/* TODO: serdes voltage is connected to core voltage on TQMLS10xxA */
	return 1000;
}

int set_serdes_volt(int svdd)
{
	/* TODO: serdes voltage is connected to core voltage on TQMLS10xxA */
	return 0;
}

int power_init_board(void)
{
	/* TODO: anything to do here? */
	setup_chip_volt();

	return 0;
}

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	return tqmls1046a_bb_misc_init_r();
}
#endif

int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);

#ifdef CONFIG_SYS_DPAA_FMAN
	fdt_fixup_fman_ethernet(blob);
#endif

	return tqmls1046a_bb_ft_board_setup(blob, bd);
}
#endif
