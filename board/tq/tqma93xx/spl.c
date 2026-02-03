// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022-2024 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <cpu_func.h>
#include <hang.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <serial.h>
#include <spl.h>
#include <asm/global_data.h>
#include <asm/sections.h>
#include <asm/arch/clock.h>
#include <asm/arch/ddr.h>
#include <asm/arch/mu.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/trdc.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/ele_api.h>
#include <dm/device.h>
#include <power/pmic.h>
#include <power/pca9450.h>

#include "../common/tq_bb.h"
#include "../common/tq_eeprom.h"

#include "tqma91_93-dram.h"

DECLARE_GLOBAL_DATA_PTR;

static const struct dram_info tqma93xx_dram_info[]  = {
#if IS_ENABLED(CONFIG_IMX91)

#if IS_ENABLED(CONFIG_TQMA93XX_RAM_1024MB)
	DRAM_INFO_ENTRY(tqma91xxca_dram_timing_1gb, TQMA93XXXA_RAM_SIZE_1G, 'c'),
	DRAM_INFO_ENTRY(tqma91xxla_dram_timing_1gb, TQMA93XXXA_RAM_SIZE_1G, 'l'),
#endif

#elif IS_ENABLED(CONFIG_IMX93)

#if IS_ENABLED(CONFIG_TQMA93XX_RAM_1024MB)
	DRAM_INFO_ENTRY(tqma93xxca_dram_timing_1gb, TQMA93XXXA_RAM_SIZE_1G, 'c'),
	DRAM_INFO_ENTRY(tqma93xxla_dram_timing_1gb, TQMA93XXXA_RAM_SIZE_1G, 'l'),
#endif
#if IS_ENABLED(CONFIG_TQMA93XX_RAM_1536MB)
	DRAM_INFO_ENTRY(tqma93xxla_dram_timing_1gb5, TQMA93XXXA_RAM_SIZE_1G5, 'l'),
#endif
#if IS_ENABLED(CONFIG_TQMA93XX_RAM_2048MB)
	DRAM_INFO_ENTRY(tqma93xxca_dram_timing_2gb, TQMA93XXXA_RAM_SIZE_2G, 'c'),
	DRAM_INFO_ENTRY(tqma93xxla_dram_timing_2gb, TQMA93XXXA_RAM_SIZE_2G, 'l'),
#endif

#endif
};

static int tqma93xx_ram_timing_idx = -1;

static struct tq_vard vard;

static inline bool is_valid_dram_entry(int idx)
{
	if (idx < 0 || idx >= ARRAY_SIZE(tqma93xx_dram_info))
		return false;
	if (!tqma93xx_dram_info[idx].table)
		return false;

	return true;
}

static int handle_vard(void)
{
	if (tq_vard_read(&vard))
		puts("ERROR: vard read\n");
	else if (!tq_vard_valid(&vard))
		puts("ERROR: vard CRC\n");
	else
		return (int)(vard.memtype & VARD_MEMTYPE_MASK_TYPE);

	return VARD_MEMTYPE_DEFAULT;
};

