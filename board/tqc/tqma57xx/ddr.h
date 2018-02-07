/*
 * Copyright (C) 2017 TQ Systems (ported AM57xx IDK to TQMa57xx)
 *
 * Author: Stefan Lange <s.lange@gateware.de>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <asm/emif.h>

/*
 * DDR timings for TQMa57xx module variant with AM572x and 2GiB DRAM
 * DDR clock: 532 MHz
 */

static const struct emif_regs tqma572x_emif1_ddr3_532mhz_emif_regs = {
	.sdram_config_init              = 0x61851b32,
	.sdram_config                   = 0x61851b32,
	.sdram_config2                  = 0x08000000,
	.ref_ctrl                       = 0x000040F1,
	.ref_ctrl_final                 = 0x00001035,
	.sdram_tim1                     = 0xcccf36ab,
	.sdram_tim2                     = 0x308f7fda,
	.sdram_tim3                     = 0x409f88a8,
	.read_idle_ctrl                 = 0x00050000,
	.zq_config                      = 0x5007190b,
	.temp_alert_config              = 0x00000000,
	.emif_ddr_phy_ctlr_1_init       = 0x0024400b,
	.emif_ddr_phy_ctlr_1            = 0x0e24400b,
	.emif_ddr_ext_phy_ctrl_1        = 0x10040100,
	.emif_ddr_ext_phy_ctrl_2        = 0x00910091,
	.emif_ddr_ext_phy_ctrl_3        = 0x00950095,
	.emif_ddr_ext_phy_ctrl_4        = 0x009b009b,
	.emif_ddr_ext_phy_ctrl_5        = 0x009e009e,
	.emif_rd_wr_lvl_rmp_win         = 0x00000000,
	.emif_rd_wr_lvl_rmp_ctl         = 0x80000000,
	.emif_rd_wr_lvl_ctl             = 0x00000000,
	.emif_rd_wr_exec_thresh         = 0x00000305
};

static const u32 tqma572x_emif1_ddr3_ext_phy_ctrl_const_regs[] = {
	0x04040100,	// EMIF1_EXT_PHY_CTRL_1
	0x006B00A2,	// EMIF1_EXT_PHY_CTRL_2
	0x006B00A2,	// EMIF1_EXT_PHY_CTRL_3
	0x006B00A6,	// EMIF1_EXT_PHY_CTRL_4
	0x006B00A6,	// EMIF1_EXT_PHY_CTRL_5
	0x006B00AA,	// EMIF1_EXT_PHY_CTRL_6
	0x00320032,	// EMIF1_EXT_PHY_CTRL_7
	0x00320032,	// EMIF1_EXT_PHY_CTRL_8
	0x00320032,	// EMIF1_EXT_PHY_CTRL_9
	0x00320032,	// EMIF1_EXT_PHY_CTRL_10
	0x00320032,	// EMIF1_EXT_PHY_CTRL_11
	0x0060006F,	// EMIF1_EXT_PHY_CTRL_12
	0x00600070,	// EMIF1_EXT_PHY_CTRL_13
	0x00600079,	// EMIF1_EXT_PHY_CTRL_14
	0x00600079,	// EMIF1_EXT_PHY_CTRL_15
	0x00600083,	// EMIF1_EXT_PHY_CTRL_16
	0x0040004F,	// EMIF1_EXT_PHY_CTRL_17
	0x00400050,	// EMIF1_EXT_PHY_CTRL_18
	0x00400059,	// EMIF1_EXT_PHY_CTRL_19
	0x00400059,	// EMIF1_EXT_PHY_CTRL_20
	0x00400063,	// EMIF1_EXT_PHY_CTRL_21
	0x00800080,	// EMIF1_EXT_PHY_CTRL_22
	0x00800080,	// EMIF1_EXT_PHY_CTRL_23
	0x40010080,	// EMIF1_EXT_PHY_CTRL_24
	0x08102040,	// EMIF1_EXT_PHY_CTRL_25
	0x005B0092,	// EMIF1_EXT_PHY_CTRL_26
	0x005B0092,	// EMIF1_EXT_PHY_CTRL_27
	0x005B0096,	// EMIF1_EXT_PHY_CTRL_28
	0x005B0096,	// EMIF1_EXT_PHY_CTRL_29
	0x005B009A,	// EMIF1_EXT_PHY_CTRL_30
	0x0030003F,	// EMIF1_EXT_PHY_CTRL_31
	0x00300040,	// EMIF1_EXT_PHY_CTRL_32
	0x00300049,	// EMIF1_EXT_PHY_CTRL_33
	0x00300049,	// EMIF1_EXT_PHY_CTRL_34
	0x00300053,	// EMIF1_EXT_PHY_CTRL_35
	0x00000077	// EMIF1_EXT_PHY_CTRL_36
};

