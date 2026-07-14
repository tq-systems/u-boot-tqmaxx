// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel, Nora Schiffer
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

#include "../common/tq_bb.h"
#include "../common/tq_som_features.h"

#define MBA8MP_RAS314_BOARD_NAME "MBa8MP-RAS314"

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_FSEL1)

static const iomux_v3_cfg_t uart_pads[] = {
	MX8MP_PAD_UART4_RXD__UART4_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX8MP_PAD_UART4_TXD__UART4_DCE_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

/*
 * NOTE: this is also used by SPL
 */
int tq_bb_board_early_init_f(void)
{
	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));
	init_uart_clk(3);

	return 0;
}

#if !defined(CONFIG_SPL_BUILD)

const char *tq_bb_get_boardname(void)
{
	return MBA8MP_RAS314_BOARD_NAME;
}

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
		return (int)env_get_ulong("mmcdev", 10, -1L);
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
	return 0;
}

int tq_bb_ft_board_setup(void *blob, struct bd_info *bis)
{
	if (IS_ENABLED(CONFIG_OF_BOARD_SETUP)) {
		const struct tq_som_feature_list *features;

		features = tq_board_detect_features();
		if (!features) {
			pr_warn("tq_board_detect_features failed\n");
			return -ENODATA;
		}

		tq_ft_fixup_features(blob, features);
	}

	return 0;
}

int tq_bb_board_late_init(void)
{
	if (IS_ENABLED(CONFIG_ENV_IS_IN_MMC))
		board_late_mmc_env_init();

	tq_set_boot_targets();

	return 0;
}

#endif
