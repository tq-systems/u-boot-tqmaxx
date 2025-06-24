// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021-2025 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <env.h>
#include <errno.h>
#include <asm/io.h>
#include <asm-generic/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx8mp_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/iomux-v3.h>
#include <linux/delay.h>
#include <usb.h>

#include "../common/tq_board_gpio.h"
#include "../common/tq_som_features.h"

#define BB_BOARD_NAME "MB-SMARC-2"

DECLARE_GLOBAL_DATA_PTR;

enum {
	SLEEP,
	BATLOW_N,
	LID,
	CHARGING_N,
	CHG_PRSNT_N,

	GPIO0,
	GPIO1,
	GPIO2,
	GPIO3,
	GPIO4,
	GPIO5,
	GPIO6,
	GPIO7,
	GPIO8,
	GPIO9,
	GPIO10,
	GPIO11,
	GPIO12,
	GPIO13,

	LCD0_BLT_EN,
	LCD1_BLT_EN,
	DP_BRIDGE_EN,
};

#define UART_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_FSEL1)

static const iomux_v3_cfg_t uart_pads[] = {
	MX8MP_PAD_SD1_DATA6__UART3_DCE_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX8MP_PAD_SD1_DATA7__UART3_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

/*
 * NOTE: this is also used by SPL
 */
int tq_bb_board_early_init_f(void)
{
	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));
	init_uart_clk(2);

	return 0;
}

#if !defined(CONFIG_SPL_BUILD)

const char *tq_bb_get_boardname(void)
{
	return BB_BOARD_NAME;
}

static struct tq_gpio_init_data gpio_init_data[] = {
	GPIO_INIT_DATA_ENTRY(SLEEP, "GPIO1_0", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(BATLOW_N, "GPIO1_1", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(LID, "GPIO1_3", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(CHARGING_N, "GPIO1_6", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(CHG_PRSNT_N, "GPIO1_7", GPIOD_IS_IN),

	GPIO_INIT_DATA_ENTRY(GPIO0, "GPIO4_25", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(GPIO1, "GPIO4_26", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(GPIO2, "GPIO4_28", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(GPIO3, "GPIO3_20", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(GPIO4, "GPIO3_19", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(GPIO5, "GPIO5_5", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(GPIO6, "GPIO4_29", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(GPIO7, "GPIO4_16", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(GPIO8, "GPIO4_1", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(GPIO9, "GPIO4_4", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(GPIO10, "GPIO1_5", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(GPIO11, "GPIO5_11", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(GPIO12, "GPIO5_10", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(GPIO13, "GPIO5_13", GPIOD_IS_IN),

	GPIO_INIT_DATA_ENTRY(LCD0_BLT_EN, "GPIO@73_0", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(LCD1_BLT_EN, "GPIO@73_2", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(DP_BRIDGE_EN, "GPIO@73_4", GPIOD_IS_OUT),
};

/*
 * USDHC3 (devno 2, e-MMC) -> mmc0 / mmcblk0
 * USDHC2 (devno 1, SD) -> mmc1 / mmcblk1
 */
int board_mmc_get_env_dev(int devno)
{
	switch (devno) {
	case 2:
		return 0;
	case 1:
		return 1;
	default:
		printf("Error: USDHC%d not handled for environment\n", devno);
		return (int)env_get_ulong("mmcdev", 10, (ulong)CONFIG_SYS_MMC_ENV_DEV);
	}
}

/*
 * we use dt alias based indexing, so kernel uses same index. See above
 */
int mmc_map_to_kernel_blk(int devno)
{
	return devno;
}

int tq_bb_board_init(void)
{
	tq_board_gpio_init(gpio_init_data, ARRAY_SIZE(gpio_init_data));

#if (IS_ENABLED(CONFIG_USB))
	init_usb_clk();
#endif

	return 0;
}

#ifdef CONFIG_OF_BOARD_SETUP

int tq_bb_ft_board_setup(void *blob, struct bd_info *bis)
{
	const struct tq_som_feature_list *features;

	features = tq_board_detect_features();
	if (!features) {
		pr_warn("tq_board_detect_features failed\n");
		return -ENODATA;
	}

	tq_ft_fixup_features(blob, features);

	return 0;
}
#endif

int tq_bb_board_late_init(void)
{
	if (IS_ENABLED(CONFIG_ENV_IS_IN_MMC))
		board_late_mmc_env_init();

	return 0;
}

#endif
