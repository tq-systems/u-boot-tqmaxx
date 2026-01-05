// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Paul Gerber
 */

#include <hang.h>
#include <init.h>
#include <log.h>
#include <serial.h>
#include <spl.h>
#include <asm/arch/imx8mp_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/sections.h>
#include <bloblist.h>
#include <power/pmic.h>

#include <power/pca9450.h>
#include <asm/arch/clock.h>
#include <dm/uclass.h>
#include <dm/device.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>
#include <mmc.h>
#include <asm/arch/ddr.h>

#include "../common/tq_bb.h"
#include "../common/tq_blob.h"
#include "../common/tq_eeprom.h"

#include "tqma8mpxs-dram.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * Currently this is known to work with up to four timings in one SPL
 * Needs to be rechecked when increasing
 */
static struct dram_info tqma8mpxs_dram_info[]  = {
#if IS_ENABLED(CONFIG_TQMA8MPXS_RAM_1024MB)
	DRAM_INFO_ENTRY(1),
#endif
#if IS_ENABLED(CONFIG_TQMA8MPXS_RAM_2048MB)
	DRAM_INFO_ENTRY(2),
#endif
#if IS_ENABLED(CONFIG_TQMA8MPXS_RAM_4096MB)
	DRAM_INFO_ENTRY(4),
#endif
#if IS_ENABLED(CONFIG_TQMA8MPXS_RAM_8192MB)
	DRAM_INFO_ENTRY(8)
#endif
};

static int tqma8mpxs_ram_timing_idx = -1;

static struct tq_vard vard;

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

#if IS_ENABLED(CONFIG_TQMA8MPXS_RAM_MULTI)
static int tqma8mpxs_query_ddr_timing(void)
{
	char sel = '-';
	phys_size_t ramsize;
	int idx;

	puts("Warning: no valid EEPROM!\n"
		"Please enter LPDDR size in GByte to proceed.\n"
		"Valid sizes are 1,2,4 and 8.\n");

	for (;;) {
		/* Flush input */
		while (serial_tstc())
			serial_getc();

		sel = serial_getc();
		putc('\n');

		if ((sel == '1') || (sel == '2') || (sel == '4') || (sel == '8'))
			break;

		puts("Please enter a valid size.\n");
	}

	ramsize = (phys_size_t)((unsigned int)sel - (unsigned int)('0')) * SZ_1G;
	for (idx = 0; idx < ARRAY_SIZE(tqma8mpxs_dram_info); ++idx) {
		if (ramsize == tqma8mpxs_dram_info[idx].size)
			break;
	}

	if (idx >= ARRAY_SIZE(tqma8mpxs_dram_info) ||
	    !tqma8mpxs_dram_info[idx].table) {
		puts("ERROR: no valid RAM timing or timing not in image, stop\n");
		hang();
	}

	return idx;
}
#endif

#if IS_ENABLED(CONFIG_IMX8M_DRAM_INLINE_ECC)
void board_dram_ecc_scrub(void)
{
	if (tqma8mpxs_ram_timing_idx >= 0) {
		tqma8mpxs_dram_info[tqma8mpxs_ram_timing_idx].board_dram_ecc_scrub();
	} else {
		printf("ERROR: no valid ram configuration, please reset\n");
		hang();
	}
}
#endif

