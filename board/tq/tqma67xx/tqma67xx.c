// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 Texas Instruments Incorporated - https://www.ti.com/
 * Copyright (c) 2020-2025 TQ-Systems GmbH <u-boot@ew.tq-group.com>, D-82229 Seefeld, Germany.
 * Author: Nora Schiffer
 *
 * Module initialization for TQ-Systems TQMa67xx
 */

#include <asm/arch/hardware.h>
#include <init.h>
#include <linux/bitfield.h>
#include <power/pmic.h>

#if IS_ENABLED(CONFIG_XPL_BUILD)

#if IS_ENABLED(CONFIG_CPU_V7R)

static void init_pmic(void)
{
#if CONFIG_IS_ENABLED(DM_PMIC) && IS_ENABLED(CONFIG_PMIC_TPS65941)
	struct udevice *dev;
	int ret, reg;

	ret = pmic_get("pmic@48", &dev);
	if (ret) {
		printf("PMIC not found\n");
		return;
	}

#define TPS65224_REG_FSM_TRIG_MASK_2 0x47
#define TPS65224_REG_FSM_TRIG_MASK_2__GPIO6_FSM_MASK 0x04
#define TPS65224_REG_FSM_TRIG_MASK_2__GPIO6_FSM_MASK_POL 0x08

#define TPS65224_REG_RECOV_CNT_REG_1 0x83
#define TPS65224_REG_RECOV_CNT_REG_1__RECOV_CNT 0x0f

#define TPS65224_REG_RECOV_CNT_REG_2 0x84
#define TPS65224_REG_RECOV_CNT_REG_2__RECOV_CNT_CLR 0x10

	reg = pmic_reg_read(dev, TPS65224_REG_FSM_TRIG_MASK_2);
	if (reg < 0) {
		printf("Reading PMIC register FSM_TRIG_MASK_2 failed: %d\n", reg);
	} else {
		/* Mask GPIO6 signal */
		reg |= TPS65224_REG_FSM_TRIG_MASK_2__GPIO6_FSM_MASK |
		       TPS65224_REG_FSM_TRIG_MASK_2__GPIO6_FSM_MASK_POL;
		ret = pmic_reg_write(dev, TPS65224_REG_FSM_TRIG_MASK_2, reg);
		if (ret < 0)
			printf("Writing PMIC register FSM_TRIG_MASK_2 failed: %d\n", ret);
	}

	reg = pmic_reg_read(dev, TPS65224_REG_RECOV_CNT_REG_1);
	if (reg < 0) {
		printf("Reading PMIC register RECOV_CNT_REG_1 failed: %d\n", reg);
	} else {
		unsigned int recov_cnt = reg & TPS65224_REG_RECOV_CNT_REG_1__RECOV_CNT;

		if (recov_cnt)
			printf("PMIC recovery counter: %u\n", recov_cnt);
	}

	reg = pmic_reg_read(dev, TPS65224_REG_RECOV_CNT_REG_2);
	if (reg < 0) {
		printf("Reading PMIC register RECOV_CNT_REG_2 failed: %d\n", reg);
	} else {
		/* Clear recovery counter */
		reg |= TPS65224_REG_RECOV_CNT_REG_2__RECOV_CNT_CLR;
		ret = pmic_reg_write(dev, TPS65224_REG_RECOV_CNT_REG_2, reg);
		if (ret < 0)
			printf("Writing PMIC register RECOV_CNT_REG_2 failed: %d\n", ret);
	}

	printf("PMIC setup done\n");
#endif
}

void spl_board_init(void) {
	u32 val;

	init_pmic();

	/* Schematic design note:
	 * CTRLMMR_WKUP_LFXOSC_TRIM[18:16] i_mult
	 * must be set to = 3b’001 for CL in the range 6pf to 9.5pf.
	 */

#define MCU_CTRL_LFXOSC_TRIM_I_MULT GENMASK(18, 16)

	val = readl(MCU_CTRL_LFXOSC_TRIM);
	val &= ~MCU_CTRL_LFXOSC_TRIM_I_MULT;
	val |= FIELD_PREP(MCU_CTRL_LFXOSC_TRIM_I_MULT, 0x1);
	writel(val, MCU_CTRL_LFXOSC_TRIM);

	val = readl(MCU_CTRL_LFXOSC_CTRL);
	val &= ~(MCU_CTRL_LFXOSC_32K_DISABLE_VAL);
	writel(val, MCU_CTRL_LFXOSC_CTRL);
	writel(MCU_CTRL_DEVICE_CLKOUT_LFOSC_SELECT_VAL,
	       MCU_CTRL_DEVICE_CLKOUT_32K_CTRL);
}

#endif /* CONFIG_CPU_V7R */

#else /* CONFIG_XPL_BUILD */

static const char *tqma67xx_cpu_type(void)
{
	u32 device_id = readl(CTRLMMR_WKUP_JTAG_DEVICE_ID);

	switch ((device_id & JTAG_DEV_ID_MASK) >> JTAG_DEV_ID_SHIFT) {
	case JTAG_DEV_ID_AM6734:
		return "AM6734";
	case JTAG_DEV_ID_AM6754:
		return "AM6754";
	case JTAG_DEV_ID_AM67A74:
		return "AM67A74";
	case JTAG_DEV_ID_AM67A94:
		return "AM67A94";
	case JTAG_DEV_ID_TDA4AEN:
		return "TDA4AEN";
	case JTAG_DEV_ID_TDA4VEN:
		return "TDA4VEN";
	case JTAG_DEV_ID_XJ722S:
		return "XJ722S";
	default:
		return "unknown";
	}
}

int checkboard(void)
{
	printf("SoC variant: %s\n", tqma67xx_cpu_type());
	return 0;
}

#endif /* CONFIG_XPL_BUILD */
