/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 TQ-Systems GmbH
 */

#ifndef __DDR_H__
#define __DDR_H__

#include <common.h>
#include <fsl_ddr_sdram.h>

// DDR Controller configured register values
#define DDRmc1_CS0_BNDS_VAL         0x3F
#define DDRmc1_CS1_BNDS_VAL         0x00
#define DDRmc1_CS0_CONFIG_VAL       0x80010312
#define DDRmc1_CS1_CONFIG_VAL       0x00
#define DDRmc1_CS2_BNDS_VAL         0x00
#define DDRmc1_CS3_BNDS_VAL         0x00
#define DDRmc1_CS2_CONFIG_VAL       0x00
#define DDRmc1_CS3_CONFIG_VAL       0x00
#define DDRmc1_TIMING_CFG_0_VAL     0x90550018
#define DDRmc1_TIMING_CFG_1_VAL     0xBCB48C42
#define DDRmc1_TIMING_CFG_2_VAL     0x0048C11C
#define DDRmc1_TIMING_CFG_3_VAL     0x010C1000
#define DDRmc1_TIMING_CFG_4_VAL     0x00000002
#define DDRmc1_TIMING_CFG_5_VAL     0x00000000
#define DDRmc1_SDRAM_CFG_VAL        0xE50C0004
#define DDRmc1_SDRAM_CFG_2_VAL      0x00401110
#define DDRmc1_SDRAM_INTERVAL_VAL   0x0C300618
#define DDRmc1_SDRAM_CLK_CNTL_VAL   0x02000000
#define DDRmc1_WRLVL_CNTL_VAL       0x8675E605
#define DDRmc1_WRLVL_CNTL_2_VAL     0x05060600
#define DDRmc1_WRLVL_CNTL_3_VAL     0x00000006
#define DDRmc1_ZQ_CNTL_VAL          0x8A090705
#define DDRmc1_SDRAM_MODE_VAL       0x01010210
#define DDRmc1_SDRAM_MODE_2_VAL     0x00000000
#define DDRmc1_SDRAM_CFG_3_VAL          0x49000000
#define DDRmc1_TIMING_CFG_6_VAL         0x00000000
#define DDRmc1_TIMING_CFG_7_VAL         0x13300000
#define DDRmc1_TIMING_CFG_8_VAL         0x00006600
#define DDRmc1_DQ_MAP0_VAL                      0x5B65B630
#define DDRmc1_DQ_MAP1_VAL                      0xB16D8000
#define DDRmc1_DQ_MAP2_VAL                      0x00000000
#define DDRmc1_DQ_MAP3_VAL                      0x00100000
#define DDRmc1_SDRAM_MODE_3_VAL     0x00
#define DDRmc1_SDRAM_MODE_4_VAL     0x00
#define DDRmc1_SDRAM_MODE_5_VAL     0x00
#define DDRmc1_SDRAM_MODE_6_VAL     0x00
#define DDRmc1_SDRAM_MODE_7_VAL     0x00
#define DDRmc1_SDRAM_MODE_8_VAL     0x00
#define DDRmc1_SDRAM_MODE_9_VAL     0x00000500
#define DDRmc1_SDRAM_MODE_10_VAL     0x04000000
#define DDRmc1_SDRAM_MODE_11_VAL     0x00
#define DDRmc1_SDRAM_MODE_12_VAL     0x00
#define DDRmc1_SDRAM_MODE_13_VAL     0x00
#define DDRmc1_SDRAM_MODE_14_VAL     0x00
#define DDRmc1_SDRAM_MODE_15_VAL     0x00
#define DDRmc1_SDRAM_MODE_16_VAL     0x00
#define DDRmc1_DDRCDR_1_VAL         0x80040000
#define DDRmc1_DDRCDR_2_VAL         0x00000081
#define DDRmc1_INIT_ADDR_VAL        0x00000000
#define DDRmc1_INIT_EXT_ADDR_VAL    0x00000000
#define DDRmc1_SDRAM_RCW_1_VAL      0x00
#define DDRmc1_SDRAM_RCW_2_VAL      0x00
#define DDRmc1_SDRAM_RCW_3_VAL      0x00
#define DDRmc1_SDRAM_RCW_4_VAL      0x00
#define DDRmc1_SDRAM_RCW_5_VAL      0x00
#define DDRmc1_SDRAM_RCW_6_VAL      0x00
#define DDRmc1_ERR_DISABLE_VAL      0x00
#define DDRmc1_ERR_INT_EN_VAL           0x00
#define DDRmc1_ERR_SBE_VAL              0x00
#define DDRmc1_DATA_INIT_VAL        0xDEADBEEF
#define DDRmc1_SDRAM_MD_CNTL_VAL    0x00000000

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
	{1,  2100, 0, 8,     9, 0x09080806, 0x07060606,},
#endif
	{}
};

static const struct board_specific_parameters *udimms[] = {
	udimm0,
};

