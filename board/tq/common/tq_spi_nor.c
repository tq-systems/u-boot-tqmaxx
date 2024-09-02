// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <common.h>
#include <dm.h>
#include <fdt_support.h>
#include <jffs2/load_kernel.h>
#include <mtd_node.h>
#include <spi.h>
#include <spi_flash.h>

#include "tq_bb.h"

/* All this code must be called from ft_board_setup */
#if defined(CONFIG_OF_BOARD_SETUP) && !defined(CONFIG_SPL_BUILD)

static void tq_ft_fixup_spi_mtdparts(void *blob,
				     const struct node_info *nodes,
				     size_t node_count)
{
/* fdt_fixup_mtdparts is only present with CONFIG_FDT_FIXUP_PARTITIONS set */
#if defined(CONFIG_FDT_FIXUP_PARTITIONS)
	/*
	 * Update MTD partition nodes using info from mtdparts env var
	 * for [Q]SPI this needs the device probed.
	 */
	puts("   Updating MTD partitions...\n");
	fdt_fixup_mtdparts(blob, nodes, node_count);
#endif
}

void tq_ft_spi_setup(void *blob, const char *path,
		     const struct node_info *nodes,
		     size_t node_count)
{
	/* This code is specific for compiled in SPI FLASH support */
	if (CONFIG_IS_ENABLED(DM_SPI_FLASH)) {
		int off;
		bool disable_flash = true;
		unsigned int bus = CONFIG_SF_DEFAULT_BUS;
		unsigned int cs = CONFIG_SF_DEFAULT_CS;

		struct udevice *new;
		int ret;

		ret = spi_flash_probe_bus_cs(bus, cs, &new);
		if (!ret) {
			tq_ft_fixup_spi_mtdparts(blob, nodes, node_count);
			disable_flash = false;
		}

		if (disable_flash) {
			off = fdt_path_offset(blob, path);
			if (off >= 0)
				fdt_set_node_status(blob, off, FDT_STATUS_DISABLED);
		}
	}
}

#endif
