// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Module initialization for TQ-Systems TQMa64xxL
 *
 * Copyright (c) 2017-2018 Texas Instruments Incorporated - http://www.ti.com/
 * Copyright (c) 2020-2025 TQ-Systems GmbH <u-boot@ew.tq-group.com>, D-82229 Seefeld, Germany.
 * Author: Matthias Schiffer
 */

#include <asm/arch/hardware.h>
#include <init.h>

#ifdef CONFIG_XPL_BUILD

#define CTRLMMR_USB0_PHY_CTRL	(WKUP_CTRL_MMR0_BASE + 0x4008)
#define CORE_VOLTAGE		0x80000000

void spl_board_init(void)
{
	u32 val;
	/* Set USB PHY core voltage to 0.85V */
	val = readl(CTRLMMR_USB0_PHY_CTRL);
	val &= ~(CORE_VOLTAGE);
	writel(val, CTRLMMR_USB0_PHY_CTRL);
}

#else /* CONFIG_XPL_BUILD */

size_t tq_common_sysinfo_macaddr_num(void) {
	return k3_has_icss() ? 4 : 1;
}

static const char *tqma64xxl_cpu_type(void)
{
	u32 device_id = readl(CTRLMMR_WKUP_JTAG_DEVICE_ID);

	switch ((device_id & JTAG_DEV_ID_MASK) >> JTAG_DEV_ID_SHIFT) {
	case JTAG_DEV_ID_AM6442:
		return "AM6442";
	case JTAG_DEV_ID_AM6441:
		return "AM6441";
	case JTAG_DEV_ID_AM6422:
		return "AM6422";
	case JTAG_DEV_ID_AM6421:
		return "AM6421";
	case JTAG_DEV_ID_AM6412:
		return "AM6412";
	case JTAG_DEV_ID_AM6411:
		return "AM6411";
	default:
		return "unknown";
	}
}

int checkboard(void)
{
	printf("SoC variant: %s\n", tqma64xxl_cpu_type());
	return 0;
}

#endif /* CONFIG_XPL_BUILD */
