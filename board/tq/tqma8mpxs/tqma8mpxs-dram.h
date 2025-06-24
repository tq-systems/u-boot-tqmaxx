/* SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (c) 2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Paul Gerber
 */

#include <asm/arch/ddr.h>
#include <linux/types.h>

struct dram_info {
	struct dram_timing_info *table;
	phys_size_t size;
	size_t type;
	void (*board_dram_ecc_scrub)(void);
};

extern struct dram_cfg_param ddr_ddrphy_trained_csr[];
extern const u32 ddr_ddrphy_trained_csr_num;

#if defined(CONFIG_IMX8M_DRAM_INLINE_ECC)

#define DECLARE_DRAM_DATA(SIZE, TYPE) \
	extern struct dram_timing_info dram_timing_##SIZE##gb_type##TYPE##_ecc; \
	void board_dram_##SIZE##gb_type##TYPE##_ecc_scrub(void)

#else /* !defined(CONFIG_IMX8M_DRAM_INLINE_ECC) */

#define DECLARE_DRAM_DATA(SIZE, TYPE) \
	extern struct dram_timing_info dram_timing_##SIZE##gb_type##TYPE##_no_ecc

#endif /* !defined(CONFIG_IMX8M_DRAM_INLINE_ECC) */

#if defined(CONFIG_IMX8M_DRAM_INLINE_ECC)
#define	DRAM_INFO_ENTRY(SIZE, TYPE)							\
	{										\
		.table = &dram_timing_##SIZE##gb_type##TYPE##_ecc,			\
		.size = SZ_1G * SIZE##ULL,						\
		.type = TYPE,								\
		.board_dram_ecc_scrub = board_dram_##SIZE##gb_type##TYPE##_ecc_scrub	\
	}
#else
#define	DRAM_INFO_ENTRY(SIZE, TYPE)						\
	{									\
		.table = &dram_timing_##SIZE##gb_type##TYPE##_no_ecc,		\
		.size = SZ_1G * SIZE##ULL,					\
		.type = TYPE,							\
	}
#endif