#ifndef CONFIG_SYS_DDR_RAW_TIMING
fsl_ddr_cfg_regs_t ddr_cfg_regs_1600 = {
	.cs[0].bnds = DDRmc1_CS0_BNDS_VAL,
	.cs[1].bnds = DDRmc1_CS1_BNDS_VAL,
	.cs[2].bnds = DDRmc1_CS2_BNDS_VAL,
	.cs[3].bnds = DDRmc1_CS3_BNDS_VAL,
	.cs[0].config = DDRmc1_CS0_CONFIG_VAL,
	.cs[0].config_2 = 0,
	.cs[1].config = DDRmc1_CS1_CONFIG_VAL,
	.cs[1].config_2 = 0,
	.cs[2].config = DDRmc1_CS2_CONFIG_VAL,
	.cs[3].config = DDRmc1_CS3_CONFIG_VAL,
	.timing_cfg_3 = DDRmc1_TIMING_CFG_3_VAL,
	.timing_cfg_0 = DDRmc1_TIMING_CFG_0_VAL,
	.timing_cfg_1 = DDRmc1_TIMING_CFG_1_VAL,
	.timing_cfg_2 = DDRmc1_TIMING_CFG_2_VAL,
	.ddr_sdram_cfg = DDRmc1_SDRAM_CFG_VAL,
	.ddr_sdram_cfg_2 = DDRmc1_SDRAM_CFG_2_VAL,
	.ddr_sdram_cfg_3 = DDRmc1_SDRAM_CFG_3_VAL,
	.ddr_sdram_mode = DDRmc1_SDRAM_MODE_VAL,
	.ddr_sdram_mode_2 = DDRmc1_SDRAM_MODE_2_VAL,
	.ddr_sdram_mode_3 = DDRmc1_SDRAM_MODE_3_VAL,
	.ddr_sdram_mode_4 = DDRmc1_SDRAM_MODE_4_VAL,
	.ddr_sdram_mode_5 = DDRmc1_SDRAM_MODE_5_VAL,
	.ddr_sdram_mode_6 = DDRmc1_SDRAM_MODE_6_VAL,
	.ddr_sdram_mode_7 = DDRmc1_SDRAM_MODE_7_VAL,
	.ddr_sdram_mode_8 = DDRmc1_SDRAM_MODE_8_VAL,
	.ddr_sdram_mode_9 = DDRmc1_SDRAM_MODE_9_VAL,
	.ddr_sdram_mode_10 = DDRmc1_SDRAM_MODE_10_VAL,
	.ddr_sdram_mode_11 = DDRmc1_SDRAM_MODE_11_VAL,
	.ddr_sdram_mode_12 = DDRmc1_SDRAM_MODE_12_VAL,
	.ddr_sdram_mode_13 = DDRmc1_SDRAM_MODE_13_VAL,
	.ddr_sdram_mode_14 = DDRmc1_SDRAM_MODE_14_VAL,
	.ddr_sdram_mode_15 = DDRmc1_SDRAM_MODE_15_VAL,
	.ddr_sdram_mode_16 = DDRmc1_SDRAM_MODE_16_VAL,
	.ddr_sdram_interval = DDRmc1_SDRAM_INTERVAL_VAL,
	.ddr_data_init = DDRmc1_DATA_INIT_VAL,
	.ddr_sdram_clk_cntl = DDRmc1_SDRAM_CLK_CNTL_VAL,
	.ddr_init_addr = DDRmc1_INIT_ADDR_VAL,
	.ddr_init_ext_addr = DDRmc1_INIT_EXT_ADDR_VAL,
	.timing_cfg_4 = DDRmc1_TIMING_CFG_4_VAL,
	.timing_cfg_5 = DDRmc1_TIMING_CFG_5_VAL,
	.timing_cfg_6 = DDRmc1_TIMING_CFG_6_VAL,
	.timing_cfg_7 = DDRmc1_TIMING_CFG_7_VAL,
	.timing_cfg_8 = DDRmc1_TIMING_CFG_8_VAL,
	.timing_cfg_9 = 0x0,
	.ddr_zq_cntl = DDRmc1_ZQ_CNTL_VAL,
	.ddr_wrlvl_cntl =  DDRmc1_WRLVL_CNTL_VAL,
	.ddr_wrlvl_cntl_2 = DDRmc1_WRLVL_CNTL_2_VAL,
	.ddr_wrlvl_cntl_3 = DDRmc1_WRLVL_CNTL_3_VAL,
	.ddr_sr_cntr = 0,
	.ddr_sdram_rcw_1 = DDRmc1_SDRAM_RCW_1_VAL,
	.ddr_sdram_rcw_2 = DDRmc1_SDRAM_RCW_2_VAL,
	.ddr_cdr1 = DDRmc1_DDRCDR_1_VAL,
	.ddr_cdr2 = DDRmc1_DDRCDR_2_VAL,
	.dq_map_0 = DDRmc1_DQ_MAP0_VAL,
	.dq_map_1 = DDRmc1_DQ_MAP1_VAL,
	.dq_map_2 = DDRmc1_DQ_MAP2_VAL,
	.dq_map_3 = DDRmc1_DQ_MAP3_VAL,
	.debug[28] = 0x0070002a,
};

fixed_ddr_parm_t fixed_ddr_parm_0[] = {
	{1550, 1650, &ddr_cfg_regs_1600},
	{0, 0, NULL}
};

#endif
#endif
