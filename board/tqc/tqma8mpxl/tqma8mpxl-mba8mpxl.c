// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2021 TQ-Systems GmbH
 * Markus Niebel <Markus.Niebel@tq-group.com>
 */

#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm-generic/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx8mp_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/gpio.h>

#include "../common/tqc_board_gpio.h"

#define MBA8MPXL_BOARD_NAME "MBa8MPxL"

DECLARE_GLOBAL_DATA_PTR;

enum {
	USB_RST_B,
	USB_OTG_PWR,
	SWITCH_A,
	SWITCH_B,
	USER_LED0,
	USER_LED1,
	USER_LED2,
	GPOUT_0,
	GPOUT_1,
	GPOUT_2,
	GPOUT_3,
	GPIN_0,
	GPIN_1,
	GPIN_2,
	GPIN_3,
	LVDS_RESET_B,
	LVDS_BLT_EN,
	LVDS_PWR_EN,
	DP_IRQ,
	DP_EN,
	HDMI_OC,
};

#define UART_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_FSEL1)

static iomux_v3_cfg_t const uart_pads[] = {
	MX8MP_PAD_UART4_RXD__UART4_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX8MP_PAD_UART4_TXD__UART4_DCE_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

/*
 * NOTE: this is also used by SPL
 */
int tqc_bb_board_early_init_f(void)
{
	init_uart_clk(3);
	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));

	return 0;
}

#if !defined(CONFIG_SPL_BUILD)

const char *tqc_bb_get_boardname(void)
{
	return MBA8MPXL_BOARD_NAME;
}

int tqc_bb_checkboard(void)
{
	return 0;
}

static struct tqc_gpio_init_data mba8mpxl_gid[] = {
	GPIO_INIT_DATA_ENTRY(USB_RST_B, "GPIO1_11",
			     GPIOD_IS_OUT | GPIOD_ACTIVE_LOW |
			     GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(USB_OTG_PWR, "GPIO1_12", GPIOD_IS_OUT),

	GPIO_INIT_DATA_ENTRY(SWITCH_A, "GPIO5_26", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(SWITCH_B, "GPIO5_27", GPIOD_IS_IN),

	GPIO_INIT_DATA_ENTRY(USER_LED0, "GPIO5_5",
			     GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(USER_LED1, "GPIO5_4", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(USER_LED2, "GPIO5_3", GPIOD_IS_OUT),

	GPIO_INIT_DATA_ENTRY(GPOUT_0, "GPIO1_0", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(GPOUT_1, "GPIO1_1", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(GPOUT_2, "GPIO1_3", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(GPOUT_3, "GPIO1_6", GPIOD_IS_OUT),

	GPIO_INIT_DATA_ENTRY(GPIN_0, "GPIO1_7", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(GPIN_1, "GPIO1_9", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(GPIN_2, "GPIO1_14", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(GPIN_3, "GPIO1_15", GPIOD_IS_IN),

	GPIO_INIT_DATA_ENTRY(LVDS_RESET_B, "GPIO3_14",
			     GPIOD_IS_OUT | GPIOD_ACTIVE_LOW |
			     GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(LVDS_BLT_EN, "GPIO3_19", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(LVDS_PWR_EN, "GPIO3_20", GPIOD_IS_OUT),

	GPIO_INIT_DATA_ENTRY(DP_IRQ, "GPIO4_18", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(DP_EN, "GPIO4_19", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(HDMI_OC, "GPIO4_20", GPIOD_IS_IN),
};

static void setup_enet(void)
{
	struct iomuxc_gpr_base_regs *gpr =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

	if (CONFIG_IS_ENABLED(FEC_MXC)) {
		/* Enable RGMII TX clk output */
		setbits_le32(&gpr->gpr[1], BIT(22));
	}

	if (CONFIG_IS_ENABLED(DWC_ETH_QOS)) {
		/* set INTF as RGMII, enable RGMII TXC clock */
		clrsetbits_le32(&gpr->gpr[1],
				IOMUXC_GPR_GPR1_GPR_ENET_QOS_INTF_SEL_MASK,
				BIT(16));
		setbits_le32(&gpr->gpr[1], BIT(19) | BIT(21));
		set_clk_eqos(ENET_125MHZ);
	}
}

int tqc_bb_board_init(void)
{
	tqc_board_gpio_init(mba8mpxl_gid, ARRAY_SIZE(mba8mpxl_gid));

	setup_enet();

	return 0;
}

#ifdef CONFIG_OF_BOARD_SETUP
int tqc_bb_ft_board_setup(void *blob, bd_t *bd)
{
	return 0;
}
#endif

int tqc_bb_board_late_init(void)
{
#ifdef CONFIG_ENV_IS_IN_MMC
	board_late_mmc_env_init();
#endif

	return 0;
}

#endif
