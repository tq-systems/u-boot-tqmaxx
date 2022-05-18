/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2013, 2014, 2016 - 2022 TQ-Systems GmbH
 * Author: Markus Niebel <markus.niebel@tq-group.com>
 */

#ifndef __TQ_BB__
#define __TQ_BB__

#include <mtd_node.h>

/*
 * Device Tree Support
 */
#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)

/**
 * modify SPI NOR specific data in the FDT before booting the OS.
 *
 * @param blob		FDT blob to update
 * @param path		path to spi controller for the SPI NOR device
 * @param nodes		list of compatibles to scan
 * @param node_count	path to spi controller for the SPI NOR device
 *
 * Test if optional SPI NOR is present and enable / disable it in device tree
 * If present, use U-Boot standard env variables to add partition info to the
 * SPI NOR device node. This is needed by kernel to use UBIFS in MTD partition
 * as rootfs.
 *
 * If compiled without SPI NOR support no changes are applied to device tree
 */
void tq_ft_spi_setup(void *blob, const char *path,
		     const struct node_info *nodes,
		     size_t node_count);
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */

#endif
