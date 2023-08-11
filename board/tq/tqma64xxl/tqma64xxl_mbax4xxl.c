// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Board specific initialization for TQ-Systems MBaX4XxL
 *
 * Copyright (c) 2020-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>, D-82229 Seefeld, Germany.
 * Author: Matthias Schiffer
 */

#include <common.h>
#include <env.h>
#include <env_internal.h>
#include <spl.h>
#include <mmc.h>

#include "tqma64xxl.h"

DECLARE_GLOBAL_DATA_PTR;

static void mbax4xxl_setup_env_bootdev(void)
{
	const char *boot_target = NULL;

	if (env_get("boot_targets"))
		return;

	switch (tqma64xxl_get_boot_device()) {
	case TQMA64XXL_BOOT_DEVICE_EMMC:
	case BOOT_DEVICE_MMC1:
		boot_target = "mmc0";
		break;
	case BOOT_DEVICE_MMC2:
		boot_target = "mmc1";
		break;
	case BOOT_DEVICE_SPI:
		boot_target = "sf0";
		break;
	case BOOT_DEVICE_USB:
		boot_target = "usb0";
		break;
	default:
		printf("Warning: unknown boot device\n");
	}

	if (boot_target)
		env_set_runtime("boot_targets", boot_target);
}

enum env_location env_get_location(enum env_operation op, int prio)
{
	if (prio > 0)
		return ENVL_UNKNOWN;

	switch (tqma64xxl_get_boot_device()) {
	case TQMA64XXL_BOOT_DEVICE_EMMC:
	case BOOT_DEVICE_MMC1:
	case BOOT_DEVICE_MMC2:
		return ENVL_MMC;
	case BOOT_DEVICE_SPI:
		return ENVL_SPI_FLASH;
	default:
		return ENVL_NOWHERE;
	}
}

#if CONFIG_IS_ENABLED(ENV_IS_IN_MMC)
int mmc_get_env_dev(void)
{
	switch (tqma64xxl_get_boot_device()) {
	case TQMA64XXL_BOOT_DEVICE_EMMC:
	case BOOT_DEVICE_MMC1:
		return 0;
	case BOOT_DEVICE_MMC2:
		return 1;
	default:
		return CONFIG_SYS_MMC_ENV_DEV;
	}
}

uint mmc_get_env_part(struct mmc *mmc)
{
	if (tqma64xxl_get_boot_device() == TQMA64XXL_BOOT_DEVICE_EMMC) {
		int bootpart = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);

		if (bootpart == 1 || bootpart == 2)
			return bootpart;
	}

	return 0;
}
#endif

int board_late_init(void)
{
	tqma64xxl_setup_sysinfo();
	mbax4xxl_setup_env_bootdev();

	return 0;
}

#if IS_ENABLED(CONFIG_OF_LIBFDT) && IS_ENABLED(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	return tqma64xxl_ft_board_setup(blob, bd);
}
#endif
