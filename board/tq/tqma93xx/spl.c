// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <common.h>
#include <bloblist.h>
#include <command.h>
#include <cpu_func.h>
#include <hang.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <serial.h>
#include <spl.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/imx93_pins.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/arch-mx7ulp/gpio.h>
#include <asm/mach-imx/syscounter.h>
#include <asm/mach-imx/s400_api.h>
#include <dm/uclass.h>
#include <dm/device.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>
#include <linux/delay.h>
#include <asm/arch/clock.h>
#include <asm/arch/ccm_regs.h>
#include <asm/arch/ddr.h>
#include <power/pmic.h>
#include <power/pca9450.h>
#include <asm/arch/trdc.h>

#include "../common/tq_bb.h"
#include "../common/tq_blob.h"
#include "../common/tq_eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

/* SPL currently not working with DM, use old style I2C API */
#define TQ_SYSTEM_EEPROM_BUS		0
#define TQ_SYSTEM_EEPROM_ADDR		0x53

extern struct dram_timing_info tqma93xxca_dram_timing;
extern struct dram_timing_info tqma93xxla_dram_timing;

struct dram_info {
	struct dram_timing_info	*table; /* from NXP RPA */
	phys_size_t		size;   /* size of RAM */
	char			variant;
};

static struct dram_info tqma93xx_dram_info[]  = {
	{ &tqma93xxca_dram_timing, SZ_1G * 1ULL, 'c' },
	/* reserved for 2 GB variant */
	{ NULL, SZ_1G * 2ULL, 'c' },
	{ &tqma93xxla_dram_timing, SZ_1G * 1ULL, 'l' },
	/* reserved for 2 GB variant */
	{ NULL, SZ_1G * 2ULL, 'l' },
};

static int tqma93xx_ram_timing_idx = -1;

static struct tq_vard vard;

static int handle_vard(void)
{
	if (tq_vard_read(TQ_SYSTEM_EEPROM_BUS, TQ_SYSTEM_EEPROM_ADDR, &vard))
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
	ramsize = (phys_size_t)((unsigned int)sel - (unsigned int)('0')) * SZ_1G;

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

	if (idx >= ARRAY_SIZE(tqma93xx_dram_info) ||
	    !tqma93xx_dram_info[idx].table) {
		puts("ERROR: no valid RAM timing or timing not in image, stop\n");
		hang();
	}

	return idx;
}

static void spl_dram_init(int memtype)
{
	int idx = -1;
	char variant;

	/* normal configuration */
	phys_size_t ramsize;

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
		ramsize = tq_vard_ramsize(&vard);
		for (idx = 0; idx < ARRAY_SIZE(tqma93xx_dram_info); ++idx) {
			if (ramsize == tqma93xx_dram_info[idx].size &&
			    variant == tqma93xx_dram_info[idx].variant)
				break;
		}
		if (idx >= ARRAY_SIZE(tqma93xx_dram_info))
			puts("DRAM init: no matching timing\n");
	} else if (vard.memtype == VARD_MEMTYPE_DEFAULT) {
		puts("DRAM init: vard invalid?\n");
	} else {
		printf("DRAM init: unknown RAM type %u\n",
		       (unsigned int)vard.memtype);
	}

	if (idx < 0 || idx >= ARRAY_SIZE(tqma93xx_dram_info))
		idx = tqma93xx_query_ddr_timing();

	if (idx < 0 || idx >= ARRAY_SIZE(tqma93xx_dram_info)) {
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

	puts("Normal Boot\n");
}

#if CONFIG_IS_ENABLED(DM_PMIC_PCA9450)
int power_init_board(void)
{
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
}
#endif

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

	ret = arch_cpu_init_dm();
	if (ret) {
		printf("Fail to init Sentinel API\n");
	} else {
		printf("SOC: 0x%x\n", gd->arch.soc_rev);
		printf("LC: 0x%x\n", gd->arch.lifecycle);
	}
	power_init_board();

	/* Increase ARM clock to max allowed for speed grade */
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
	if (!ret)
		printf("M33 prepare ok\n");

	board_init_r(NULL, 0);
}