static void spl_dram_init(int memtype)
{
	int ret;
	int idx = -1;

	/* normal configuration */
	if (IS_ENABLED(CONFIG_TQMA8MPXS_RAM_MULTI)) {
		phys_size_t ramsize;

		if (memtype == 1) {
			ramsize = tq_vard_ramsize(&vard);
			for (idx = 0; idx < ARRAY_SIZE(tqma8mpxs_dram_info); ++idx) {
				if (ramsize == tqma8mpxs_dram_info[idx].size)
					break;
			}
			if (idx >= ARRAY_SIZE(tqma8mpxs_dram_info))
				puts("DRAM init: no matching timing\n");
		} else if (vard.memtype == VARD_MEMTYPE_DEFAULT) {
			puts("DRAM init: vard invalid?\n");
		} else {
			printf("DRAM init: unknown RAM type %u\n",
			       (unsigned int)vard.memtype);
		}

		if (idx < 0 || idx >= ARRAY_SIZE(tqma8mpxs_dram_info))
			idx = tqma8mpxs_query_ddr_timing();
	} else {
		puts("DRAM init: missing or invalid RAM config");
	}

	if (idx < 0 || idx >= ARRAY_SIZE(tqma8mpxs_dram_info)) {
		printf("ERROR: no valid ram configuration, please reset\n");
		hang();
	}

	/*
	 * ddr_ddrphy_trained_csr_num can't be set in the timing configuration
	 * directly, as it is defined in a different compilation unit. Fill it
	 * in here.
	 */
	tqma8mpxs_dram_info[idx].table->ddrphy_trained_csr = ddr_ddrphy_trained_csr;
	tqma8mpxs_dram_info[idx].table->ddrphy_trained_csr_num = ddr_ddrphy_trained_csr_num;

	/*
	 * If inline ECC is enabled, ddr_init calls board_dram_ecc_scrub. The
	 * TQMa8MPxL implementation of board_dram_ecc_scrub uses
	 * tqma8mpxs_ram_timing_idx to call the appropriate scrubbing function
	 * for ram size. Set the index before calling ddr_init.
	 * In case of initialization error set the index back to -1, so
	 * later users of this index can handle the error.
	 */
	tqma8mpxs_ram_timing_idx = idx;
	ret = ddr_init(tqma8mpxs_dram_info[idx].table);
	if (ret)
		tqma8mpxs_ram_timing_idx = -1;
}

#define UBOOT_RAW_SECTOR_OFFSET 0x40
unsigned long spl_mmc_get_uboot_raw_sector(struct mmc *mmc,
					   unsigned long raw_sect)
{
	u32 boot_dev = spl_boot_device();

	switch (boot_dev) {
	case BOOT_DEVICE_MMC2:
		return CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR -
		       UBOOT_RAW_SECTOR_OFFSET;
	default:
		return CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR;
	}
}

void spl_board_init(void)
{
	arch_misc_init();

	/*
	 * Set GIC clock to 500Mhz for OD VDD_SOC. Kernel driver does
	 * not allow to change it. Should set the clock after PMIC
	 * setting done. Default is 400Mhz (system_pll1_800m with div = 2)
	 * set by ROM for ND VDD_SOC
	 */
	if (IS_ENABLED(CONFIG_IMX8M_LPDDR4) && !IS_ENABLED(CONFIG_IMX8M_VDD_SOC_850MV)) {
		clock_enable(CCGR_GIC, 0);
		clock_set_target_val(GIC_CLK_ROOT, CLK_ROOT_ON |
				CLK_ROOT_SOURCE_SEL(5));
		clock_enable(CCGR_GIC, 1);
	}

	tq_bb_spl_board_init();

	if (tqma8mpxs_ram_timing_idx >= 0) {
#if CONFIG_IS_ENABLED(BLOBLIST)
		/*
		 * board_init_f runs before bloblist_init (board_init_r),
		 * so we need this here in spl_board_init
		 */
		struct tq_raminfo *raminfo_blob;

		raminfo_blob = bloblist_ensure(BLOBLISTT_TQ_RAMSIZE,
					       sizeof(*raminfo_blob));
		if (raminfo_blob && tqma8mpxs_ram_timing_idx >= 0) {
			if (IS_ENABLED(CONFIG_IMX8M_DRAM_INLINE_ECC)) {
				/*
				 * All 7 ECC protectable main memory regions (7/8 of main
				 * memory) are configured to be ECC protected with the
				 * shipped timings, so the top 1/8 of memory is used for
				 * ECC parity data.
				 *
				 * Hence, if inline ECC is used, only propagate 7/8 of total
				 * memory to U-Boot proper and Linux. Upper 1/8 is used for
				 * ECC parity data.
				 */
				raminfo_blob->memsize =
					(tqma8mpxs_dram_info[tqma8mpxs_ram_timing_idx].size /
					8ULL) *
					7ULL;
			} else {
				raminfo_blob->memsize =
					tqma8mpxs_dram_info[tqma8mpxs_ram_timing_idx].size;
			}
		}
#endif
	} else {
		printf("ERROR: no valid ram configuration, please reset\n");
		hang();
	}
}

