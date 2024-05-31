/* SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (c) 2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Paul Gerber
 */

#include <linux/types.h>

struct dram_info {
	struct dram_timing_info *table;
	phys_size_t size;
#if defined(CONFIG_IMX8M_DRAM_INLINE_ECC)
	void (*board_dram_ecc_scrub)(void);
#endif
};

#if defined(CONFIG_IMX8M_DRAM_INLINE_ECC)

#if defined(CONFIG_TQMA8MPXL_RAM_1024MB)
extern struct dram_timing_info dram_timing_1gb_ecc;
void board_dram_1gb_ecc_scrub(void);
#endif
#if defined(CONFIG_TQMA8MPXL_RAM_2048MB)
extern struct dram_timing_info dram_timing_2gb_ecc;
void board_dram_2gb_ecc_scrub(void);
#endif
#if defined(CONFIG_TQMA8MPXL_RAM_4096MB)
extern struct dram_timing_info dram_timing_4gb_ecc;
void board_dram_4gb_ecc_scrub(void);
#endif
#if defined(CONFIG_TQMA8MPXL_RAM_8192MB)
extern struct dram_timing_info dram_timing_8gb_ecc;
void board_dram_8gb_ecc_scrub(void);
#endif

#else /* !defined(CONFIG_IMX8M_DRAM_INLINE_ECC) */

#if defined(CONFIG_TQMA8MPXL_RAM_1024MB)
extern struct dram_timing_info dram_timing_1gb_no_ecc;
#endif
#if defined(CONFIG_TQMA8MPXL_RAM_2048MB)
extern struct dram_timing_info dram_timing_2gb_no_ecc;
#endif
#if defined(CONFIG_TQMA8MPXL_RAM_4096MB)
extern struct dram_timing_info dram_timing_4gb_no_ecc;
#endif
#if defined(CONFIG_TQMA8MPXL_RAM_8192MB)
extern struct dram_timing_info dram_timing_8gb_no_ecc;
#endif

#endif /* !defined(CONFIG_IMX8M_DRAM_INLINE_ECC) */

#if defined(CONFIG_IMX8M_DRAM_INLINE_ECC)
#define	DRAM_INFO_ENTRY(SIZE)						       \
	{								       \
		.table = &dram_timing_##SIZE##gb_ecc,			       \
		.size = SZ_1G * SIZE##ULL,				       \
		.board_dram_ecc_scrub = board_dram_##SIZE##gb_ecc_scrub	       \
	}
#else
#define	DRAM_INFO_ENTRY(SIZE)						       \
	{								       \
		.table = &dram_timing_##SIZE##gb_no_ecc,		       \
		.size = SZ_1G * SIZE##ULL,				       \
	}
#endif
