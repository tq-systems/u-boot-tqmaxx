/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 TQ-Systems GmbH
 */

#ifndef __DDR_H__
#define __DDR_H__

extern void erratum_a008850_post(void);

#if defined(CONFIG_SYS_DDR_RAW_TIMING) || defined(CONFIG_FSL_DDR_INTERACTIVE)
struct board_specific_parameters {
	u32 n_ranks;
	u32 datarate_mhz_high;
	u32 rank_gb;
	u32 clk_adjust;
	u32 wrlvl_start;
	u32 wrlvl_ctl_2;
	u32 wrlvl_ctl_3;
	u32 cpo_override;
	u32 write_data_delay;
	u32 force_2t;
};

/*
 * These tables contain all valid speeds we want to override with board
 * specific parameters. datarate_mhz_high values need to be in ascending order
 * for each n_ranks group.
 */
static const struct board_specific_parameters udimm0[] = {
	/*
	 * memory controller 0
	 *   num|  hi| rank|  clk| wrlvl |   wrlvl   |  wrlvl | cpo  |wrdata|2T
	 * ranks| mhz| GB  |adjst| start |   ctl2    |  ctl3  |      |delay |
	 */
#ifdef CONFIG_SYS_FSL_DDR4
	{1,  1600, 0, 8,     9, 0x06070707, 0x07070707,},
#endif
	{}
};

static const struct board_specific_parameters *udimms[] = {
	udimm0,
};
#endif

fsl_ddr_cfg_regs_t ddr_cfg_regs_1600 = {
#ifdef CONFIG_TQMLS1043A_DDR_2G
	.cs[0].bnds         = 0x0000007F,
	.cs[0].config       = 0x80010412,
#else
	.cs[0].bnds         = 0x0000003F,
	.cs[0].config       = 0x80010312,
#endif
	.cs[0].config_2     = 0x00000000,
	.cs[1].bnds         = 0x0040007F,
	.cs[1].config       = 0x00000202,
	.cs[1].config_2     = 0x00000000,
	.cs[2].bnds         = 0x008000BF,
	.cs[2].config       = 0x00000202,
	.cs[2].config_2     = 0x00000000,
	.cs[3].bnds         = 0x00C000FF,
	.cs[3].config       = 0x00000202,
	.cs[3].config_2     = 0x00000000,
#ifdef CONFIG_TQMLS1043A_DDR_2G
	.timing_cfg_3       = 0x010D1000,
	.timing_cfg_1       = 0xBCB44D66,
#else
	.timing_cfg_3       = 0x010C1000,
	.timing_cfg_1       = 0xBCB48D66,
#endif
	.timing_cfg_0       = 0xAA650008,
	.timing_cfg_2       = 0x0059119C,
	.ddr_sdram_cfg      = 0x650C0004,
	.ddr_sdram_cfg_2    = 0x00401010,
	.ddr_sdram_cfg_3    = 0x00000000,
	.ddr_sdram_mode     = 0x01010610,
	.ddr_sdram_mode_2   = 0x00100000,
	.ddr_sdram_mode_3   = 0x00000000,
	.ddr_sdram_mode_4   = 0x00000000,
	.ddr_sdram_mode_5   = 0x00000000,
	.ddr_sdram_mode_6   = 0x00000000,
	.ddr_sdram_mode_7   = 0x00000000,
	.ddr_sdram_mode_8   = 0x00000000,
	.ddr_sdram_mode_9   = 0x00000400,
	.ddr_sdram_mode_10  = 0x04800000,
	.ddr_sdram_mode_11  = 0x00000000,
	.ddr_sdram_mode_12  = 0x00000000,
	.ddr_sdram_mode_13  = 0x00000000,
	.ddr_sdram_mode_14  = 0x00000000,
	.ddr_sdram_mode_15  = 0x00000000,
	.ddr_sdram_mode_16  = 0x00000000,
	.ddr_sdram_interval = 0x0F3C076C,
	.ddr_data_init      = 0xDEADBEEF,
	.ddr_sdram_clk_cntl = 0x02400000,
	.ddr_init_addr      = 0x00000000,
	.ddr_init_ext_addr  = 0x00000000,
	.timing_cfg_4       = 0x00000001,
	.timing_cfg_5       = 0x08401400,
	.timing_cfg_6       = 0x00000000,
	.timing_cfg_7       = 0x15500000,
	.timing_cfg_8       = 0x03115800,
	.timing_cfg_9       = 0x00000000,
	.ddr_zq_cntl        = 0x8A090705,
	.ddr_wrlvl_cntl     = 0x86750607,
	.ddr_wrlvl_cntl_2   = 0x06070707,
	.ddr_wrlvl_cntl_3   = 0x07070707,
	.ddr_sr_cntr        = 0x00000000,
	.ddr_sdram_rcw_1    = 0x00000000,
	.ddr_sdram_rcw_2    = 0x00000000,
	.ddr_sdram_rcw_3    = 0x00000000,
	.ddr_cdr1           = 0x80080000,
	.ddr_cdr2           = 0x00000080,
	.dq_map_0           = 0x04104104,
	.dq_map_1           = 0x04104104,
	.dq_map_2           = 0x04104104,
	.dq_map_3           = 0x04104000,
	.debug[28]          = 0x00700046,

};

fixed_ddr_parm_t fixed_ddr_parm_0[] = {
	{1550, 1650, &ddr_cfg_regs_1600},
	{0, 0, NULL}
};

#endif
