// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Board specific initialization for TQ-Systems MBaX4XxL
 *
 * Copyright (C) 2017-2018 Texas Instruments Incorporated - http://www.ti.com/
 * Copyright (c) 2020-2022 TQ-Systems GmbH <license@tq-group.com>, D-82229 Seefeld, Germany.
 *
 */

#include <common.h>
#include <dm.h>
#include <env.h>
#include <env_internal.h>
#include <spl.h>
#include <mmc.h>

#include "tqma64xxl.h"

DECLARE_GLOBAL_DATA_PTR;

static void mbax4xxl_setup_env_bootdev(void)
{
	switch (tqma64xxl_get_boot_device()) {
	case BOOT_DEVICE_MMC1:
		if (!env_get("boot"))
			env_set_runtime("boot", "mmc");
		if (!env_get("mmcdev"))
			env_set_runtime("mmcdev", "0");
		break;
	case BOOT_DEVICE_MMC2:
		if (!env_get("boot"))
			env_set_runtime("boot", "mmc");
		if (!env_get("mmcdev"))
			env_set_runtime("mmcdev", "1");
		break;
	case BOOT_DEVICE_SPI:
		if (!env_get("boot"))
			env_set_runtime("boot", "ubi");
		break;
	default:
		printf("Warning: unknown boot device\n");
	}
}

enum env_location env_get_location(enum env_operation op, int prio)
{
	if (prio > 0)
		return ENVL_UNKNOWN;

	switch (tqma64xxl_get_boot_device()) {
	case BOOT_DEVICE_MMC1:
	case BOOT_DEVICE_MMC2:
		return ENVL_MMC;
	case BOOT_DEVICE_SPI:
		return ENVL_SPI_FLASH;
	default:
		return ENVL_NOWHERE;
	}
}

#ifdef CONFIG_ENV_IS_IN_MMC
int mmc_get_env_dev(void)
{
	switch (tqma64xxl_get_boot_device()) {
	case BOOT_DEVICE_MMC1:
		return 0;
	case BOOT_DEVICE_MMC2:
		return 1;
	default:
		return CONFIG_SYS_MMC_ENV_DEV;
	}
}
#endif

int board_late_init(void)
{
	tqma64xxl_parse_eeprom();
	mbax4xxl_setup_env_bootdev();

	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	return tqma64xxl_ft_board_setup(blob, bd);
}
#endif