static const struct emif_regs tqma572x_emif2_ddr3_532mhz_emif_regs = {
	.sdram_config_init              = 0x61851b32,
	.sdram_config                   = 0x61851b32,
	.sdram_config2                  = 0x08000000,
	.ref_ctrl                       = 0x000040F1,
	.ref_ctrl_final                 = 0x00001035,
	.sdram_tim1                     = 0xcccf36b3,
	.sdram_tim2                     = 0x308f7fda,
	.sdram_tim3                     = 0x407f88a8,
	.read_idle_ctrl                 = 0x00050000,
	.zq_config                      = 0x5007190b,
	.temp_alert_config              = 0x00000000,
	.emif_ddr_phy_ctlr_1_init       = 0x0024400b,
	.emif_ddr_phy_ctlr_1            = 0x0e24400b,
	.emif_ddr_ext_phy_ctrl_1        = 0x10040100,
	.emif_ddr_ext_phy_ctrl_2        = 0x00910091,
	.emif_ddr_ext_phy_ctrl_3        = 0x00950095,
	.emif_ddr_ext_phy_ctrl_4        = 0x009b009b,
	.emif_ddr_ext_phy_ctrl_5        = 0x009e009e,
	.emif_rd_wr_lvl_rmp_win         = 0x00000000,
	.emif_rd_wr_lvl_rmp_ctl         = 0x80000000,
	.emif_rd_wr_lvl_ctl             = 0x00000000,
	.emif_rd_wr_exec_thresh         = 0x00000305
};

static const u32 tqma572x_emif2_ddr3_ext_phy_ctrl_const_regs[] = {
	0x04040100,	// EMIF2_EXT_PHY_CTRL_1
	0x006B009B,	// EMIF2_EXT_PHY_CTRL_2
	0x006B009A,	// EMIF2_EXT_PHY_CTRL_3
	0x006B0091,	// EMIF2_EXT_PHY_CTRL_4
	0x006B0090,	// EMIF2_EXT_PHY_CTRL_5
	0x006B006B,	// EMIF2_EXT_PHY_CTRL_6
	0x00320032,	// EMIF2_EXT_PHY_CTRL_7
	0x00320032,	// EMIF2_EXT_PHY_CTRL_8
	0x00320032,	// EMIF2_EXT_PHY_CTRL_9
	0x00320032,	// EMIF2_EXT_PHY_CTRL_10
	0x00320032,	// EMIF2_EXT_PHY_CTRL_11
	0x0060006B,	// EMIF2_EXT_PHY_CTRL_12
	0x0060006A,	// EMIF2_EXT_PHY_CTRL_13
	0x00600066,	// EMIF2_EXT_PHY_CTRL_14
	0x00600066,	// EMIF2_EXT_PHY_CTRL_15
	0x00600060,	// EMIF2_EXT_PHY_CTRL_16
	0x0040004B,	// EMIF2_EXT_PHY_CTRL_17
	0x0040004A,	// EMIF2_EXT_PHY_CTRL_18
	0x00400046,	// EMIF2_EXT_PHY_CTRL_19
	0x00400046,	// EMIF2_EXT_PHY_CTRL_20
	0x00400040,	// EMIF2_EXT_PHY_CTRL_21
	0x00800080,	// EMIF2_EXT_PHY_CTRL_22
	0x00800080,	// EMIF2_EXT_PHY_CTRL_23
	0x40010080,	// EMIF2_EXT_PHY_CTRL_24
	0x08102040,	// EMIF2_EXT_PHY_CTRL_25
	0x005B008B,	// EMIF2_EXT_PHY_CTRL_26
	0x005B008A,	// EMIF2_EXT_PHY_CTRL_27
	0x005B0081,	// EMIF2_EXT_PHY_CTRL_28
	0x005B0080,	// EMIF2_EXT_PHY_CTRL_29
	0x005B005B,	// EMIF2_EXT_PHY_CTRL_30
	0x0030003B,	// EMIF2_EXT_PHY_CTRL_31
	0x0030003A,	// EMIF2_EXT_PHY_CTRL_32
	0x00300036,	// EMIF2_EXT_PHY_CTRL_33
	0x00300036,	// EMIF2_EXT_PHY_CTRL_34
	0x00300030,	// EMIF2_EXT_PHY_CTRL_35
	0x00000077	// EMIF2_EXT_PHY_CTRL_36
};

