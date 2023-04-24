// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel, Matthias Schiffer
 */

#include <common.h>
#include <fdt_support.h>
#include <spi.h>
#include <spi_flash.h>

#include "tq_spi_nor.h"

/* All this code must be called from ft_board_setup */
#if defined(CONFIG_OF_BOARD_SETUP)

void tq_ft_spi_setup(void *blob, const char *path,
		     const struct node_info *nodes,
		     size_t node_count)
{
#if CONFIG_IS_ENABLED(DM_SPI_FLASH)
	bool enable_flash = false;
	struct udevice *dev;
	int off, ret;

	ret = spi_flash_probe_bus_cs(CONFIG_SF_DEFAULT_BUS,
				     CONFIG_SF_DEFAULT_CS,
				     CONFIG_SF_DEFAULT_SPEED,
				     CONFIG_SF_DEFAULT_MODE, &dev);
	if (!ret) {
		fdt_fixup_mtdparts(blob, nodes, node_count);
		enable_flash = true;
	}

	off = fdt_path_offset(blob, path);
	if (off >= 0) {
		printf("%s %s\n", enable_flash ? "enable" : "disable",
		       path);
		fdt_set_node_status(blob, off, enable_flash ?
				    FDT_STATUS_OKAY : FDT_STATUS_DISABLED, 0);
	}
#endif
}

#endif
