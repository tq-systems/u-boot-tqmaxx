/* SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (c) 2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#ifndef __TQMA91_93_DRAM__
#define __TQMA91_93_DRAM__

#include <asm/arch/ddr.h>
#include <linux/types.h>

/* DDR timing / configuration generated with NXP DDR Tool */
#if IS_ENABLED(CONFIG_IMX93)
extern struct dram_timing_info tqma93xxca_dram_timing_1gb;
extern struct dram_timing_info tqma93xxla_dram_timing_1gb;
extern struct dram_timing_info tqma93xxla_dram_timing_1gb5;
extern struct dram_timing_info tqma93xxca_dram_timing_2gb;
extern struct dram_timing_info tqma93xxla_dram_timing_2gb;
#elif IS_ENABLED(CONFIG_IMX91)
extern struct dram_timing_info tqma91xxca_dram_timing_1gb;
extern struct dram_timing_info tqma91xxla_dram_timing_1gb;
#endif

enum tqma93xxxa_ram_size {
	TQMA93XXXA_RAM_SIZE_1G = 1,
	TQMA93XXXA_RAM_SIZE_1G5,
	TQMA93XXXA_RAM_SIZE_2G,
};

struct dram_info {
	struct dram_timing_info	*table;  /* from NXP RPA */
	phys_size_t		size;    /* size of RAM */
	char			variant; /* char to help user query */
};

extern struct dram_cfg_param ddr_ddrphy_trained_csr[];
extern const u32 ddr_ddrphy_trained_csr_num;

#endif
