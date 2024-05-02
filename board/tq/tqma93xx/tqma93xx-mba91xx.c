// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022-2024 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 * Author: Alexander Stein
 */

#include <common.h>
#include <i2c.h>
#include <usb.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch-imx9/ccm_regs.h>
#include <asm/arch-imx9/imx93_pins.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm-generic/gpio.h>

#include "../common/tcpc.h"
#include "../common/tq_bb.h"
#include "../common/tq_board_gpio.h"
#include "../common/tq_som_features.h"

DECLARE_GLOBAL_DATA_PTR;

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
#if !CONFIG_IS_ENABLED(GPIO_HOG)
	MPCIE_WAKE_B,
	MPCIE_PERST_B,
	MPCIE_WDISABLE_B,
	WLAN_W_DISABLE_B,
	WLAN_PERST_B,
	WLAN_PD_B,
#endif
	/* expander @70 */
	MPCIE_1V5_EN,
	MPCIE_3V3_EN,
	BUTTON_A_B,
	BUTTON_B_B,
	/* expander @71 */
#if !CONFIG_IS_ENABLED(NETDEVICES)
	ENET1_RESET_B,
	ENET2_RESET_B,
#endif
#if !CONFIG_IS_ENABLED(USB_ONBOARD_HUB)
	USB_RESET_B,
#endif
	V12V_EN,
	/* expander @72 */
	LCD_RESET_B,
	LCD_PWR_EN,
	LCD_BLT_EN,
	LVDS_SHDN_B,
	FAN_PWR_EN,
	USER_LED1,
	USER_LED2,
};