#if CONFIG_IS_ENABLED(DM_PMIC_PCA9450)

#if defined(DEBUG)
static void print_pmic_config(struct udevice *dev)
{
	s32 regval;

	regval = pmic_reg_read(dev, PCA9450_BUCK123_DVS);
	printf("PMIC:  PCA9450_BUCK123_DVS=0x%02x\n", regval);
	regval = pmic_reg_read(dev, PCA9450_BUCK1OUT_DVS0);
	printf("PMIC:  PCA9450_BUCK1OUT_DVS0=0x%02x\n", regval);
	regval = pmic_reg_read(dev, PCA9450_BUCK1OUT_DVS1);
	printf("PMIC:  PCA9450_BUCK1OUT_DVS1=0x%02x\n", regval);
	regval = pmic_reg_read(dev, PCA9450_BUCK1CTRL);
	printf("PMIC:  PCA9450_BUCK1CTRL=0x%02x\n", regval);
	regval = pmic_reg_read(dev, PCA9450_BUCK2OUT_DVS0);
	printf("PMIC:  PCA9450_BUCK2OUT_DVS0=0x%02x\n", regval);
}
#else
static inline void print_pmic_config(struct udevice *dev) {}
#endif

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

	print_pmic_config(dev);

	/*
	 * BUCKxOUT_DVS0/1 control BUCK123 output
	 * clear PRESET_EN,
	 * BUCK voltage is determined by each BUCKxOUT_DVS0 or BUCKxOUT_DVS1
	 */
	pmic_reg_write(dev, PCA9450_BUCK123_DVS, 0x29);

	/*
	 * Increase VDD_SOC to typical value 0.95V before first
	 * DRAM access, set DVS1 to 0.85V for suspend.
	 * Enable DVS control through PMIC_STBY_REQ and
	 * set B1_ENMODE=1 (ON by PMIC_ON_REQ=H)
	 */
	if (IS_ENABLED(CONFIG_IMX8M_VDD_SOC_850MV))
		pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, 0x14);
	else
		pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, 0x1C);
	pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS1, 0x14);
	pmic_reg_write(dev, PCA9450_BUCK1CTRL, 0x59);

	/*
	 * Kernel uses OD/OD freq for SOC.
	 * To avoid timing risk from SOC to ARM,increase VDD_ARM to OD
	 * voltage 0.95V.
	 */
	pmic_reg_write(dev, PCA9450_BUCK2OUT_DVS0, 0x1C);

	print_pmic_config(dev);

	/*
	 * PCA9450_RESET_CTRL is initialized via PMIC driver,
	 * nxp,wdog_b-warm-reset or as cold reset per default
	 */

	return 0;
}
#endif

#include <dm/uclass-internal.h>

void init_console_clock(void)
{
#if defined(CFG_MXC_UART_BASE)
	switch (CFG_MXC_UART_BASE) {
	case UART_BASE_ADDR(1):
		init_uart_clk(0);
		break;
	case UART_BASE_ADDR(2):
		init_uart_clk(1);
		break;
	case UART_BASE_ADDR(3):
		init_uart_clk(2);
		break;
	case UART_BASE_ADDR(4):
		init_uart_clk(3);
		break;
	}
#endif
}

void board_init_f(ulong dummy)
{
	struct udevice *dev;
	int ramtype;
	int ret;

	/*
	 * Clearing the BSS here is not needed. Will be called by generic
	 * SPL code before board_init_r is called
	 */

	arch_cpu_init();

	board_early_init_f();

	timer_init();

	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	ret = uclass_get_device_by_name(UCLASS_CLK,
					"clock-controller@30380000",
					&dev);
	if (ret < 0) {
		printf("Failed to find clock node. Check device tree\n");
		hang();
	}

	preloader_console_init();
	enable_tzc380();

	power_init_board();

	/* DDR initialization */
	ramtype = handle_vard();
	tq_vard_show(&vard);
	spl_dram_init(ramtype);

	/* board_init_r will be called by generic SPL code */
}
