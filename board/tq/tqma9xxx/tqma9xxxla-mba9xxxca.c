// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <common.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <errno.h>
#include <i2c.h>
#include <asm/io.h>
#include <asm-generic/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch-imx9/ccm_regs.h>
#include <asm/arch-imx9/imx93_pins.h>
#include <asm/mach-imx/iomux-v3.h>

#include "../common/tq_bb.h"
#include "../common/tq_board_gpio.h"

DECLARE_GLOBAL_DATA_PTR;

#define MBA9XXXCA_BOARD_NAME "MBa9xxxCA"

#define UART_PAD_CTRL	(PAD_CTL_DSE(6) | PAD_CTL_FSEL2)

static const iomux_v3_cfg_t uart_pads[] = {
	MX93_PAD_UART1_RXD__LPUART1_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX93_PAD_UART1_TXD__LPUART1_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

/*
 * NOTE: this is also used by SPL
 */
int tq_bb_board_early_init_f(void)
{
	init_uart_clk(LPUART1_CLK_ROOT);
	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));

	return 0;
}

#if !defined(CONFIG_SPL_BUILD)

enum {
	/* expander @70 */
	FAN_PWR_EN,
	MPCIE_WAKE_B,
	MPCIE_1V5_EN,
	MPCIE_3V3_EN,
	MPCIE_PERST_B,
	MPCIE_WDISABLE_B,
	BUTTON_A_B,
	BUTTON_B_B,
	/* expander @71 */
	/* assigned */
	/* ENET1_RESET_B, */
	/* assigned */
	/* ENET2_RESET_B, */
	USB_RESET_B,
	USB_H2_SELECT,
	WLAN_PD_B,
	WLAN_W_DISABLE_B,
	WLAN_PERST_B,
	V12V_EN,
	/* expander @72 */
	LCD_RESET_B,
	LCD_PWR_EN,
	LCD_BLT_EN,
	DP_EN,
	MIPI_CSI_EN,
	MIPI_CSI_RST_B,
	USER_LED1,
	USER_LED2,
};

static struct tq_gpio_init_data mba9xxxca_gid[] = {
	/* expander 0 */
	GPIO_INIT_DATA_ENTRY(FAN_PWR_EN, "gpio@70_0", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(MPCIE_WAKE_B, "gpio@70_1", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(MPCIE_1V5_EN, "gpio@70_2", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(MPCIE_3V3_EN, "gpio@70_3", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(MPCIE_PERST_B, "gpio@70_4",
			     GPIOD_IS_OUT | GPIOD_ACTIVE_LOW |
			     GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(MPCIE_WDISABLE_B, "gpio@70_5",
			     GPIOD_IS_OUT | GPIOD_ACTIVE_LOW |
			     GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(BUTTON_A_B, "gpio@70_6", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(BUTTON_B_B, "gpio@70_7", GPIOD_IS_IN),
	/* expander 1 */
	GPIO_INIT_DATA_ENTRY(USB_RESET_B, "gpio@71_2",
			     GPIOD_IS_OUT | GPIOD_ACTIVE_LOW |
			     GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(USB_H2_SELECT, "gpio@71_3", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(WLAN_PD_B, "gpio@71_4",
			     GPIOD_IS_OUT | GPIOD_ACTIVE_LOW |
			     GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(WLAN_W_DISABLE_B, "gpio@71_5",
			     GPIOD_IS_OUT | GPIOD_ACTIVE_LOW |
			     GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(WLAN_PERST_B, "gpio@71_6",
			     GPIOD_IS_OUT | GPIOD_ACTIVE_LOW |
			     GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(V12V_EN, "gpio@71_7", GPIOD_IS_OUT),
	/* expander 2 */
	GPIO_INIT_DATA_ENTRY(LCD_RESET_B, "gpio@72_0",
			     GPIOD_IS_OUT | GPIOD_ACTIVE_LOW |
			     GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(LCD_PWR_EN, "gpio@72_1", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(LCD_BLT_EN, "gpio@72_2", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(DP_EN, "gpio@72_3", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(MIPI_CSI_EN, "gpio@72_4", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(MIPI_CSI_RST_B, "gpio@72_5",
			     GPIOD_IS_OUT | GPIOD_ACTIVE_LOW |
			     GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(USER_LED1, "gpio@72_6", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(USER_LED2, "gpio@72_7", GPIOD_IS_OUT),
};

const char *tq_bb_get_boardname(void)
{
	return MBA9XXXCA_BOARD_NAME;
}

int tq_bb_checkboard(void)
{
	return 0;
}

/*
 * SD0 -> mmc0 / mmcblk0
 * SD1 -> mmc1 / mmcblk1
 */
int board_mmc_get_env_dev(int devno)
{
	return devno;
}

/*
 * we use dt alias based indexing, so kernel uses same index. See above
 */
int mmc_map_to_kernel_blk(int devno)
{
	return devno;
}

static int setup_fec(void)
{
	return set_clk_enet(ENET_125MHZ);
}

static int setup_eqos(void)
{
	struct blk_ctrl_wakeupmix_regs *bctrl =
		(struct blk_ctrl_wakeupmix_regs *)BLK_CTRL_WAKEUPMIX_BASE_ADDR;

	/* set INTF as RGMII, enable RGMII TXC clock */
	clrsetbits_le32(&bctrl->eqos_gpr,
			BCTRL_GPR_ENET_QOS_INTF_MODE_MASK,
			BCTRL_GPR_ENET_QOS_INTF_SEL_RGMII | BCTRL_GPR_ENET_QOS_CLK_GEN_EN);

	return set_clk_eqos(ENET_125MHZ);
}

int tq_bb_board_init(void)
{
	tq_board_gpio_init(mba9xxxca_gid, ARRAY_SIZE(mba9xxxca_gid));

	if (CONFIG_IS_ENABLED(FEC_MXC))
		setup_fec();

	if (CONFIG_IS_ENABLED(DWC_ETH_QOS))
		setup_eqos();

	return 0;
}

int tq_bb_board_late_init(void)
{
	if (CONFIG_IS_ENABLED(ENV_IS_IN_MMC))
		board_late_mmc_env_init();

	return 0;
}

#endif
