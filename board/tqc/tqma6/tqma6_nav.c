/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 *
 * Copyright (C) 2013, 2014 - 2016 TQ Systems (ported SabreSD to TQMa6x)
 * Author: Markus Niebel <markus.niebel@tq-group.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/sys_proto.h>
#include <asm/errno.h>
#include <asm/gpio.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/imx-common/mxc_i2c.h>

#include <common.h>
#include <fdt_support.h>
#include <fsl_esdhc.h>
#include <i2c.h>
#include <libfdt.h>
#include <malloc.h>
#include <micrel.h>
#include <miiphy.h>
#include <mmc.h>
#include <netdev.h>
#include <usb.h>

#include "../common/tqc_bb.h"
#include "../common/tqc_eeprom.h"
#include "tqma6_private.h"

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_CLK_PAD_CTRL (PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define GPIO_OUT_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_40ohm   | PAD_CTL_HYS)

#define GPIO_IN_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_240ohm   | PAD_CTL_HYS)

#define GPIO_TP_PAD_CTRL  (PAD_CTL_PUS_100K_DOWN | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_240ohm | PAD_CTL_HYS)

#define GPIO_PKE_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_40ohm   | PAD_CTL_HYS)

#define GPIO_HOG_PAD_CTRL  (PAD_CTL_DSE_DISABLE | PAD_CTL_PKE)

#define I2C_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
	PAD_CTL_DSE_80ohm | PAD_CTL_HYS |			\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#define USB_PAD_CTRL	(PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW | \
			 PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST | \
			 PAD_CTL_HYS)

#define ENET_RX_PAD_CTRL	(PAD_CTL_DSE_34ohm)
#define ENET_TX_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_DSE_34ohm)
#define ENET_CLK_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_HIGH | \
				 PAD_CTL_DSE_40ohm | PAD_CTL_HYS)
#define ENET_MDIO_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
				 PAD_CTL_DSE_60ohm)

static iomux_v3_cfg_t const nav_enet_pads[] = {
	NEW_PAD_CTRL(MX6_PAD_ENET_MDIO__ENET_MDIO,	ENET_MDIO_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET_MDC__ENET_MDC,	ENET_MDIO_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_ENET_TXD0__ENET_TX_DATA0,	ENET_TX_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET_TXD1__ENET_TX_DATA1,	ENET_TX_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET_TX_EN__ENET_TX_EN,	ENET_TX_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET_CRS_DV__ENET_RX_EN,	ENET_TX_PAD_CTRL),


	NEW_PAD_CTRL(MX6_PAD_ENET_RXD0__ENET_RX_DATA0,	ENET_RX_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET_RXD1__ENET_RX_DATA1,	ENET_RX_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET_RX_ER__ENET_RX_ER,	ENET_RX_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_GPIO_16__ENET_REF_CLK,	ENET_CLK_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_GPIO_7__GPIO1_IO07,	GPIO_OUT_PAD_CTRL) |
		     MUX_MODE_SION,
};

#define ENET_PHY_RESET_GPIO IMX_GPIO_NR(1, 7)

static void nav_setup_iomuxc_enet(void)
{
	struct iomuxc *const iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;

	/* clear gpr1[ENET_CLK_SEL] for external clock */
	clrbits_le32(&iomuxc_regs->gpr[1], IOMUXC_GPR1_ENET_CLK_SEL_MASK);
	if (is_mx6dqp()) {
		clrbits_le32(&iomuxc_regs->gpr[5], IOMUXC_GPR5_ENET_TXCLK_SEL_MASK);
	}

	imx_iomux_v3_setup_multiple_pads(nav_enet_pads,
					 ARRAY_SIZE(nav_enet_pads));

	gpio_request(ENET_PHY_RESET_GPIO, "phy-rst#");
	/* Reset PHY */
	gpio_direction_output(ENET_PHY_RESET_GPIO , 0);
	/* SMSC 8720 Datasheet: 100 us + 800 ns until outputs are driven */
	udelay(200);
	gpio_set_value(ENET_PHY_RESET_GPIO, 1);
	/* wait after reset so all phy internals are stable and setup */
	udelay(50);
}

static iomux_v3_cfg_t const nav_uart2_pads[] = {
	NEW_PAD_CTRL(MX6_PAD_SD4_DAT4__UART2_RX_DATA, UART_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD4_DAT7__UART2_TX_DATA, UART_PAD_CTRL),
};

static void nav_setup_iomuxc_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(nav_uart2_pads,
					 ARRAY_SIZE(nav_uart2_pads));
}

