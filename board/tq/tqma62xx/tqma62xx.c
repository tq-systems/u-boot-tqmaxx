// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Module initialization for TQ-Systems TQMa62xx
 *
 * Copyright (c) 2020-2022 Texas Instruments Incorporated - https://www.ti.com/
 * Copyright (c) 2020-2025 TQ-Systems GmbH <u-boot@ew.tq-group.com>, D-82229 Seefeld, Germany.
 * Author: Matthias Schiffer
 */

#include <asm/arch/hardware.h>
#include <init.h>

#ifdef CONFIG_XPL_BUILD

#define CTRLMMR_USB0_PHY_CTRL	(WKUP_CTRL_MMR0_BASE + 0x4008)
#define CTRLMMR_USB1_PHY_CTRL	(WKUP_CTRL_MMR0_BASE + 0x4018)
#define CORE_VOLTAGE		0x80000000

void spl_board_init(void)
{
	u32 val;

	/* Clear USB0_PHY_CTRL_CORE_VOLTAGE */
	/* TI recommends to clear the bit regardless of VDDA_CORE_USB */
	val = readl(CTRLMMR_USB0_PHY_CTRL);
	val &= ~CORE_VOLTAGE;
	writel(val, CTRLMMR_USB0_PHY_CTRL);

	/* Clear USB1_PHY_CTRL_CORE_VOLTAGE */
	val = readl(CTRLMMR_USB1_PHY_CTRL);
	val &= ~CORE_VOLTAGE;
	writel(val, CTRLMMR_USB1_PHY_CTRL);

	/* We have 32k crystal, so lets enable it */
	val = readl(MCU_CTRL_LFXOSC_CTRL);
	val &= ~(MCU_CTRL_LFXOSC_32K_DISABLE_VAL);
	writel(val, MCU_CTRL_LFXOSC_CTRL);
	/* Add any TRIM needed for the crystal here.. */
	/* Make sure to mux up to take the SoC 32k from the crystal */
	writel(MCU_CTRL_DEVICE_CLKOUT_LFOSC_SELECT_VAL,
	       MCU_CTRL_DEVICE_CLKOUT_32K_CTRL);
}

#else /* CONFIG_XPL_BUILD */

static const char *tqma62xx_cpu_type(void)
{
	u32 device_id = readl(CTRLMMR_WKUP_JTAG_DEVICE_ID);

	switch ((device_id & JTAG_DEV_ID_MASK) >> JTAG_DEV_ID_SHIFT) {
	case JTAG_DEV_ID_AM6254:
		return "AM6254";
	case JTAG_DEV_ID_AM6252:
		return "AM6252";
	case JTAG_DEV_ID_AM6251:
		return "AM6251";
	case JTAG_DEV_ID_AM6234:
		return "AM6234";
	case JTAG_DEV_ID_AM6232:
		return "AM6232";
	case JTAG_DEV_ID_AM6231:
		return "AM6231";
	default:
		return "unknown";
	}
}

int checkboard(void)
{
	printf("SoC variant: %s\n", tqma62xx_cpu_type());
	return 0;
}

#endif /* !CONFIG_XPL_BUILD */
