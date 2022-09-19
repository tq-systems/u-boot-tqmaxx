// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 - 2022 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <common.h>
#include <cpu_func.h>
#include <hang.h>
#include <spl.h>
#include <asm/io.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/arch/imx8mp_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <bloblist.h>
#include <power/pmic.h>

#include <power/pca9450.h>
#include <asm/arch/clock.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <fsl_esdhc_imx.h>
#include <mmc.h>
#include <asm/arch/ddr.h>

#include "../common/tqc_bb.h"
#include "../common/tqc_blob.h"
#include "../common/tqc_eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

/* SPL currently not working with DM, use old style I2C API */
#define TQC_SYSTEM_EEPROM_BUS	0
#define TQC_SYSTEM_EEPROM_ADDR	0x53

#if defined(CONFIG_IMX8M_DRAM_INLINE_ECC)
#error "Inline ECC is not supported yet"
#else

#if defined(CONFIG_TQMA8MPXL_RAM_1024MB)
extern struct dram_timing_info dram_timing_1gb_no_ecc;
#endif
#if defined(CONFIG_TQMA8MPXL_RAM_2048MB)
extern struct dram_timing_info dram_timing_2gb_no_ecc;
#endif
#if defined(CONFIG_TQMA8MPXL_RAM_4096MB)
extern struct dram_timing_info dram_timing_4gb_no_ecc;
#endif
#if defined(CONFIG_TQMA8MPXL_RAM_8192MB)
extern struct dram_timing_info dram_timing_8gb_no_ecc;
#endif

struct dram_info {
	struct dram_timing_info	*table;
	phys_size_t		size;
};

/*
 * Currently this is known to work with up to four timings in one SPL
 * Needs to be rechecked when increasing
 */
static struct dram_info tqma8mpxl_dram_info[]  = {
#if defined(CONFIG_TQMA8MPXL_RAM_1024MB)
	{ &dram_timing_1gb_no_ecc, SZ_1G * 1ULL },
#endif
#if defined(CONFIG_TQMA8MPXL_RAM_2048MB)
	{ &dram_timing_2gb_no_ecc, SZ_1G * 2ULL },
#endif
#if defined(CONFIG_TQMA8MPXL_RAM_4096MB)
	{ &dram_timing_4gb_no_ecc, SZ_1G * 4ULL },
#endif
#if defined(CONFIG_TQMA8MPXL_RAM_8192MB)
	{ &dram_timing_8gb_no_ecc, SZ_1G * 8ULL },
#endif
};

static int tqma8mpxl_ram_timing_idx = -1;

#endif

static struct tq_vard vard;

static int handle_vard(void)
{
	if (tq_vard_read(TQC_SYSTEM_EEPROM_BUS, TQC_SYSTEM_EEPROM_ADDR, &vard))
		puts("ERROR: vard read\n");
	else if (!tq_vard_valid(&vard))
		puts("ERROR: vard CRC\n");
	else
		return (int)(vard.memtype & VARD_MEMTYPE_MASK_TYPE);

	return VARD_MEMTYPE_DEFAULT;
};

#if defined(CONFIG_TQMA8MPXL_RAM_MULTI)
static int tqma8mpxl_query_ddr_timing(void)
{
	char sel = '-';
	phys_size_t ramsize;
	int idx;

	puts("Warning: no valid EEPROM!\n"
		"Please enter LPDDR size in GByte to procced.\n"
		"Valid sizes are 1,2,4 and 8.\n");

	for (;;) {
		/* Flush input */
		while (tstc())
			getc();

		sel = getc();
		putc('\n');

		if ((sel == '1') || (sel == '2') || (sel == '4') || (sel == '8'))
			break;
		else
			puts("Please enter a valid size.\n");
	}

	ramsize = (phys_size_t)((unsigned int)sel - (unsigned int)('0')) * SZ_1G;
	for (idx = 0; idx < ARRAY_SIZE(tqma8mpxl_dram_info); ++idx) {
		if (ramsize == tqma8mpxl_dram_info[idx].size)
			break;
	}

	if (idx >= ARRAY_SIZE(tqma8mpxl_dram_info) ||
	    !tqma8mpxl_dram_info[idx].table) {
		puts("ERROR: no valid RAM timing or timing not in image, stop\n");
		hang();
	}

	return idx;
}
#endif

