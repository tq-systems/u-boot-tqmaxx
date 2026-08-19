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

int tq_print_bootinfo(void)
{
	enum boot_device bt_dev;

	bt_dev = get_boot_device();

	puts("Boot:  ");
	switch (bt_dev) {
	case SD1_BOOT:
		puts("USDHC1(SD)\n");
		break;
	case SD2_BOOT:
		puts("USDHC2(SD)\n");
		break;
	case SD3_BOOT:
		puts("USDHC3(SD)\n");
		break;
	case MMC1_BOOT:
		puts("USDHC1(eMMC)\n");
		break;
	case MMC2_BOOT:
		puts("USDHC2(eMMC)\n");
		break;
	case MMC3_BOOT:
		puts("USDHC3(eMMC)\n");
		break;
	case USB_BOOT:
		puts("USB\n");
		break;
	case USB2_BOOT:
		puts("USB\n");
		break;
	case QSPI_BOOT:
		puts("FlexSPI\n");
		break;
	default:
		printf("Unknown/Unsupported device %u\n", bt_dev);
		break;
	}

	return 0;
}

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
