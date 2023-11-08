// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2020-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <common.h>
#include <env_internal.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>

enum env_location env_get_location(enum env_operation op, int prio)
{
	enum boot_device dev = get_boot_device();
	enum env_location env_loc = ENVL_UNKNOWN;

	if (prio)
		return env_loc;

	switch (dev) {
	case SPI_NOR_BOOT:
	case QSPI_BOOT:
	case FLEXSPI_BOOT:
		if (IS_ENABLED(CONFIG_ENV_IS_IN_SPI_FLASH))
			env_loc = ENVL_SPI_FLASH;
		break;
	case SD1_BOOT:
	case SD2_BOOT:
	case SD3_BOOT:
	case MMC1_BOOT:
	case MMC2_BOOT:
	case MMC3_BOOT:
		if (IS_ENABLED(CONFIG_ENV_IS_IN_MMC))
			env_loc =  ENVL_MMC;
		break;
	case USB_BOOT:
	default:
		if (IS_ENABLED(CONFIG_ENV_IS_NOWHERE))
			env_loc = ENVL_NOWHERE;
		break;
	}

	return env_loc;
}
