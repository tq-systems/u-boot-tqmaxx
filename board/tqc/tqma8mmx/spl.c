// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 TQ Systems GmbH
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
#include <fsl_esdhc.h>
#include <mmc.h>
#include <power/pmic.h>
#ifdef CONFIG_POWER_PCA9450
#include <power/pca9450.h>
#endif
#include <spl.h>

#include "../common/tqc_bb.h"

DECLARE_GLOBAL_DATA_PTR;


#define USDHC1_PWR_GPIO IMX_GPIO_NR(2, 10)

#define USDHC_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE |PAD_CTL_PE | \
			 PAD_CTL_FSEL2)

#define USDHC_GPIO_PAD_CTRL (PAD_CTL_HYS | PAD_CTL_DSE1)


#ifdef CONFIG_IMX8MN

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

static iomux_v3_cfg_t const usdhc1_pads[] = {
	IMX8MN_PAD_SD1_CLK__USDHC1_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MN_PAD_SD1_CMD__USDHC1_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MN_PAD_SD1_DATA0__USDHC1_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MN_PAD_SD1_DATA1__USDHC1_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MN_PAD_SD1_DATA2__USDHC1_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MN_PAD_SD1_DATA3__USDHC1_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MN_PAD_SD1_DATA4__USDHC1_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MN_PAD_SD1_DATA5__USDHC1_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MN_PAD_SD1_DATA6__USDHC1_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MN_PAD_SD1_DATA7__USDHC1_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MN_PAD_SD1_RESET_B__GPIO2_IO10 | MUX_PAD_CTRL(USDHC_GPIO_PAD_CTRL),
	IMX8MN_PAD_SD1_STROBE__USDHC1_STROBE | MUX_PAD_CTRL(USDHC_GPIO_PAD_CTRL),
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

static iomux_v3_cfg_t const usdhc1_pads[] = {
	IMX8MM_PAD_SD1_CLK_USDHC1_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_SD1_CMD_USDHC1_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_SD1_DATA0_USDHC1_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_SD1_DATA1_USDHC1_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_SD1_DATA2_USDHC1_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_SD1_DATA3_USDHC1_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_SD1_DATA4_USDHC1_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_SD1_DATA5_USDHC1_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_SD1_DATA6_USDHC1_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_SD1_DATA7_USDHC1_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MM_PAD_SD1_RESET_B_GPIO2_IO10 | MUX_PAD_CTRL(USDHC_GPIO_PAD_CTRL),
	IMX8MM_PAD_SD1_STROBE_USDHC1_STROBE | MUX_PAD_CTRL(USDHC_GPIO_PAD_CTRL),
};

extern struct dram_timing_info tqma8mxml_1gb_dram_timing;

static struct dram_timing_info *default_dram_timing = &tqma8mxml_1gb_dram_timing;

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
	case USDHC1_BASE_ADDR:
		ret = 1;
		break;
	default:
		ret = tqc_bb_board_mmc_getcd(mmc);
	}

	return ret;
}



static struct fsl_esdhc_cfg usdhc1_cfg = {
	.esdhc_base = USDHC1_BASE_ADDR,
	.max_bus_width = 8,
};

int board_mmc_init(bd_t *bis)
{
	int ret;

	debug("board_mmc_init\n");
	/*
	 * According to the board_mmc_init() the following map is done:
	 * (U-Boot device node)    (Physical Port)
	 * mmc0                    USDHC1
	 * mmc1                    USDHC2
	 */
	init_clk_usdhc(0);
	usdhc1_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
	imx_iomux_v3_setup_multiple_pads(
		usdhc1_pads, ARRAY_SIZE(usdhc1_pads));
	gpio_request(USDHC1_PWR_GPIO, "usdhc1_reset");
	gpio_direction_output(USDHC1_PWR_GPIO, 0);
	udelay(500);
	gpio_direction_output(USDHC1_PWR_GPIO, 1);
	ret = fsl_esdhc_initialize(bis, &usdhc1_cfg);
	if (ret)
		return ret;

	ret = tqc_bb_board_mmc_init(bis);

	return ret;
}

#if defined(CONFIG_POWER)

#define I2C_PMIC	0

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

	/*
	 * TODO:
	 * check DVS for BUCK (power save with PMIC_STBY_REQ)
	 * check VDD_SOC/dRAM -> 0.95 Volt
	 * check VDD_SNVS_0V8 -> 0.85V
	 * see imx8m[m,n]_evk
	 */

	/* set WDOG_B_CFG to cold reset w/o LDO1/2*/
	pmic_reg_read(p, PCA9450_RESET_CTRL, &regval);
	regval &= 0x3f;
	regval |= 0x80;
	pmic_reg_write(p, PCA9450_RESET_CTRL, regval);

	return 0;
}

#endif /* CONFIG_POWER_PCA9450 */

#endif /* CONFIG_POWER */

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
