// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <common.h>
#include <bloblist.h>
#include <cpu_func.h>
#include <hang.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <serial.h>
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
#include "../common/tq_blob.h"
#include "../common/tq_eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

extern struct dram_timing_info tqma93xxca_dram_timing_1gb;
extern struct dram_timing_info tqma93xxla_dram_timing_1gb;
extern struct dram_timing_info tqma93xxca_dram_timing_2gb;
extern struct dram_timing_info tqma93xxla_dram_timing_2gb;

struct dram_info {
	struct dram_timing_info	*table;  /* from NXP RPA */
	phys_size_t			size;    /* size of RAM */
	char				variant; /* char to help user query */
};

static const struct dram_info tqma93xx_dram_info[]  = {
	{ &tqma93xxca_dram_timing_1gb, SZ_1G * 1ULL, 'c' },
	/* reserved for 2 GB variant */
	{ &tqma93xxca_dram_timing_2gb, SZ_1G * 2ULL, 'c' },
	{ &tqma93xxla_dram_timing_1gb, SZ_1G * 1ULL, 'l' },
	/* reserved for 2 GB variant */
	{ &tqma93xxla_dram_timing_2gb, SZ_1G * 2ULL, 'l' },
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
	char sel = '-';
	char var = '-';
	unsigned int ramsize_gb;
	phys_size_t ramsize;
	int idx;

	puts("Warning: no valid EEPROM!\n"
		"Please enter LPDDR size in GByte to proceed.\n"
		"Valid sizes are 1 and 2.\n");

	for (;;) {
		/* Flush input */
		while (serial_tstc())
			serial_getc();

		sel = serial_getc();
		putc('\n');

		if ((sel == '1') || (sel == '2'))
			break;

		puts("Please enter a valid size.\n");
	}

	/*
	 * We expect ASCII codes from the terminal. So we can easily subtract
	 * to get the assembled size of DRAM in GB.
	 * Our lookup has the size in bytes, hence we have to multiply.
	 */
	ramsize_gb = (unsigned int)sel - (unsigned int)'0';
	ramsize = (phys_size_t)(ramsize_gb) * SZ_1G;

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
		printf("ERROR: no valid ram configuration, please reset\n");
		hang();
	}

	ddr_init(tqma93xx_dram_info[idx].table);
	tqma93xx_ram_timing_idx = idx;
}

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	return BOOT_DEVICE_BOOTROM;
}

void spl_board_init(void)
{
	/*
	 * board_init_f runs before bloblist_init, so we need this here
	 * in spl_board_init
	 */
	struct tq_raminfo *raminfo_blob;
	int ret;

	if (is_usb_boot())
		puts("USB Boot\n");
	else
		puts("Normal Boot\n");

	ret = ahab_start_rng();
	if (ret)
		printf("Fail to start RNG: %d\n", ret);

	tq_bb_spl_board_init();

	raminfo_blob = bloblist_ensure(BLOBLISTT_TQ_RAMSIZE,
				       sizeof(*raminfo_blob));
	if (raminfo_blob && tqma93xx_ram_timing_idx >= 0) {
		raminfo_blob->memsize =
			tqma93xx_dram_info[tqma93xx_ram_timing_idx].size;
	} else {
		printf("ERROR: no valid ram configuration, please reset\n");
		hang();
	}
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
	int ramtype;

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
	ramtype = handle_vard();
	tq_vard_show(&vard);
	spl_dram_init(ramtype);

	/* Put M33 into CPUWAIT for following kick */
	ret = m33_prepare();
	if (ret)
		printf("ERROR: M33 prepare %d\n", ret);

	board_init_r(NULL, 0);
}
