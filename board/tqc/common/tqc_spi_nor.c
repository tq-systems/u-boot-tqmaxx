// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2022 TQ-Systems GmbH
 * Markus Niebel <Markus.Niebel@tq-group.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/uclass-internal.h>
#include <fdt_support.h>
#include <mtd_node.h>
#include <jffs2/load_kernel.h>

#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>

#include <spi.h>
#include <spi_flash.h>

#include "tqc_bb.h"

/* All this code must be called from ft_board_setup */
#if defined(CONFIG_OF_BOARD_SETUP) && !defined(CONFIG_SPL_BUILD)

static void tqc_ft_fixup_spi_mtdparts(void *blob,
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

void tqc_ft_spi_setup(void *blob, const char *path,
		      const struct node_info *nodes,
		      size_t node_count)
{
/* This code is specific for compiled in SPI FLASH support */
#if defined(CONFIG_SPI_FLASH)
	int off;
	bool enable_flash = false;

	unsigned int bus = CONFIG_SF_DEFAULT_BUS;
	unsigned int cs = CONFIG_SF_DEFAULT_CS;
	unsigned int speed = CONFIG_SF_DEFAULT_SPEED;
	unsigned int mode = CONFIG_SF_DEFAULT_MODE;

	/*
	 * TODO: to make this code portable until we land it in upstream
	 * U-Boot, we support DM and non DM case at least in theory.
	 * when upstreaming, non DM code should be deleted
	 */
	if (CONFIG_IS_ENABLED(DM_SPI_FLASH)) {
		struct udevice *new;
		int ret;

		ret = spi_flash_probe_bus_cs(bus, cs, speed, mode, &new);
		if (!ret) {
			tqc_ft_fixup_spi_mtdparts(blob, nodes, node_count);
			enable_flash = 1;
		}
	} else {
		struct spi_flash *new;

		new = spi_flash_probe(bus, cs, speed, mode);
		if (new) {
			tqc_ft_fixup_spi_mtdparts(blob, nodes, node_count);
			spi_flash_free(new);
			enable_flash = 1;
		}
	}

	off = fdt_path_offset(blob, path);
	if (off >= 0) {
		printf("%s %s\n", (enable_flash) ? "enable" : "disable",
		       path);
		fdt_set_node_status(blob, off, (enable_flash) ?
				    FDT_STATUS_OKAY : FDT_STATUS_DISABLED,
				    0);
	}
#endif
}

#endif