static void spl_dram_init(int memtype)
{
	int idx = -1;

	/* normal configuration */
#if defined(CONFIG_TQMA8MPXL_RAM_MULTI)
	phys_size_t ramsize;

	if (memtype == 1) {
		ramsize = tq_vard_ramsize(&vard);
		for (idx = 0; idx < ARRAY_SIZE(tqma8mpxl_dram_info); ++idx) {
			if (ramsize == tqma8mpxl_dram_info[idx].size)
				break;
		}
		if (idx >= ARRAY_SIZE(tqma8mpxl_dram_info))
			puts("DRAM init: no matching timing\n");
	} else if (vard.memtype == VARD_MEMTYPE_DEFAULT) {
		puts("DRAM init: vard invalid?\n");
	} else {
		printf("DRAM init: unknown RAM type %u\n",
			(unsigned int)vard.memtype);
	}

	if (idx < 0 || idx >= ARRAY_SIZE(tqma8mpxl_dram_info))
		idx = tqma8mpxl_query_ddr_timing();
	/* REV.020x prototypes without variant data in EEPROM */
#elif defined(CONFIG_TQMA8MPXL_RAM_SINGLE_2GB)
	for (idx = 0; idx < ARRAY_SIZE(tqma8mpxl_dram_info); ++idx) {
		if ((tqma8mpxl_dram_info[idx].size == 2ULL * SZ_1G) &&
		    tqma8mpxl_dram_info[idx].table)
			break;
	}
#else
#error "missing or invalid RAM config"
#endif

	if (idx < 0 || idx >= ARRAY_SIZE(tqma8mpxl_dram_info)) {
		printf("ERROR: no valid ram configuration, please reset\n");
		hang();
	}

	ddr_init(tqma8mpxl_dram_info[idx].table);
	tqma8mpxl_ram_timing_idx = idx;
}

#ifdef CONFIG_SPL_MMC_SUPPORT

#define USDHC_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE | \
			 PAD_CTL_PE | PAD_CTL_FSEL2)

static iomux_v3_cfg_t const usdhc3_pads[] = {
	MX8MP_PAD_NAND_WE_B__USDHC3_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_WP_B__USDHC3_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_DATA04__USDHC3_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_DATA05__USDHC3_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_DATA06__USDHC3_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_DATA07__USDHC3_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_RE_B__USDHC3_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_CE2_B__USDHC3_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_CE3_B__USDHC3_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_CLE__USDHC3_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
};

static struct fsl_esdhc_cfg usdhc_cfg = {
	USDHC3_BASE_ADDR, 0, 8,
};

int board_mmc_init(bd_t *bis)
{
	int ret;
	/*
	 * According to the board_mmc_init() the following map is done:
	 * (U-Boot device node)    (Physical Port)
	 * mmc1                    USDHC1
	 * mmc0                    USDHC3 (e-MMC)
	 */
	init_clk_usdhc(2);
	usdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
	imx_iomux_v3_setup_multiple_pads(usdhc3_pads, ARRAY_SIZE(usdhc3_pads));

	ret = fsl_esdhc_initialize(bis, &usdhc_cfg);
	if (ret)
		return ret;

	return tqc_bb_board_mmc_init(bis);
}

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC3_BASE_ADDR:
		ret = 1;
		break;
	default:
		ret = tqc_bb_board_mmc_getcd(mmc);
	}

	return ret;
}

int board_mmc_getwp(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 1;

	switch (cfg->esdhc_base) {
	case USDHC3_BASE_ADDR:
		ret = 0;
		break;
	default:
		ret = tqc_bb_board_mmc_getwp(mmc);
	}

	return ret;
}

#define UBOOT_RAW_SECTOR_OFFSET 0x40
unsigned long spl_mmc_get_uboot_raw_sector(struct mmc *mmc)
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

#endif /* CONFIG_SPL_MMC_SUPPORT */

#define I2C_PAD_CTRL (PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE | PAD_CTL_PE)
#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)
struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode = MX8MP_PAD_I2C1_SCL__I2C1_SCL | PC,
		.gpio_mode = MX8MP_PAD_I2C1_SCL__GPIO5_IO14 | PC,
		.gp = IMX_GPIO_NR(5, 14),
	},
	.sda = {
		.i2c_mode = MX8MP_PAD_I2C1_SDA__I2C1_SDA | PC,
		.gpio_mode = MX8MP_PAD_I2C1_SDA__GPIO5_IO15 | PC,
		.gp = IMX_GPIO_NR(5, 15),
	},
};

#ifdef CONFIG_POWER

#if defined(DEBUG)
static void print_pmic_config(struct pmic *p)
{
	u32 regval;

	pmic_reg_read(p, PCA9450_BUCK123_DVS, &regval);
	printf("PMIC:  PCA9450_BUCK123_DVS=0x%02x\n", regval);
	pmic_reg_read(p, PCA9450_BUCK1OUT_DVS0, &regval);
	printf("PMIC:  PCA9450_BUCK1OUT_DVS0=0x%02x\n", regval);
	pmic_reg_read(p, PCA9450_BUCK1OUT_DVS1, &regval);
	printf("PMIC:  PCA9450_BUCK1OUT_DVS1=0x%02x\n", regval);
	pmic_reg_read(p, PCA9450_BUCK1CTRL, &regval);
	printf("PMIC:  PCA9450_BUCK1CTRL=0x%02x\n", regval);
	pmic_reg_read(p, PCA9450_BUCK2OUT_DVS0, &regval);
	printf("PMIC:  PCA9450_BUCK2OUT_DVS0=0x%02x\n", regval);
}
#else
static inline void print_pmic_config(struct pmic *p) {}
#endif

