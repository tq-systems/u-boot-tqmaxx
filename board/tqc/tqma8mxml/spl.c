// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2020 - 2021 TQ-Systems GmbH
 * Martin Schmiedel <Martin.Schmiedel@tq-group.com>
 */

#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/ddr.h>
#ifdef CONFIG_IMX8MN
#include <asm/arch/imx8mn_pins.h>
#elif defined(CONFIG_IMX8MM)
#include <asm/arch/imx8mm_pins.h>
#else
#error
#endif
#include <asm/arch/sys_proto.h>
#include <asm/arch/clock.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <fsl_esdhc_imx.h>
#include <hang.h>
#include <mmc.h>
#include <power/pmic.h>
#ifdef CONFIG_POWER_PCA9450
#include <power/pca9450.h>
#endif
#include <spl.h>

#include "../common/tqc_bb.h"

DECLARE_GLOBAL_DATA_PTR;

#define USDHC_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE | \
			 PAD_CTL_PE | PAD_CTL_FSEL2)

#define USDHC_GPIO_PAD_CTRL (PAD_CTL_HYS | PAD_CTL_DSE1)

#define EMMC_BASE_ADDR	USDHC3_BASE_ADDR
#define EMMC_PWR_GPIO	IMX_GPIO_NR(3, 16)
#define EMMC_CLK	MXC_ESDHC3_CLK
#define EMMC_IDX	2

#if defined(CONFIG_IMX8MN)

#define I2C_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE | PAD_CTL_PE)
static struct i2c_pads_info i2c_pad_info = {
	.scl = {
		.i2c_mode = IMX8MN_PAD_I2C1_SCL__I2C1_SCL | MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = IMX8MN_PAD_I2C1_SCL__GPIO5_IO14 | MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(5, 14),
	},
	.sda = {
		.i2c_mode = IMX8MN_PAD_I2C1_SDA__I2C1_SDA | MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = IMX8MN_PAD_I2C1_SDA__GPIO5_IO15 | MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(5, 15),
	},
};

static iomux_v3_cfg_t const emmc_pads[] = {
	IMX8MN_PAD_NAND_WE_B__USDHC3_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MN_PAD_NAND_WP_B__USDHC3_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MN_PAD_NAND_DATA04__USDHC3_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MN_PAD_NAND_DATA05__USDHC3_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MN_PAD_NAND_DATA06__USDHC3_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MN_PAD_NAND_DATA07__USDHC3_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MN_PAD_NAND_RE_B__USDHC3_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MN_PAD_NAND_CE2_B__USDHC3_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MN_PAD_NAND_CE3_B__USDHC3_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MN_PAD_NAND_CLE__USDHC3_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MN_PAD_NAND_READY_B__GPIO3_IO16 | MUX_PAD_CTRL(USDHC_GPIO_PAD_CTRL),
	IMX8MN_PAD_NAND_CE1_B__USDHC3_STROBE | MUX_PAD_CTRL(USDHC_GPIO_PAD_CTRL),
};

extern struct dram_timing_info tqma8mxnl_1gb_lpddr4_timing;
static struct dram_timing_info *default_dram_timing = &tqma8mxnl_1gb_lpddr4_timing;

#elif defined(CONFIG_IMX8MM)

/* TODO: check if this is correct */
#define I2C_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS)
static struct i2c_pads_info i2c_pad_info = {
	.scl = {
		.i2c_mode = IMX8MM_PAD_I2C1_SCL_I2C1_SCL | MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = IMX8MM_PAD_I2C1_SCL_GPIO5_IO14 | MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(5, 14),
	},
	.sda = {
		.i2c_mode = IMX8MM_PAD_I2C1_SDA_I2C1_SDA | MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = IMX8MM_PAD_I2C1_SDA_GPIO5_IO15 | MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(5, 15),
	},
};

static iomux_v3_cfg_t const emmc_pads[] = {
	IMX8MM_PAD_NAND_WE_B_USDHC3_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_NAND_WP_B_USDHC3_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_NAND_DATA04_USDHC3_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_NAND_DATA05_USDHC3_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_NAND_DATA06_USDHC3_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_NAND_DATA07_USDHC3_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_NAND_RE_B_USDHC3_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_NAND_CE2_B_USDHC3_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_NAND_CE3_B_USDHC3_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_NAND_CLE_USDHC3_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_NAND_READY_B_GPIO3_IO16 | MUX_PAD_CTRL(USDHC_GPIO_PAD_CTRL),
	IMX8MM_PAD_NAND_CE1_B_USDHC3_STROBE | MUX_PAD_CTRL(USDHC_GPIO_PAD_CTRL),
};

