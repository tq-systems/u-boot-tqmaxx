// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021-2025 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Martin Schmiedel
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
#include "../common/tq_board_gpio.h"
#include "../common/tq_som_features.h"

#define MBA8MP_RAS314_BOARD_NAME "MBa8MP-RAS314"

DECLARE_GLOBAL_DATA_PTR;

enum {
	LVDS_RESET_B,
	HDMI_OC,
	PMIC_IRQ,
	ENET0_INT,
	ENET1_INT,
	TEMP_EVENT,
	RTC_EVENT,
	CODEC_RST,
	CAM_GPIO1,
	CAM_GPIO2,
};

#define UART_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_FSEL1)

static iomux_v3_cfg_t const uart_pads[] = {
	MX8MP_PAD_UART4_RXD__UART4_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX8MP_PAD_UART4_TXD__UART4_DCE_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

/*
 * NOTE: this is also used by SPL
 */
int tq_bb_board_early_init_f(void)
{
	init_uart_clk(3);
	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));

	return 0;
}

#if !defined(CONFIG_SPL_BUILD)

const char *tq_bb_get_boardname(void)
{
	return MBA8MP_RAS314_BOARD_NAME;
}

static struct tq_gpio_init_data mba8mpxl_gid[] = {
	GPIO_INIT_DATA_ENTRY(LVDS_RESET_B, "GPIO1_01",
			     GPIOD_IS_OUT | GPIOD_ACTIVE_LOW |
			     GPIOD_IS_OUT_ACTIVE),

	GPIO_INIT_DATA_ENTRY(HDMI_OC, "GPIO4_20", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(PMIC_IRQ, "GPIO1_8", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(ENET0_INT, "GPIO4_1", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(ENET1_INT, "GPIO4_3", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(TEMP_EVENT, "GPIO3_20", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(RTC_EVENT, "GPIO3_19", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(CODEC_RST, "GPIO5_11", GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(CAM_GPIO1, "GPIO2_6", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(CAM_GPIO2, "GPIO2_7", GPIOD_IS_OUT),
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

#define FSL_SIP_GPC			0xC2000000
#define FSL_SIP_CONFIG_GPC_PM_DOMAIN	0x3
#define DISPMIX				13
#define MIPI				15

int tq_bb_board_init(void)
{
	tq_board_gpio_init(mba8mpxl_gid, ARRAY_SIZE(mba8mpxl_gid));

#if (IS_ENABLED(CONFIG_USB))
	init_usb_clk();
#endif

	if (CONFIG_IS_ENABLED(TQMA8MPXL_LVDS)) {
		/* enable the dispmix & mipi phy power domain */
		call_imx_sip(FSL_SIP_GPC, FSL_SIP_CONFIG_GPC_PM_DOMAIN, DISPMIX, true, 0);
		call_imx_sip(FSL_SIP_GPC, FSL_SIP_CONFIG_GPC_PM_DOMAIN, MIPI, true, 0);
	}

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
#ifdef CONFIG_ENV_IS_IN_MMC
	board_late_mmc_env_init();
#endif
	tq_set_boot_targets();

	return 0;
}

#endif