static iomux_v3_cfg_t const nav_usdhc2_pads[] = {
	NEW_PAD_CTRL(MX6_PAD_SD2_CLK__SD2_CLK,		USDHC_CLK_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD2_CMD__SD2_CMD,		USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD2_DAT0__SD2_DATA0,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD2_DAT1__SD2_DATA1,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD2_DAT2__SD2_DATA2,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD2_DAT3__SD2_DATA3,	USDHC_PAD_CTRL),
	/* CD */
	NEW_PAD_CTRL(MX6_PAD_GPIO_4__GPIO1_IO04,	GPIO_IN_PAD_CTRL),
	/* WP */
	NEW_PAD_CTRL(MX6_PAD_GPIO_2__GPIO1_IO02,	GPIO_IN_PAD_CTRL),
};

#define USDHC2_CD_GPIO	IMX_GPIO_NR(1, 4)
#define USDHC2_WP_GPIO	IMX_GPIO_NR(1, 2)

int tqc_bb_board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	if (cfg->esdhc_base == USDHC2_BASE_ADDR)
		ret = !gpio_get_value(USDHC2_CD_GPIO);

	return ret;
}

int tqc_bb_board_mmc_getwp(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	if (cfg->esdhc_base == USDHC2_BASE_ADDR)
		ret = gpio_get_value(USDHC2_WP_GPIO);

	return ret;
}

static struct fsl_esdhc_cfg nav_usdhc_cfg = {
	.esdhc_base = USDHC2_BASE_ADDR,
	.max_bus_width = 4,
};

int tqc_bb_board_mmc_init(bd_t *bis)
{
	imx_iomux_v3_setup_multiple_pads(nav_usdhc2_pads,
					 ARRAY_SIZE(nav_usdhc2_pads));
	gpio_request(USDHC2_CD_GPIO, "usdhc2-cd");
	gpio_request(USDHC2_WP_GPIO, "usdhc2-wp");
	gpio_direction_input(USDHC2_CD_GPIO);
	gpio_direction_input(USDHC2_WP_GPIO);

	nav_usdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
	if (fsl_esdhc_initialize(bis, &nav_usdhc_cfg))
		puts("Warning: failed to initialize SD\n");

	return 0;
}