#if defined(CONFIG_TQMA8MMX_RAM_2048MB)

extern struct dram_timing_info tqma8mxml_2gb_dram_timing;
static struct dram_timing_info *default_dram_timing = &tqma8mxml_2gb_dram_timing;

#elif defined(CONFIG_TQMA8MMX_RAM_1024MB)

extern struct dram_timing_info tqma8mxml_1gb_dram_timing;
static struct dram_timing_info *default_dram_timing = &tqma8mxml_1gb_dram_timing;

#else
#error
#endif

#else
#error
#endif

static void spl_dram_init(void)
{
	ddr_init(default_dram_timing);
};

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case EMMC_BASE_ADDR:
		ret = 1;
		break;
	default:
		ret = tqc_bb_board_mmc_getcd(mmc);
	}

	return ret;
}

static struct fsl_esdhc_cfg emmc_cfg = {
	.esdhc_base = EMMC_BASE_ADDR,
	.max_bus_width = 8,
};

int board_mmc_init(bd_t *bis)
{
	int ret;

	debug("%s\n", __func__);
	/*
	 * According to the board_mmc_init() the following map is done:
	 * (U-Boot device node)    (Physical Port)
	 * mmc0                    USDHC3 (REV.0200)
	 * mmc1                    USDHC2
	 */
	init_clk_usdhc(EMMC_IDX);
	emmc_cfg.sdhc_clk = mxc_get_clock(EMMC_CLK);
	imx_iomux_v3_setup_multiple_pads(emmc_pads, ARRAY_SIZE(emmc_pads));
	gpio_request(EMMC_PWR_GPIO, "emmc_reset");
	gpio_direction_output(EMMC_PWR_GPIO, 0);
	udelay(500);
	gpio_direction_output(EMMC_PWR_GPIO, 1);
	ret = fsl_esdhc_initialize(bis, &emmc_cfg);
	if (ret)
		return ret;

	ret = tqc_bb_board_mmc_init(bis);

	return ret;
}

#if defined(CONFIG_POWER)

#define I2C_PMIC	0

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
	pmic_reg_read(p, PCA9450_BUCK3OUT_DVS0, &regval);
	printf("PMIC:  PCA9450_BUCK3OUT_DVS0=0x%02x\n", regval);
	pmic_reg_read(p, PCA9450_BUCK3OUT_DVS1, &regval);
	printf("PMIC:  PCA9450_BUCK3OUT_DVS1=0x%02x\n", regval);
	pmic_reg_read(p, PCA9450_BUCK3CTRL, &regval);
	printf("PMIC:  PCA9450_BUCK3CTRL=0x%02x\n", regval);
	pmic_reg_read(p, PCA9450_LDO2CTRL, &regval);
	printf("PMIC:  PCA9450_LDO2CTRL=0x%02x\n", regval);
	pmic_reg_read(p, PCA9450_RESET_CTRL, &regval);
	printf("PMIC:  PCA9450_RESET_CTRL=0x%02x\n", regval);
}
#else
static inline void print_pmic_config(struct pmic *p) {}
#endif

#if defined(CONFIG_POWER_PCA9450)
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
#if defined(CONFIG_IMX8MN)

#ifdef CONFIG_IMX8MN_LOW_DRIVE_MODE
#error "not tested"
	/* Set VDD_SOC to 0.8v for low drive mode */
	pmic_reg_write(p, PCA9450_BUCK1OUT_DVS0, 0x10);
	pmic_reg_write(p, PCA9450_BUCK3OUT_DVS0, 0x10);
#else
	/*
	 * DVS0 @ PMIC_STBY_REQ = L
	 * increase BUCK1 DVS0 (VDD_SOC) to typical value 0.95V
	 * increase BUCK3 DVS0 (VDD_GPU / DRAM) to typical value 0.95V
	 * before first DRAM access
	 * Datasheet 3.1.3: VDD_DRAM, VDD_SOC, and VDD_GPU are required
	 * to be tied together and keep same or ground.
	 */
	pmic_reg_write(p, PCA9450_BUCK1OUT_DVS0, 0x1C);
	pmic_reg_write(p, PCA9450_BUCK3OUT_DVS0, 0x1C);
