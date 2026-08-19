// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <env.h>
#include <fdt_support.h>
#include <init.h>
#include <mtd_node.h>
#include <asm/global_data.h>
#include <asm/arch/sys_proto.h>
#include <jffs2/load_kernel.h>
#include <linux/errno.h>

#include "../common/tq_bb.h"

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	return tq_bb_board_early_init_f();
}

#if !IS_ENABLED(CONFIG_XPL_BUILD)

int board_init(void)
{
	return tq_bb_board_init();
}

int board_late_init(void)
{
	if (IS_ENABLED(CONFIG_AHAB_BOOT))
		env_set("sec_boot", "yes");
	else
		env_set("sec_boot", "no");

	return tq_bb_board_late_init();
}

#if IS_ENABLED(CONFIG_OF_BOARD_SETUP)

int ft_board_setup(void *blob, struct bd_info *bd)
{
	if (IS_ENABLED(CONFIG_FDT_FIXUP_PARTITIONS)) {
		const char * const path = "/soc/bus@42000000/spi@425e0000";
		const struct node_info nodes[] = {
			{ "jedec,spi-nor",	MTD_DEV_TYPE_NOR, },
		};

		/*
		 * Update MTD partition nodes using info from
		 * mtdparts env var
		 * for [Q]SPI this needs the device probed.
		 */
		tq_ft_spi_setup(blob, path, nodes,
				ARRAY_SIZE(nodes));
	}

	return tq_bb_ft_board_setup(blob, bd);
}
#endif

#if IS_ENABLED(CONFIG_OF_BOARD_FIXUP)
extern int board_fix_fdt_fuse(void *fdt);

int board_fix_fdt(void *fdt)
{
	/* Remove nodes based on fuses. */
	board_fix_fdt_fuse(fdt);

	return 0;
}
#endif

void board_quiesce_devices(void)
{
	tq_bb_board_quiesce_devices();
}

/**
 * Translate detected CPU variant into TQ-Systems SOM variant short name
 *
 * return: string consisting of the name or string indicating unknown variant
 */
const char *tq_get_boardname(void)
{
	static char *bname = "TQMa95xxLA";
	u32 cpurev = get_cpu_rev();
	const char *name = get_imx_type((cpurev & 0x1FF000) >> 12);

	if (name)
		sprintf(bname, "TQMa%sLA", name);

	return bname;
}

int checkboard(void)
{
	tq_print_bootinfo();
	printf("Board: %s on a %s\n", tq_get_boardname(),
	       tq_bb_get_boardname());

	return tq_bb_checkboard();
}

#endif /* !IS_ENABLED(CONFIG_XPL_BUILD) */
