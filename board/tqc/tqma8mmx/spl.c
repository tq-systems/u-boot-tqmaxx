// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 TQ Systems GmbH
 */

#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/ddr.h>
#include <asm/arch/imx8mm_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/clock.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/gpio.h>
#include <fsl_esdhc.h>
#include <mmc.h>
#include <spl.h>

#include "../common/tqc_bb.h"

DECLARE_GLOBAL_DATA_PTR;

extern struct dram_timing_info tqma8mxxl_512mb_dram_timing;

static void spl_dram_init(void)
{
	ddr_init(&tqma8mxxl_512mb_dram_timing);
}


#define USDHC1_PWR_GPIO IMX_GPIO_NR(2, 10)

#define USDHC_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE |PAD_CTL_PE | \
			 PAD_CTL_FSEL2)
#define USDHC_GPIO_PAD_CTRL (PAD_CTL_HYS | PAD_CTL_DSE1 )

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

	/* DDR initialization */
	spl_dram_init();

	board_init_r(NULL, 0);
}
