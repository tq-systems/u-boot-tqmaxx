/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 * Copyright (C) 2018-2019 TQ-Systems GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <asm/arch/sys_proto.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <mmc.h>
#include <stdbool.h>

static int check_mmc_autodetect(void)
{
	char *autodetect_str = env_get("mmcautodetect");

	if ((autodetect_str != NULL) &&
		(strcmp(autodetect_str, "yes") == 0)) {
		return 1;
	}

	return 0;
}

/* This should be defined for each board */
__weak int mmc_map_to_kernel_blk(int dev_no)
{
	return dev_no;
}

void board_late_mmc_env_init(void)
{
	u32 dev_no = mmc_get_env_dev();

	if (!check_mmc_autodetect())
		return;

	env_set_ulong("mmcdev", dev_no);
	env_set_ulong("mmcblkdev", mmc_map_to_kernel_blk(dev_no));
}
