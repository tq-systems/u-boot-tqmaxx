/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2013-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#ifndef __TQ_BB_H
#define __TQ_BB_H

struct mmc;
struct bd_info;
struct node_info;

int tq_bb_board_mmc_getwp(struct mmc *mmc);
int tq_bb_board_mmc_getcd(struct mmc *mmc);
int tq_bb_board_mmc_init(struct bd_info *bis);

int tq_bb_board_early_init_f(void);
int tq_bb_board_init(void);
int tq_bb_board_late_init(void);
int tq_bb_checkboard(void);
void tq_bb_board_quiesce_devices(void);

const char *tq_bb_get_boardname(void);

#if IS_ENABLED(CONFIG_SPL_BUILD)
void tq_bb_board_init_f(ulong dummy);
void tq_bb_spl_board_init(void);
#endif

/*
 * Device Tree Support
 */
#if IS_ENABLED(CONFIG_OF_BOARD_SETUP) && IS_ENABLED(CONFIG_OF_LIBFDT)

struct node_info;

int tq_bb_ft_board_setup(void *blob, struct bd_info *bis);
/**
 * modify SPI NOR specific data in the FDT before booting the OS.
 *
 * @param blob		FDT blob to update
 * @param path		path to spi controller for the SPI NOR device
 * @param nodes		list of compatibles to scan
 * @param node_count	path to spi controller for the SPI NOR device
 *
 * Test if optional SPI NOR is present and disable it in device tree if not found.
 * If present, use U-Boot standard env variables to add partition info to the
 * SPI NOR device node. This is needed by kernel to use UBIFS in MTD partition
 * as rootfs.
 *
 * If compiled without SPI NOR support no changes are applied to device tree
 */
void tq_ft_spi_setup(void *blob, const char *path,
		     const struct node_info *nodes,
		     size_t node_count);
#endif /* IS_ENABLED(CONFIG_OF_BOARD_SETUP) && IS_ENABLED(CONFIG_OF_LIBFDT) */

#endif /* __TQ_BB_H */