static struct tq_gpio_init_data mba91xx_gid[] = {
#if !CONFIG_IS_ENABLED(GPIO_HOG)
	GPIO_INIT_DATA_ENTRY(MPCIE_WAKE_B, "gpio@70_1", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(MPCIE_PERST_B, "gpio@70_4",
			     GPIOD_IS_OUT | GPIOD_ACTIVE_LOW |
			     GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(MPCIE_WDISABLE_B, "gpio@70_5",
			     GPIOD_IS_OUT | GPIOD_ACTIVE_LOW |
			     GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(WLAN_PD_B, "gpio@71_4",
			     GPIOD_IS_OUT | GPIOD_ACTIVE_LOW |
			     GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(WLAN_W_DISABLE_B, "gpio@71_5",
			     GPIOD_IS_OUT | GPIOD_ACTIVE_LOW |
			     GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(WLAN_PERST_B, "gpio@71_6",
			     GPIOD_IS_OUT | GPIOD_ACTIVE_LOW |
			     GPIOD_IS_OUT_ACTIVE),
#endif
	/* expander 0 */
	GPIO_INIT_DATA_ENTRY(MPCIE_1V5_EN, "gpio@70_2", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(MPCIE_3V3_EN, "gpio@70_3", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(BUTTON_A_B, "gpio@70_6", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(BUTTON_B_B, "gpio@70_7", GPIOD_IS_IN),
	/* expander 1 */
#if !CONFIG_IS_ENABLED(NETDEVICES)
	GPIO_INIT_DATA_ENTRY(ENET1_RESET_B, "gpio@71_0",
			     GPIOD_IS_OUT | GPIOD_ACTIVE_LOW |
			     GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(ENET2_RESET_B, "gpio@71_1",
			     GPIOD_IS_OUT | GPIOD_ACTIVE_LOW |
			     GPIOD_IS_OUT_ACTIVE),
#endif
#if !CONFIG_IS_ENABLED(USB_ONBOARD_HUB)
	GPIO_INIT_DATA_ENTRY(USB_RESET_B, "gpio@71_2",
			     GPIOD_IS_OUT | GPIOD_ACTIVE_LOW |
			     GPIOD_IS_OUT_ACTIVE),
#endif
	GPIO_INIT_DATA_ENTRY(V12V_EN, "gpio@71_7", GPIOD_IS_OUT),
	/* expander 2 */
	GPIO_INIT_DATA_ENTRY(LCD_RESET_B, "gpio@72_0",
			     GPIOD_IS_OUT | GPIOD_ACTIVE_LOW |
			     GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(LCD_PWR_EN, "gpio@72_1", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(LCD_BLT_EN, "gpio@72_2", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(LVDS_SHDN_B, "gpio@72_3", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(FAN_PWR_EN, "gpio@72_4", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(USER_LED1, "gpio@72_6", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(USER_LED2, "gpio@72_7", GPIOD_IS_OUT),
};

const char *tq_bb_get_boardname(void)
{
	return "MBa91xxCA";
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

#if CONFIG_IS_ENABLED(USB)

#if CONFIG_IS_ENABLED(USB_TCPC)

struct tcpc_port typec_port;

static struct tcpc_port_config typec_port_config = {
	.i2c_bus = 2, /* I2C3 */
	.addr = 0x50,
	.port_type = TYPEC_PORT_UFP, /* Device only */
/*
 * We are UFP, no power delivery. For completeness use reasonable values
 * if we ever want to use this as dual role port in U-Boot.
 */
	.max_snk_mv = 5000,
	.max_snk_ma = 500,
	.max_snk_mw = 2500,
	.op_snk_mv = 5000,

	.disable_pd = true,
};

static int setup_typec(void)
{
	int ret;

	debug("tcpc_init port\n");
	ret = tcpc_init(&typec_port, typec_port_config, NULL);
	if (ret) {
		printf("%s: tcpc port init failed, err=%d\n",
		       __func__, ret);
	}

	return ret;
}

int board_ehci_usb_phy_mode(struct udevice *dev)
{
	enum typec_cc_polarity pol;
	enum typec_cc_state state;
	struct tcpc_port *port_ptr;
	int ret = 0;

	debug("%s %d\n", __func__, dev_seq(dev));
	/* dev_seq == 0: USB1, dev_seq == 1: USB2 */
	if (dev_seq(dev) == 1)
		return USB_INIT_HOST;

	port_ptr = &typec_port;

	tcpc_setup_ufp_mode(port_ptr);
	ret = tcpc_get_cc_status(port_ptr, &pol, &state);

	tcpc_print_log(port_ptr);
	if (!ret) {
		if (state == TYPEC_STATE_SRC_RD_RA || state == TYPEC_STATE_SRC_RD)
			return USB_INIT_HOST;
	} else {
		printf("ERROR: getting TypeC CC status %d\n", ret);
	}

	return USB_INIT_DEVICE;
}

#endif

int board_usb_init(int index, enum usb_init_type init)
{
	int ret = 0;

	switch (index) {
#if CONFIG_IS_ENABLED(USB_TCPC)
	case 0:
		debug("USB1/Type-C\n");
		switch (init) {
		case USB_INIT_DEVICE:
			ret = tcpc_setup_ufp_mode(&typec_port);
			break;
		case USB_INIT_HOST:
		default:
			printf("USB1: unsupported init type\n");
			ret = -EINVAL;
		}
		break;
#endif
	case 1:
		debug("USB2/HUB\n");
#if !CONFIG_IS_ENABLED(USB_ONBOARD_HUB)
		switch (init) {
		case USB_INIT_DEVICE:
			ret = -ENODEV;
			break;
		case USB_INIT_HOST:
			dm_gpio_set_value(&mba91xx_gid[USB_RESET_B].desc, 0);
			break;
		default:
			printf("USB2: unknown init type\n");
			ret = -EINVAL;
		}
#endif
		break;
	default:
		printf("invalid USB port %d\n", index);
		ret = -EINVAL;
	}

	return ret;
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	int ret = 0;

	switch (index) {
#if CONFIG_IS_ENABLED(USB_TCPC)
	case 0:
		debug("USB1/Type-C\n");
		if (init == USB_INIT_HOST)
			ret = tcpc_disable_src_vbus(&typec_port);
		break;
#endif
	case 1:
		debug("USB2/HUB\n");
#if !CONFIG_IS_ENABLED(USB_ONBOARD_HUB)
		dm_gpio_set_value(&mba91xx_gid[USB_RESET_B].desc, 1);
#endif
		break;
	default:
		printf("invalid USB port %d\n", index);
		ret = -EINVAL;
	}

	return ret;
}
#endif

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
	tq_board_gpio_init(mba91xx_gid, ARRAY_SIZE(mba91xx_gid));

	if (CONFIG_IS_ENABLED(USB_TCPC))
		setup_typec();

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

#if CONFIG_IS_ENABLED(OF_BOARD_SETUP)

int tq_bb_ft_board_setup(void *blob, struct bd_info *bis)
{
	const struct tq_som_feature_list *features = tq_board_detect_features();

	if (!features) {
		pr_warn("WARN: tq_board_detect_features failed\n");
		return -ENODATA;
	}

	tq_ft_fixup_features(blob, features);

	return 0;
}
#endif

#endif