#endif
	/*
	 * DVS1 @ PMIC_STBY_REQ = H
	 * set BUCK1 DVS1 (VDD_SOC) to 0.85V for suspend
	 * set BUCK3 DVS1 (VDD_GPU / DRAM) to 0.85V for suspend
	 */
	pmic_reg_write(p, PCA9450_BUCK1OUT_DVS1, 0x14);
	pmic_reg_write(p, PCA9450_BUCK3OUT_DVS1, 0x14);
	/*
	 * DVS_CTRL=1 (Enable DVS control through PMIC_STBY_REQ)
	 * Bx_ENMODE=1 (ON by PMIC_ON_REQ=H)
	 */
	pmic_reg_write(p, PCA9450_BUCK1CTRL, 0x59);
	pmic_reg_write(p, PCA9450_BUCK3CTRL, 0x59);
	/* TODO: BUCK2 DVS CTRL, see PCA9450DS.pdf, 7.3.7 STANDBY */

	/* set VDD_SNVS_0V8 to 0.8V */
	pmic_reg_write(p, PCA9450_LDO2CTRL, 0xC0);
#elif defined(CONFIG_IMX8MM)
	/*
	 * DVS0 @ PMIC_STBY_REQ = L
	 * increase BUCK1 DVS0 (VDD_SOC) to typical value 0.85V with PCIe
	 * increase BUCK3 DVS0 (VDD_GPU / VPU / DRAM) to typical value 0.95V
	 * before first DRAM access
	 * Datasheet 3.1.3: VDD_DRAM, VDD_VPU, and VDD_GPU for overdrive mode
	 */
	pmic_reg_write(p, PCA9450_BUCK1OUT_DVS0, 0x14);
	pmic_reg_write(p, PCA9450_BUCK3OUT_DVS0, 0x1C);
	/*
	 * DVS1 @ PMIC_STBY_REQ = H
	 * set BUCK1 DVS1 (VDD_SOC) to 0.85V for suspend
	 * set BUCK3 DVS1 (VDD_GPU / VDD_VPU / DRAM) to 0.85V for suspend
	 */
	pmic_reg_write(p, PCA9450_BUCK1OUT_DVS1, 0x14);
	pmic_reg_write(p, PCA9450_BUCK3OUT_DVS1, 0x14);
	/*
	 * DVS_CTRL=1 (Enable DVS control through PMIC_STBY_REQ)
	 * Bx_ENMODE=1 (ON by PMIC_ON_REQ=H)
	 */
	pmic_reg_write(p, PCA9450_BUCK1CTRL, 0x59);
	pmic_reg_write(p, PCA9450_BUCK3CTRL, 0x59);
	/* TODO: BUCK2 DVS CTRL, see PCA9450DS.pdf, 7.3.7 STANDBY */

	/* set VDD_SNVS_0V8 to 0.8V */
	pmic_reg_write(p, PCA9450_LDO2CTRL, 0xC0);
#else
# error
#endif

	/* set WDOG_B_CFG to cold reset w/o LDO1/2 */
	pmic_reg_read(p, PCA9450_RESET_CTRL, &regval);
	regval &= 0x3f;
	regval |= 0x80;
	pmic_reg_write(p, PCA9450_RESET_CTRL, regval);

	print_pmic_config(p);

	return 0;
}

#endif /* CONFIG_POWER_PCA9450 */

#else
# error "please define CONFIG_POWER"
#endif

void spl_board_init(void)
{
#ifndef CONFIG_SPL_USB_SDP_SUPPORT
	/* Serial download mode */
	if (is_usb_boot()) {
		puts("Back to ROM, SDP\n");
		restore_boot_params();
	}
#endif

	tqc_bb_spl_board_init();

	puts("Normal Boot\n");
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* Just empty function now - can't decide what to choose */
	debug("%s: %s\n", __func__, name);

	return 0;
}
#endif

/*
 * TODO: to use DM and device tree, need to call spl_init()
 */
void board_init_f(ulong dummy)
{
	int ret;

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	arch_cpu_init();

	board_early_init_f();

	timer_init();
	/* do it here to have afterwards the uart configured */
	tqc_bb_board_init_f(dummy);

	preloader_console_init();

	ret = spl_init();
	if (ret) {
		debug("spl_init() failed: %d\n", ret);
		hang();
	}

	enable_tzc380();

	/* TODO: Adjust pmic voltages - really needed ? */
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info);
	power_init_board();

	/* DDR initialization */
	spl_dram_init();

	board_init_r(NULL, 0);
}