static int tqma93xx_query_ddr_timing(void)
{
	unsigned int ramsize_choice;
	phys_size_t ramsize;
	char sel = '-';
	char var = '-';
	int idx;

	puts("Warning: no valid EEPROM!\n"
	     "Please choose LPDDR size to proceed.\n"
	     "1 - 512 MiB\n"
	     "2 -   1 GiB\n"
	     "3 - 1.5 GiB\n"
	     "4 -   2 GiB\n");

	for (;;) {
		/* Flush input */
		while (serial_tstc())
			serial_getc();

		sel = serial_getc();
		putc('\n');

		if ((sel == '1') || (sel == '2') || (sel == '3') || (sel == '4'))
			break;

		puts("Please enter a valid size.\n");
	}

	/*
	 * We expect ASCII codes from the terminal. So we can easily subtract
	 * to get the choice.
	 */
	ramsize_choice = (unsigned int)sel - (unsigned int)'0';

	switch (ramsize_choice) {
	case TQMA93XXXA_RAM_SIZE_0G5:
	case TQMA93XXXA_RAM_SIZE_1G:
	case TQMA93XXXA_RAM_SIZE_1G5:
	case TQMA93XXXA_RAM_SIZE_2G:
		ramsize = ramsize_choice * ULL(SZ_512M);
		break;
	default:
		puts("ERROR: no valid RAM size given, stop\n");
		hang();
	}

	puts("Warning: no valid EEPROM!\n"
	     "Please enter form factor to proceed.\n"
	     "Valid are l (LGA variant) and c (click in variant).\n");

	for (;;) {
		/* Flush input */
		while (serial_tstc())
			serial_getc();

		var = serial_getc();
		putc('\n');

		if ((var == 'l') || (var == 'c'))
			break;

		puts("Please enter a valid variant.\n");
	}

	for (idx = 0; idx < ARRAY_SIZE(tqma93xx_dram_info); ++idx) {
		if (ramsize == tqma93xx_dram_info[idx].size &&
		    var == tqma93xx_dram_info[idx].variant)
			break;
	}

	if (!is_valid_dram_entry(idx)) {
		puts("ERROR: no valid RAM timing or timing not in image, stop\n");
		hang();
	}

	return idx;
}

static void spl_dram_init(int memtype)
{
	int idx = -1;
	char variant;

	switch (tq_vard_get_formfactor(&vard)) {
	case VARD_FORMFACTOR_TYPE_LGA:
		variant = 'l';
		break;
	case VARD_FORMFACTOR_TYPE_CONNECTOR:
		variant = 'c';
		break;
	default:
		variant = '-';
		break;
	}

	if (memtype == 1) {
		phys_size_t ramsize = tq_vard_ramsize(&vard);

		for (idx = 0; idx < ARRAY_SIZE(tqma93xx_dram_info); ++idx) {
			if (ramsize == tqma93xx_dram_info[idx].size &&
			    variant == tqma93xx_dram_info[idx].variant)
				break;
		}
		if (!is_valid_dram_entry(idx))
			puts("DRAM init: no matching timing\n");
	} else if (vard.memtype == VARD_MEMTYPE_DEFAULT) {
		puts("DRAM init: vard invalid?\n");
	} else {
		printf("DRAM init: unknown RAM type %u\n",
		       (unsigned int)vard.memtype);
	}

	if (!is_valid_dram_entry(idx))
		idx = tqma93xx_query_ddr_timing();

	if (!is_valid_dram_entry(idx)) {
		puts("ERROR: no valid ram configuration, please reset\n");
		hang();
	}

	/*
	 * ddr_ddrphy_trained_csr_num can't be set in the timing configuration
	 * directly, as it is defined in a different compilation unit. Fill it
	 * in here.
	 */
	tqma93xx_dram_info[idx].table->ddrphy_trained_csr = ddr_ddrphy_trained_csr;
	tqma93xx_dram_info[idx].table->ddrphy_trained_csr_num = ddr_ddrphy_trained_csr_num;

	tqma93xx_ram_timing_idx = idx;
	if (ddr_init(tqma93xx_dram_info[idx].table))
		tqma93xx_ram_timing_idx = -1;
}

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

	ret = ele_start_rng();
	if (ret)
		printf("Fail to start RNG: %d\n", ret);

	tq_bb_spl_board_init();
	if (!(is_valid_dram_entry(tqma93xx_ram_timing_idx))) {
		printf("ERROR: no valid ram configuration, please reset\n");
		hang();
	}
}

