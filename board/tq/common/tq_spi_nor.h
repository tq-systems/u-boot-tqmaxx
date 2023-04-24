/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2013-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel, Matthias Schiffer
 */

#ifndef __TQ_SPI_NOR_H__
#define __TQ_SPI_NOR_H__

#include <mtd_node.h>

#if defined(CONFIG_OF_BOARD_SETUP)

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

#endif

#endif /* __TQ_SPI_NOR_H__ */