#define I2C_PMIC	0

int power_init_board(void)
{
	struct pmic *p;
	int ret;
	u32 regval;

	ret = power_pca9450b_init(I2C_PMIC);
	if (ret)
		printf("power init failed");
	p = pmic_get("PCA9450");
	pmic_probe(p);

	pmic_reg_read(p, PCA9450_REG_DEV_ID, &regval);
	printf("PMIC:  PCA9450 ID=0x%02x\n", regval);

	print_pmic_config(p);
	/*
	 * BUCKxOUT_DVS0/1 control BUCK123 output
	 * clear PRESET_EN,
	 * BUCK voltage is determined by each BUCKxOUT_DVS0 or BUCKxOUT_DVS1
	 */
	pmic_reg_write(p, PCA9450_BUCK123_DVS, 0x29);

	/*
	 * increase VDD_SOC to typical value 0.95V before first
	 * DRAM access, set DVS1 to 0.85v for suspend.
	 * Enable DVS control through PMIC_STBY_REQ and
	 * set B1_ENMODE=1 (ON by PMIC_ON_REQ=H)
	 */
	pmic_reg_write(p, PCA9450_BUCK1OUT_DVS0, 0x1C);
	pmic_reg_write(p, PCA9450_BUCK1OUT_DVS1, 0x14);
	pmic_reg_write(p, PCA9450_BUCK1CTRL, 0x59);

	/*
	 * Kernel uses OD/OD freq for SOC
	 * To avoid timing risk from SOC to ARM,increase VDD_ARM to OD
	 * voltage 0.95v
	 */
	pmic_reg_write(p, PCA9450_BUCK2OUT_DVS0, 0x1C);

	print_pmic_config(p);

	/*
	 * set WDOG_B_CFG to cold reset w/o LDO1/2
	 * per default PMIC_RST_CFG is also cold reset w/o LDO1/2
	 * T_PMIC_RST_DEB is configured for 50 msec
	 */
	pmic_reg_read(p, PCA9450_RESET_CTRL, &regval);
	regval &= 0x3f;
	regval |= 0x80;
	pmic_reg_write(p, PCA9450_RESET_CTRL, regval);

	return 0;
}

#else
# error "please define CONFIG_POWER"
#endif

void spl_board_init(void)
{
	/*
	 * board_init_f runs before bloblist_init, so we need this here in in
	 * spl_board_init
	 */
	struct tq_raminfo *raminfo_blob;

	/*
	 * Set GIC clock to 500Mhz for OD VDD_SOC. Kernel driver does not allow
	 * to change it. Should set the clock after PMIC setting done.
	 * Default is 400Mhz (system_pll1_800m with div = 2) set by ROM for
	 * ND VDD_SOC
	 */
#ifdef CONFIG_IMX8M_LPDDR4
	clock_enable(CCGR_GIC, 0);
	clock_set_target_val(GIC_CLK_ROOT, CLK_ROOT_ON |
			     CLK_ROOT_SOURCE_SEL(5));
	clock_enable(CCGR_GIC, 1);
#endif
	tqc_bb_spl_board_init();

	raminfo_blob = bloblist_ensure(BLOBLISTT_TQ_RAMSIZE,
				       sizeof(*raminfo_blob));
	if (raminfo_blob && tqma8mpxl_ram_timing_idx >= 0) {
		raminfo_blob->memsize =
			tqma8mpxl_dram_info[tqma8mpxl_ram_timing_idx].size;
	} else {
		printf("ERROR: no valid ram configuration, please reset\n");
		hang();
	}

	puts("Normal Boot\n");
}

void board_init_f(ulong dummy)
{
	int ret;
	int ramtype;

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	arch_cpu_init();

	board_early_init_f();

	timer_init();

	preloader_console_init();

	ret = spl_init();
	if (ret) {
		debug("spl_init() failed: %d\n", ret);
		hang();
	}

	enable_tzc380();

	/* Adjust pmic voltage to 1.0V for 800M */
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);
	power_init_board();

	/* DDR initialization */
	ramtype = handle_vard();
	tq_vard_show(&vard);
	spl_dram_init(ramtype);

	board_init_r(NULL, 0);
}

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	puts("resetting ...\n");

	reset_cpu(WDOG1_BASE_ADDR);

	return 0;
}
