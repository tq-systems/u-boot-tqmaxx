/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 TQ-Systems GmbH
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
	{1,  2100, 0, 8,     9, 0x0A080807, 0x0706060A,},
#endif
	{}
};

static const struct board_specific_parameters *udimms[] = {
	udimm0,
};
#endif

#ifdef CONFIG_TQMLS1046A_DDR_8G
fsl_ddr_cfg_regs_t ddr_cfg_regs_2000 = {
	.cs[0].bnds         = 0x000001FF,
	.cs[0].config       = 0x80010512,
	.cs[0].config_2     = 0x00000000,
	.cs[1].bnds         = 0x00000000,
	.cs[1].config       = 0x00000000,
	.cs[1].config_2     = 0x00000000,
	.cs[2].bnds         = 0x00000000,
	.cs[2].config       = 0x00000000,
	.cs[2].config_2     = 0x00000000,
	.cs[3].bnds         = 0x00000000,
	.cs[3].config       = 0x00000000,
	.cs[3].config_2     = 0x00000000,
	.timing_cfg_3       = 0x12551100,
	.timing_cfg_0       = 0xA0660008,
	.timing_cfg_1       = 0x060E6278,
	.timing_cfg_2       = 0x0058625E,
	.ddr_sdram_cfg      = 0x65200008,
	.ddr_sdram_cfg_2    = 0x00401070,
	.ddr_sdram_cfg_3    = 0x40000000,
	.ddr_sdram_mode     = 0x05030635,
	.ddr_sdram_mode_2   = 0x00100000,
	.ddr_sdram_mode_3   = 0x00000000,
	.ddr_sdram_mode_4   = 0x00000000,
	.ddr_sdram_mode_5   = 0x00000000,
	.ddr_sdram_mode_6   = 0x00000000,
	.ddr_sdram_mode_7   = 0x00000000,
	.ddr_sdram_mode_8   = 0x00000000,
	.ddr_sdram_mode_9   = 0x00000701,
	.ddr_sdram_mode_10  = 0x08800000,
	.ddr_sdram_mode_11  = 0x00000000,
	.ddr_sdram_mode_12  = 0x00000000,
	.ddr_sdram_mode_13  = 0x00000000,
	.ddr_sdram_mode_14  = 0x00000000,
	.ddr_sdram_mode_15  = 0x00000000,
	.ddr_sdram_mode_16  = 0x00000000,
	.ddr_sdram_interval = 0x0F3C079E,
	.ddr_data_init      = 0xDEADBEEF,
	.ddr_sdram_clk_cntl = 0x02400000,
	.ddr_init_addr      = 0x00000000,
	.ddr_init_ext_addr  = 0x00000000,
	.timing_cfg_4       = 0x00224502,
	.timing_cfg_5       = 0x06401400,
	.timing_cfg_6       = 0x00000000,
	.timing_cfg_7       = 0x25540000,
	.timing_cfg_8       = 0x06445A00,
	.timing_cfg_9       = 0x00000000,
	.ddr_zq_cntl        = 0x8A090705,
	.ddr_wrlvl_cntl     = 0x86750608,
	.ddr_wrlvl_cntl_2   = 0x07070605,
	.ddr_wrlvl_cntl_3   = 0x05040407,
	.ddr_sr_cntr        = 0x00000000,
	.ddr_sdram_rcw_1    = 0x00000000,
	.ddr_sdram_rcw_2    = 0x00000000,
	.ddr_sdram_rcw_3    = 0x00000000,
	.ddr_cdr1           = 0x80080000,
	.ddr_cdr2           = 0x00000080,
	.dq_map_0           = 0x06106104,
	.dq_map_1           = 0x84184184,
	.dq_map_2           = 0x06106104,
	.dq_map_3           = 0x84184000,
	.debug[28]          = 0x00700046,
};
#else
fsl_ddr_cfg_regs_t ddr_cfg_regs_2000 = {
	.cs[0].bnds         = 0x0000007F,
	.cs[0].config       = 0x80010312,
	.cs[0].config_2     = 0x00000000,
	.cs[1].bnds         = 0x00000000,
	.cs[1].config       = 0x00000000,
	.cs[1].config_2     = 0x00000000,
	.cs[2].bnds         = 0x00000000,
	.cs[2].config       = 0x00000000,
	.cs[2].config_2     = 0x00000000,
	.cs[3].bnds         = 0x00000000,
	.cs[3].config       = 0x00000000,
	.cs[3].config_2     = 0x00000000,
	.timing_cfg_3       = 0x020F1100,
	.timing_cfg_0       = 0xF7660008,
	.timing_cfg_1       = 0xF1FC4178,
	.timing_cfg_2       = 0x00590160,
	.ddr_sdram_cfg      = 0x65000008,
	.ddr_sdram_cfg_2    = 0x00401150,
	.ddr_sdram_cfg_3    = 0x40000000,
	.ddr_sdram_mode     = 0x01030631,
	.ddr_sdram_mode_2   = 0x00100200,
	.ddr_sdram_mode_3   = 0x00000000,
	.ddr_sdram_mode_4   = 0x00000000,
	.ddr_sdram_mode_5   = 0x00000000,
	.ddr_sdram_mode_6   = 0x00000000,
	.ddr_sdram_mode_7   = 0x00000000,
	.ddr_sdram_mode_8   = 0x00000000,
	.ddr_sdram_mode_9   = 0x00000500,
	.ddr_sdram_mode_10  = 0x08800000,
	.ddr_sdram_mode_11  = 0x00000000,
	.ddr_sdram_mode_12  = 0x00000000,
	.ddr_sdram_mode_13  = 0x00000000,
	.ddr_sdram_mode_14  = 0x00000000,
	.ddr_sdram_mode_15  = 0x00000000,
	.ddr_sdram_mode_16  = 0x00000000,
	.ddr_sdram_interval = 0x0F3C079E,
	.ddr_data_init      = 0xDEADBEEF,
	.ddr_sdram_clk_cntl = 0x03000000,
	.ddr_init_addr      = 0x00000000,
	.ddr_init_ext_addr  = 0x00000000,
	.timing_cfg_4       = 0x00220002,
	.timing_cfg_5       = 0x00000000,
	.timing_cfg_6       = 0x00000000,
	.timing_cfg_7       = 0x25500000,
	.timing_cfg_8       = 0x05447A00,
	.timing_cfg_9       = 0x00000000,
	.ddr_zq_cntl        = 0x8A090705,
	.ddr_wrlvl_cntl     = 0x8605070A,
	.ddr_wrlvl_cntl_2   = 0x0A080807,
	.ddr_wrlvl_cntl_3   = 0x0706060A,
	.ddr_sr_cntr        = 0x00000000,
	.ddr_sdram_rcw_1    = 0x00000000,
	.ddr_sdram_rcw_2    = 0x00000000,
	.ddr_sdram_rcw_3    = 0x00000000,
	.ddr_cdr1           = 0x80080000,
	.ddr_cdr2           = 0x000000C0,
	.dq_map_0           = 0x00000000,
	.dq_map_1           = 0x00000000,
	.dq_map_2           = 0x00000000,
	.dq_map_3           = 0x00000000,
	.debug[28]          = 0x00700046,
};
#endif

fixed_ddr_parm_t fixed_ddr_parm_0[] = {
	{1950, 2050, &ddr_cfg_regs_2000},
	{0, 0, NULL}
};

#endif
