// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Module initialization for TQ-Systems TQMa62xx
 *
 * Copyright (c) 2020-2022 Texas Instruments Incorporated - https://www.ti.com/
 * Copyright (c) 2020-2025 TQ-Systems GmbH <u-boot@ew.tq-group.com>, D-82229 Seefeld, Germany.
 * Author: Matthias Schiffer
 */

#include <asm/arch/hardware.h>
#include <dm/device.h>
#include <dm/ofnode.h>
#include <fdt_support.h>
#include <i2c.h>
#include <init.h>
#include <power/tps65219.h>

#include "../common/common.h"
#include "tqma62xx.h"

#define PMIC_I2C_PATH "/bus@f0000/i2c@20000000"
#define PMIC_ADDR 0x30
#define PMIC_BUCK1_PATH PMIC_I2C_PATH "/pmic@30/regulators/buck1"

static int tqma62xx_read_core_voltage(void);

#if IS_ENABLED(CONFIG_K3_CORE_VOLTAGE_BOARD)
bool k3_is_core_voltage_0v85(void)
{
	int voltage;

	voltage = tqma62xx_read_core_voltage();
	if (voltage < 0)
		return false;

	return voltage >= 810000;
}
#endif

#if defined(CONFIG_XPL_BUILD) && IS_ENABLED(CONFIG_CPU_V7R)

/* From tps65219_regulator.c */
static int tps65219_buck_val2volt(int val)
{
	if (val > TPS65219_VOLT_MASK)
		return -EINVAL;
	else if (val > TPS65219_BUCK_REG_3V4)
		return TPS65219_BUCK_3V4;
	else if (val > TPS65219_BUCK_REG_1V4)
		return TPS65219_BUCK_1V4 + (val - TPS65219_BUCK_REG_1V4) * TPS65219_VOLT_STEP_100MV;
	else if (val >= TPS65219_BUCK_REG_0V6)
		return TPS65219_BUCK_0V6 + val * TPS65219_VOLT_STEP_25MV;
	else
		return -EINVAL;
}

/* In the R5 SPL, we read the voltage directly from the PMIC */
static int tqma62xx_read_core_voltage(void)
{
	static int ret;

	struct udevice *bus, *pmic;
	ofnode node;
	u8 reg;

	/* Return cached result */
	if (ret)
		return ret;

	node = ofnode_path(PMIC_I2C_PATH);
	if (!ofnode_valid(node))
		return -EINVAL;

	ret = device_get_global_by_ofnode(node, &bus);
	if (ret)
		return ret;

	ret = i2c_get_chip(bus, PMIC_ADDR, 1, &pmic);
	if (ret)
		return ret;

	ret = dm_i2c_read(pmic, TPS65219_BUCK1_VOUT_REG, &reg, sizeof(reg));
	if (ret)
		return ret;

	ret = tps65219_buck_val2volt(reg & TPS65219_VOLT_MASK);
	if (ret < 0)
		return ret;

	return ret;
}
#else
/* Use the voltage from the DTB that was set by the R5 SPL */
static int tqma62xx_read_core_voltage(void)
{
	static int ret;

	u32 min, max;
	ofnode node;

	/* Return cached result */
	if (ret)
		return ret;

	node = ofnode_path(PMIC_BUCK1_PATH);
	if (!ofnode_valid(node))
		return -EINVAL;

	ret = ofnode_read_u32(node, "regulator-min-microvolt", &min);
	if (ret)
		return ret;
	ret = ofnode_read_u32(node, "regulator-max-microvolt", &max);
	if (ret)
		return ret;

	if (min != max)
		return -EINVAL;
	if (min > INT_MAX)
		return -ERANGE;

	ret = min;
	return ret;
}
#endif

static void tqma62xx_ft_setup_core_voltage(void *blob, u32 value)
{
	int node;

	node = fdt_path_offset(blob, PMIC_BUCK1_PATH);
	if (node < 0)
		return;

	fdt_setprop_inplace_u32(blob, node, "regulator-min-microvolt", value);
	fdt_setprop_inplace_u32(blob, node, "regulator-max-microvolt", value);
}

#ifdef CONFIG_XPL_BUILD

void tq_common_spl_board_setup(void *blob)
{
	int core_voltage;

	core_voltage = tqma62xx_read_core_voltage();
	if (core_voltage > 0)
		tqma62xx_ft_setup_core_voltage(blob, core_voltage);
}

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

void tq_common_spl_board_setup(void *blob);

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

void tqma62xx_ft_board_setup(void *blob)
{
	enum fdt_status opp1400_status = FDT_STATUS_DISABLED;
	int core_voltage = tqma62xx_read_core_voltage();

	if (core_voltage > 0)
		tqma62xx_ft_setup_core_voltage(blob, core_voltage);

	if (k3_get_a53_max_frequency() >= 1400000000)
		opp1400_status = FDT_STATUS_OKAY;
	fdt_set_status_by_pathf(blob, opp1400_status, "/opp-table/opp-1400000000");
}

#endif /* !CONFIG_XPL_BUILD */
