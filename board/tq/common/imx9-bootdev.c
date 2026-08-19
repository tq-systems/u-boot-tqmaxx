// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <env.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include "tq_bb.h"

void tq_set_boot_targets(void)
{
	enum boot_device bt_dev;
	const char *target;

	if (env_get("boot_targets"))
		return;

	bt_dev = get_boot_device();

	switch (bt_dev) {
	case SD3_BOOT:
	case MMC3_BOOT:
		target = "mmc0";
		break;
	case SD2_BOOT:
	case MMC2_BOOT:
		target = "mmc1";
		break;
	case USB_BOOT:
		target = "usb0";
		break;
	case QSPI_BOOT:
		/* Change to serial_flash0 when migrating from distroboot to stdboot */
		target = "sf0";
		break;
	default:
		return;
	}

	env_set("boot_targets", target);
}