static int power_init_board(void)
{
	if (CONFIG_IS_ENABLED(DM_PMIC_PCA9450)) {
		unsigned int trim_val, buck_val;
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
		/* enable DVS control through PMIC_STBY_REQ */
		pmic_reg_write(dev, PCA9450_BUCK1CTRL, 0x59);

		/*
		 * HACK:
		 * Newer PMIC versions defaults to PCA9450_REG_PWRCTRL_TOFF_DEB == 1b
		 * while older versions had PCA9450_REG_PWRCTRL_TOFF_DEB == 0b as
		 * power on default.
		 * This is the only way to handle differences for BUCK1/3 regulator range.
		 * regulator step equals to 12.5 mV, new versions start at 650 mV while
		 * older versions start at 600 mV, hence 0x4 equals to 50 mV.
		 * Attention: When setting PCA9450_REG_PWRCTRL_TOFF_DEB at runtime and using
		 * WDOG warm reset this will not work.
		 */
		ret = pmic_reg_read(dev, PCA9450_PWR_CTRL);
		if (ret < 0) {
			pr_err("ERROR: access pca9450@25 %d\n", ret);
			return ret;
		}

		trim_val = ((unsigned int)ret & PCA9450_REG_PWRCTRL_TOFF_DEB) ? 0x0 : 0x4;
		puts("PMIC: ");
		if (is_voltage_mode(VOLT_LOW_DRIVE)) {
			/* 0.8v for Low drive mode */
			buck_val = 0x0c + trim_val;
			puts("Low Drive ");
		} else if (is_voltage_mode(VOLT_NOMINAL_DRIVE)) {
			/* 0.85v for Nominal drive mode */
			buck_val = 0x10 + trim_val;
			puts("Nominal ");
		} else {
			/* 0.9v for Over drive mode */
			buck_val = 0x14 + trim_val;
			puts("Over Drive ");
		}
		puts("Voltage Mode\n");

		pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, buck_val);
		pmic_reg_write(dev, PCA9450_BUCK3OUT_DVS0, buck_val);
		/* set standby voltage to 0.65v */
		pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS1, trim_val);

		if (IS_ENABLED(CONFIG_IMX91)) {
			puts("TQMa91xx: LPDDR4\n");
			/* Set VDDQ to 1.1V from buck2 (LPDDR4): 600 mV + 40 * 12.5 mV */
			pmic_reg_write(dev, PCA9450_BUCK2OUT_DVS0, 40);

		} else if (IS_ENABLED(CONFIG_IMX93)) {
			puts("TQMa93xx: LPDDR4x\n");
			/* Set VDDQ to 0.6V from buck2 (LPDDR4x): 600 mV + 0 * 12.5 mV */
			pmic_reg_write(dev, PCA9450_BUCK2OUT_DVS0, 0);
		}
		/*
		 * I2C_LT_EN: I2C level translator Forcedly Disable (POR default)
		 * usage is dicouraged by TQ Systems according to in house testing
		 */
		pmic_reg_write(dev, PCA9450_CONFIG2, 0x00);
		/*
		 * T_PMIC_RST_DEB: 001b (50 ms, default)
		 * PMIC_RST_CFG: 10b Cold Reset except LDO1 (V_1V8_BBSM), default
		 * WDOG_B_CFG: 10b  Cold Reset except LDO1 (V_1V8_BBSM)
		 * This follows the recommendations from NXP hardware design guide
		 */
		pmic_clrsetbits(dev, PCA9450_RESET_CTRL,
				PCA9450_PMIC_RESET_WDOG_B_CFG_MASK,
				PCA9450_PMIC_RESET_WDOG_B_CFG_COLD_LDO12);
	}

	return 0;
}

void board_init_f(ulong dummy)
{
	int ret;
	int ramtype;

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	timer_init();

	arch_cpu_init();

	board_early_init_f();

	spl_early_init();

	preloader_console_init();

	ret = imx9_probe_mu();
	if (ret) {
		printf("ERROR: init ELE API %d\n", ret);
	} else {
		printf("SOC rev:   0x%x\n", gd->arch.soc_rev);
		printf("lifecycle: 0x%x\n", gd->arch.lifecycle);
	}

	clock_init_late();

	power_init_board();

	if (!is_voltage_mode(VOLT_LOW_DRIVE))
		set_arm_core_max_clk();

	/* Init power of mix */
	soc_power_init();

	/* Setup TRDC for DDR access */
	trdc_init();

	/* DDR initialization */
	ramtype = handle_vard();
	tq_vard_show(&vard);
	spl_dram_init(ramtype);

	if (IS_ENABLED(CONFIG_IMX93)) {
		/* Put M33 into CPUWAIT for following kick */
		ret = m33_prepare();
		if (ret)
			printf("ERROR: M33 prepare %d\n", ret);
	}

	board_init_r(NULL, 0);
}
