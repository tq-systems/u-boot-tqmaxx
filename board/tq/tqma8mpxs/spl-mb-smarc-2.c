// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021-2025 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <spl.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>

#include "../common/tq_bb.h"

DECLARE_GLOBAL_DATA_PTR;

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	if (IS_ENABLED(CONFIG_SPL_BOOTROM_SUPPORT))
		return BOOT_DEVICE_BOOTROM;

	switch (boot_dev_spl) {
	case SD1_BOOT:
	case MMC1_BOOT:
	case SD2_BOOT:
	case MMC2_BOOT:
		return BOOT_DEVICE_MMC1;
	case SD3_BOOT:
	case MMC3_BOOT:
		return BOOT_DEVICE_MMC2;
	case QSPI_BOOT:
		return BOOT_DEVICE_NOR;
	case NAND_BOOT:
		return BOOT_DEVICE_NAND;
	case USB_BOOT:
		return BOOT_DEVICE_BOARD;
	default:
		return BOOT_DEVICE_NONE;
	}

	return BOOT_DEVICE_NONE;
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* Just empty function now - needed for multi dtb in fitImage */
	debug("%s: %s\n", __func__, name);

	return 0;
}
#endif
