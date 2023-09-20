// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Martin Schmiedel
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
#include <usb.h>

#include "../common/tqc_board_gpio.h"
#include "../common/tq_som_features.h"
#include "tqma8mpxl-usbg.h"

#define MBA8MP_RAS314_BOARD_NAME "MBa8MP-RAS314"

DECLARE_GLOBAL_DATA_PTR;

enum {
	USB_RST_B,
	USER_LED1,
	USER_LED2,
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
int tqc_bb_board_early_init_f(void)
{
	init_uart_clk(3);
	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));

	return 0;
}

#if !defined(CONFIG_SPL_BUILD)

const char *tqc_bb_get_boardname(void)
{
	return MBA8MP_RAS314_BOARD_NAME;
}

int tqc_bb_checkboard(void)
{
	return 0;
}

static struct tqc_gpio_init_data mba8mpxl_gid[] = {
	GPIO_INIT_DATA_ENTRY(USB_RST_B, "GPIO5_26",
			     GPIOD_IS_OUT | GPIOD_ACTIVE_LOW |
			     GPIOD_IS_OUT_ACTIVE),

	GPIO_INIT_DATA_ENTRY(USER_LED1, "GPIO4_18", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(USER_LED2, "GPIO4_19", GPIOD_IS_OUT),

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
		return env_get_ulong("mmcdev", 10, CONFIG_SYS_MMC_ENV_DEV);
	}
}

/*
 * we use dt alias based indexing, so kernel uses same index. See above
 */
int mmc_map_to_kernel_blk(int devno)
{
	return devno;
}

#if (IS_ENABLED(CONFIG_USB))

int board_usb_init(int index, enum usb_init_type init)
{
	int ret = 0;
	struct gpio_desc *gpio;

	imx8m_usb_power(index, true);

	switch (index) {
	case 0:
		puts("USB0/DEVICE\n");

		if (IS_ENABLED(CONFIG_USB_DWC3_GADGET))
			ret = tqma8mpxl_usb_dwc3_gadget_init(USB_SPEED_SUPER);

		break;
	case 1:
		puts("USB1/HUB\n");
		gpio = &mba8mpxl_gid[USB_RST_B].desc;
		dm_gpio_set_value(gpio, 1);
		udelay(100);
		dm_gpio_set_value(gpio, 0);
		udelay(100);
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
	struct gpio_desc *gpio;

	switch (index) {
	case 0:
		puts("USB0/DEVICE\n");
		break;
	case 1:
		puts("USB1/HUB\n");
		gpio = &mba8mpxl_gid[USB_RST_B].desc;
		dm_gpio_set_value(gpio, 1);
		break;
	default:
		printf("invalid USB port %d\n", index);
		ret = -EINVAL;
	}

	imx8m_usb_power(index, false);

	return ret;
}

#endif

#define FSL_SIP_GPC			0xC2000000
#define FSL_SIP_CONFIG_GPC_PM_DOMAIN	0x3
#define DISPMIX				13
#define MIPI				15

int tqc_bb_board_init(void)
{
	tqc_board_gpio_init(mba8mpxl_gid, ARRAY_SIZE(mba8mpxl_gid));

	setup_enet();

#if defined(CONFIG_USB)
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

int tqc_bb_ft_board_setup(void *blob, bd_t *bd)
{
	struct tq_som_feature_list *features;

	features = tq_board_detect_features();
	if (!features) {
		pr_warn("tq_board_detect_features failed\n");
		return -ENODATA;
	}

	tq_ft_fixup_features(blob, features);

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
