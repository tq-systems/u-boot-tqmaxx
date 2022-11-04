// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <common.h>
#include <cpu_func.h>
#include <hang.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <spl.h>
#include <asm/global_data.h>
#include <asm/arch/clock.h>
#include <asm/arch/ddr.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/trdc.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/ele_api.h>
#include <dm/device.h>
#include <power/pmic.h>
#include <power/pca9450.h>

#include "../common/tq_bb.h"

DECLARE_GLOBAL_DATA_PTR;

extern struct dram_timing_info tqma93xxla_dram_timing;

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	return BOOT_DEVICE_BOOTROM;
}

void spl_board_init(void)
{
	int ret;

	if (is_usb_boot())
		puts("USB Boot\n");
	else
		puts("Normal Boot\n");

	ret = ahab_start_rng();
	if (ret)
		printf("Fail to start RNG: %d\n", ret);
}

void spl_dram_init(void)
{
	ddr_init(&tqma93xxla_dram_timing);
}

static int power_init_board(void)
{
#if CONFIG_IS_ENABLED(DM_PMIC_PCA9450)
	struct udevice *dev;
	int ret;

	ret = pmic_get("pmic@25", &dev);
	if (ret == -ENODEV) {
		puts("ERROR: pca9450@25 not found\n");
		return 0;
	}
	if (ret != 0) {
		pr_err("ERROR: request pca9450@25 %d\n", ret);
		return ret;
	}

	/* BUCKxOUT_DVS0/1 control BUCK123 output */
	pmic_reg_write(dev, PCA9450_BUCK123_DVS, 0x29);

/* TODO: LOW_DRIVE_MODE / OVERDRIVE + PWRCTRL_TOFF_DEB -> imx93-evk */

	/* enable DVS control through PMIC_STBY_REQ */
	pmic_reg_write(dev, PCA9450_BUCK1CTRL, 0x59);
	/* 0.9 V */
	pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, 0x18);
	pmic_reg_write(dev, PCA9450_BUCK3OUT_DVS0, 0x18);
	/* set standby voltage to 0.65v */
	pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS1, 0x4);

	/* I2C_LT_EN*/
	pmic_reg_write(dev, 0xa, 0x3);

	/* set WDOG_B_CFG to cold reset */
	pmic_reg_write(dev, PCA9450_RESET_CTRL, 0xA1);
	return 0;
#endif
}

extern int imx9_probe_mu(void *ctx, struct event *event);

void board_init_f(ulong dummy)
{
	int ret;

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	timer_init();

	arch_cpu_init();

	board_early_init_f();

	spl_early_init();

	preloader_console_init();

	ret = imx9_probe_mu(NULL, NULL);
	if (ret) {
		printf("ERROR: init ELE API %d\n", ret);
	} else {
		printf("SOC rev:   0x%x\n", gd->arch.soc_rev);
		printf("lifecycle: 0x%x\n", gd->arch.lifecycle);
	}

	power_init_board();

	if (!IS_ENABLED(CONFIG_IMX9_LOW_DRIVE_MODE))
		set_arm_core_max_clk();

	/* Init power of mix */
	soc_power_init();

	/* Setup TRDC for DDR access */
	trdc_init();

	/* DDR initialization */
	spl_dram_init();

	/* Put M33 into CPUWAIT for following kick */
	ret = m33_prepare();
	if (ret)
		printf("ERROR: M33 prepare %d\n", ret);

	board_init_r(NULL, 0);
}