static struct i2c_pads_info nav_i2c1_pads = {
/* I2C1: MBa6x */
	.scl = {
		.i2c_mode = NEW_PAD_CTRL(MX6_PAD_CSI0_DAT9__I2C1_SCL,
					 I2C_PAD_CTRL),
		.gpio_mode = NEW_PAD_CTRL(MX6_PAD_CSI0_DAT9__GPIO5_IO27,
					  I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(5, 27)
	},
	.sda = {
		.i2c_mode = NEW_PAD_CTRL(MX6_PAD_CSI0_DAT8__I2C1_SDA,
					 I2C_PAD_CTRL),
		.gpio_mode = NEW_PAD_CTRL(MX6_PAD_CSI0_DAT8__GPIO5_IO26,
					  I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(5, 26)
	}
};

static void nav_setup_i2c(void)
{
	int ret;

	if (tqma6_get_system_i2c_bus() == 0)
		return;
	/*
	 * use logical index for bus, e.g. I2C1 -> 0
	 * warn on error
	 */
	ret = setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &nav_i2c1_pads);
	if (ret)
		printf("setup I2C1 failed: %d\n", ret);
}

int board_eth_init(bd_t *bis)
{
	struct iomuxc *iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;

	/* clear gpr1[ENET_CLK_SEL] -> disable clock output from anatop */
	clrbits_le32(&iomuxc_regs->gpr[1], IOMUXC_GPR1_ENET_CLK_SEL_MASK);
	return cpu_eth_init(bis);
}

int board_get_dtt_bus(void)
{
	return tqma6_get_system_i2c_bus();
}

iomux_v3_cfg_t const nav_usb_pads[] = {
	NEW_PAD_CTRL(MX6_PAD_GPIO_0__GPIO1_IO00, USB_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_GPIO_3__USB_H1_OC, USB_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_GPIO_1__USB_OTG_ID, USB_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_D21__USB_OTG_OC, USB_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_D22__GPIO3_IO22, USB_PAD_CTRL),
};

/*
 * use gpio instead of PWR as log as he ehci driver does not support
 * board specific polarity
 */
#define NAV_OTG_PWR_GPIO IMX_GPIO_NR(3, 22)
#define NAV_H1_PWR_GPIO IMX_GPIO_NR(1, 0)

static void nav_setup_iomux_usb(void)
{
	int ret;

	imx_iomux_v3_setup_multiple_pads(nav_usb_pads,
					 ARRAY_SIZE(nav_usb_pads));
	ret = gpio_request(NAV_OTG_PWR_GPIO, "usb-otg-pwr");
	if (!ret)
		gpio_direction_output(NAV_OTG_PWR_GPIO, 0);
	ret = gpio_request(NAV_H1_PWR_GPIO, "usb-h1-pwr");
	if (!ret)
		gpio_direction_output(NAV_H1_PWR_GPIO, 0);
}

int board_ehci_hcd_init(int port)
{
	switch (port) {
	case 0:
	case 1:
		break;
	case 2:
	case 3:
		printf("MXC USB port %d not yet supported\n", port);
		return -ENODEV;
		break;
	default:
		return -ENODEV;
	}
	return 0;
}

int board_ehci_power(int port, int on)
{
	switch (port) {
	case 0:
		gpio_set_value(NAV_OTG_PWR_GPIO, on);
		break;
	case 1:
		gpio_set_value(NAV_H1_PWR_GPIO, on);
		break;
	case 2:
	case 3:
		printf("MXC USB port %d not yet supported\n", port);
		return -ENODEV;
		break;
	default:
		return -ENODEV;
	}
	return 0;
}

int board_usb_phy_mode(int index)
{
	return USB_INIT_HOST;
}

static iomux_v3_cfg_t const nav_hog_pads[] = {
	NEW_PAD_CTRL(MX6_PAD_NANDF_D0__GPIO2_IO00, GPIO_HOG_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_NANDF_D1__GPIO2_IO01, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_NANDF_D2__GPIO2_IO02, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_NANDF_D3__GPIO2_IO03, GPIO_HOG_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_SD4_DAT0__GPIO2_IO08, GPIO_HOG_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_NANDF_ALE__GPIO6_IO08, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_NANDF_CS0__GPIO6_IO11, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_NANDF_CS1__GPIO6_IO14, GPIO_HOG_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_SD4_DAT1__GPIO2_IO09, GPIO_HOG_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_CSI0_DAT12__GPIO5_IO30, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_CSI0_DAT13__GPIO5_IO31, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_CSI0_DAT14__GPIO6_IO00, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_CSI0_DAT15__GPIO6_IO01, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_CSI0_DAT16__GPIO6_IO02, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_CSI0_DAT17__GPIO6_IO03, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_CSI0_DAT18__GPIO6_IO04, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_CSI0_DAT19__GPIO6_IO05, GPIO_HOG_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_CSI0_DAT10__GPIO5_IO28, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_CSI0_DAT11__GPIO5_IO29, GPIO_HOG_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_SD4_DAT2__GPIO2_IO10, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_GPIO_19__GPIO4_IO05, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_GPIO_9__GPIO1_IO09, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_GPIO_8__GPIO1_IO08, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_GPIO_17__GPIO7_IO12, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_KEY_COL0__GPIO4_IO06, GPIO_HOG_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_CSI0_PIXCLK__GPIO5_IO18, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_CSI0_VSYNC__GPIO5_IO21, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_CSI0_DAT8__GPIO5_IO26, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_CSI0_DAT9__GPIO5_IO27, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_KEY_COL4__GPIO4_IO14, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_KEY_ROW4__GPIO4_IO15, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_KEY_COL2__GPIO4_IO10, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_KEY_ROW2__GPIO4_IO11, GPIO_HOG_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_SD1_DAT0__GPIO1_IO16, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD1_DAT1__GPIO1_IO17, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD1_CMD__GPIO1_IO18, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD1_CLK__GPIO1_IO20, GPIO_HOG_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_GPIO_18__GPIO7_IO13, GPIO_HOG_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_NANDF_D4__GPIO2_IO04, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_NANDF_D5__GPIO2_IO05, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_NANDF_D6__GPIO2_IO06, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_NANDF_D7__GPIO2_IO07, GPIO_HOG_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_SD4_DAT5__GPIO2_IO13, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD4_DAT6__GPIO2_IO14, GPIO_HOG_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_RGMII_TXC__GPIO6_IO19, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_RXC__GPIO6_IO30, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_RD0__GPIO6_IO25, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_RD1__GPIO6_IO27, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_RD2__GPIO6_IO28, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_RD3__GPIO6_IO29, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_RX_CTL__GPIO6_IO24, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_TD0__GPIO6_IO20, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_TD1__GPIO6_IO21, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_TD2__GPIO6_IO22, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_TD3__GPIO6_IO23, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_TX_CTL__GPIO6_IO26, GPIO_HOG_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_KEY_ROW0__GPIO4_IO07, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_KEY_ROW1__GPIO4_IO09, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_KEY_COL1__GPIO4_IO08, GPIO_HOG_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_SD1_DAT2__GPIO1_IO19, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD4_CLK__GPIO7_IO10, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD4_CMD__GPIO7_IO09, GPIO_HOG_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_EIM_D16__GPIO3_IO16, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_D17__GPIO3_IO17, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_D18__GPIO3_IO18, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_D19__GPIO3_IO19, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_D20__GPIO3_IO20, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_D23__GPIO3_IO23, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_D24__GPIO3_IO24, GPIO_HOG_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_EIM_D26__GPIO3_IO26, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_D28__GPIO3_IO28, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_D30__GPIO3_IO30, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_D31__GPIO3_IO31, GPIO_HOG_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_EIM_CS0__GPIO2_IO23, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_EB3__GPIO2_IO31, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_EB2__GPIO2_IO30, GPIO_HOG_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_EIM_RW__GPIO2_IO26, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_EB1__GPIO2_IO29, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_EB0__GPIO2_IO28, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_LBA__GPIO2_IO27, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_WAIT__GPIO5_IO00, GPIO_HOG_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_EIM_A16__GPIO2_IO22, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_A17__GPIO2_IO21, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_A18__GPIO2_IO20, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_A19__GPIO2_IO19, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_A20__GPIO2_IO18, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_A21__GPIO2_IO17, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_A22__GPIO2_IO16, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_A23__GPIO6_IO06, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_A24__GPIO5_IO04, GPIO_HOG_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_EIM_DA0__GPIO3_IO00, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_DA1__GPIO3_IO01, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_DA2__GPIO3_IO02, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_DA3__GPIO3_IO03, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_DA4__GPIO3_IO04, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_DA5__GPIO3_IO05, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_DA6__GPIO3_IO06, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_DA7__GPIO3_IO07, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_DA8__GPIO3_IO08, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_DA9__GPIO3_IO09, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_DA10__GPIO3_IO10, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_DA11__GPIO3_IO11, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_DA12__GPIO3_IO12, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_DA13__GPIO3_IO13, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_DA14__GPIO3_IO14, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_DA15__GPIO3_IO15, GPIO_HOG_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_EIM_BCLK__GPIO6_IO31, GPIO_HOG_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_DI0_PIN2__GPIO4_IO18, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_DI0_PIN3__GPIO4_IO19, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_DI0_PIN4__GPIO4_IO20, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_DI0_PIN15__GPIO4_IO17, GPIO_HOG_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_DISP0_DAT0__GPIO4_IO21, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_DISP0_DAT1__GPIO4_IO22, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_DISP0_DAT2__GPIO4_IO23, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_DISP0_DAT3__GPIO4_IO24, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_DISP0_DAT4__GPIO4_IO25, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_DISP0_DAT5__GPIO4_IO26, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_DISP0_DAT6__GPIO4_IO27, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_DISP0_DAT7__GPIO4_IO28, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_DISP0_DAT8__GPIO4_IO29, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_DISP0_DAT9__GPIO4_IO30, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_DISP0_DAT10__GPIO4_IO31, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_DISP0_DAT11__GPIO5_IO05, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_DISP0_DAT12__GPIO5_IO06, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_DISP0_DAT13__GPIO5_IO07, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_DISP0_DAT14__GPIO5_IO08, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_DISP0_DAT15__GPIO5_IO09, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_DISP0_DAT16__GPIO5_IO10, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_DISP0_DAT18__GPIO5_IO12, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_DISP0_DAT20__GPIO5_IO14, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_DISP0_DAT21__GPIO5_IO15, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_DISP0_DAT22__GPIO5_IO16, GPIO_HOG_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_DISP0_DAT23__GPIO5_IO17, GPIO_HOG_PAD_CTRL),
};

static iomux_v3_cfg_t const nav_gpio_pads[] = {
	NEW_PAD_CTRL(MX6_PAD_DISP0_DAT17__GPIO5_IO11, GPIO_TP_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_DISP0_DAT19__GPIO5_IO13, GPIO_TP_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_EIM_CS1__GPIO2_IO24, GPIO_PKE_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_OE__GPIO2_IO25, GPIO_PKE_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_D27__GPIO3_IO27, GPIO_PKE_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_D29__GPIO3_IO29, GPIO_PKE_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_CSI0_DATA_EN__GPIO5_IO20, GPIO_OUT_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_NANDF_CLE__GPIO6_IO07, GPIO_OUT_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD4_DAT3__GPIO2_IO11, GPIO_PKE_PAD_CTRL),
};

struct gpio_output_pin {
	unsigned gpio;
	const char *name;
	bool out;
	int level;
};

static const struct gpio_output_pin nav_gpios[] = {
	{
		.gpio = IMX_GPIO_NR(2, 11),
		.name = "zero-credit",
	}, {
		.gpio = IMX_GPIO_NR(6, 7),
		.name = "ir-out",
		.out = true
	}, {
		.gpio = IMX_GPIO_NR(5, 20),
		.name = "aud-rst",
		.out = true,
		.level = 0,
	}, {
		.gpio = IMX_GPIO_NR(2, 24),
		.name = "revdet0",
	}, {
		.gpio = IMX_GPIO_NR(2, 25),
		.name = "revdet1",
	}, {
		.gpio = IMX_GPIO_NR(3, 27),
		.name = "revdet2",
	}, {
		.gpio = IMX_GPIO_NR(2, 29),
		.name = "revdet3",
	}, {
		.gpio = IMX_GPIO_NR(5, 11),
		.name = "pmbin-0",
	}, {
		.gpio = IMX_GPIO_NR(5, 13),
		.name = "pmbin-1",
	},
};

void nav_setup_gpio(void)
{
	int i;
	int ret;

	imx_iomux_v3_setup_multiple_pads(nav_gpio_pads,
					 ARRAY_SIZE(nav_gpio_pads));
	for (i = 0; i < ARRAY_SIZE(nav_gpios); ++i) {
		ret = gpio_request(nav_gpios[i].gpio, nav_gpios[i].name);
		if (!ret) {
			if (nav_gpios[i].out)
				gpio_direction_output(nav_gpios[i].gpio,
						      nav_gpios[i].level);
			else
				gpio_direction_input(nav_gpios[i].gpio);
		}
	}
}

int tqc_bb_board_early_init_f(void)
{
#if defined(CONFIG_DISABLE_CONSOLE)
	gd->flags |= (GD_FLG_DISABLE_CONSOLE);
#else
	nav_setup_iomuxc_uart();
#endif

	return 0;
}

int tqc_bb_board_init(void)
{
	nav_setup_gpio();

	nav_setup_i2c();
	/* do it here - to have reset completed */
	nav_setup_iomuxc_enet();

	nav_setup_iomux_usb();

	return 0;
}

int tqc_bb_board_late_init(void)
{
	enum boot_device bd;

	/*
	* try to get sd card slots in order:
	* eMMC: on Module
	* -> therefore index 0 for bootloader
	* index n in kernel (controller instance 3) -> patches needed for
	* alias indexing
	* SD2: on Mainboard
	* index n in kernel (controller instance 2) -> patches needed for
	* alias indexing
	* we assume to have a kernel patch that will present mmcblk dev
	* indexed like controller devs
	*/
	puts("Boot:\t");
	bd = get_boot_device();
	switch (bd) {
	case MMC3_BOOT:
		setenv("boot_dev", "mmc");
		puts("USDHC3 (eMMC)\n");
		setenv("mmcblkdev", "0");
		setenv("mmcdev", "0");
		break;
	case SD2_BOOT:
		setenv("boot_dev", "mmc");
		puts("USDHC2 (SD)\n");
		setenv("mmcblkdev", "1");
		setenv("mmcdev", "1");
		break;
	default:
		printf("unhandled boot device %d\n",(int)bd);
		setenv("mmcblkdev", "");
		setenv("mmcdev", "");
	}

	return 0;
}

const char *tqc_bb_get_boardname(void)
{
	return "NAV";
}

int board_mmc_get_env_dev(int devno)
{
	/*
	 * eMMC:	USDHC3 -> 0
	 * SD:		USDHC1 -> 1
	 */
	return (2 == devno) ? 0 : 1;
}

/*
 * Device Tree Support
 */
#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
void tqc_bb_ft_board_setup(void *blob, bd_t *bd)
{
	int offset, len, chk;
	const char *compatible, *expect;

	if (tqma6_get_enet_workaround())
		expect = "tq,nav";

	offset = fdt_path_offset(blob, "/");
	compatible = fdt_getprop(blob, offset, "compatible", &len);
	if (len > 0)
		printf("   Device Tree: /compatible = %s\n", compatible);

	chk = fdt_node_check_compatible(blob, offset, expect);
	switch (chk) {
		case 0:
			break;
		case 1:
			printf("   WARNING! Wrong DT variant: "
						"Expecting %s!\n", expect);
			break;
		default:
			printf("   Device Tree: cannot read /compatible"
						"to identify tree variant!\n");
	}
}
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */
