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
#include <usb.h>

#include "../common/tqc_board_gpio.h"
#include "tqma8mpxl-usbg.h"

#define MBA8MPXL_BOARD_NAME "MBa8MPxL"

#define USB_CTRL0_OFFSET	0xF0000
#define USB_CTRL1_OFFSET	0xF0004
#define USB_STS0_OFFSET		0xF0020
#define PHY_CTRL2_OFFSET	0xF0048

#define USB_CTRL0_IDDIG_SEL	BIT(24) /* 0 - ID / 1 - GPIO */
#define USB_CTRL1_OC_POLARITY	BIT(16) /* 0 - HIGH / 1 - LOW */
#define USB_CTRL1_PWR_POLARITY	BIT(17) /* 0 - HIGH / 1 - LOW */
#define USB_STS0_IDDIG		BIT(15) /* 0 - ID GND / 1 - ID float */
#define PHY_CTRL2_IDPULLUP	BIT(14) /* 1 for ID pin sampling */

DECLARE_GLOBAL_DATA_PTR;

enum {
	USB_OTG_ID,
	USB_OTG_PWR,
	USB_RST_B,
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
	PMIC_IRQ,
#if CONFIG_IS_ENABLED(TQMA8MPXL_REV_0200)
	ENET0_INT,
	ENET1_INT,
	TEMP_EVENT,
	RTC_EVENT,
	PCIE_CLKREQ,
	FAN_PWR,
	CODEC_RST,
	VCC12V_EN,
	PERST,
	CLKREQ,
	PEWAKE,
	CSI0_RESET,
	CSI0_SYNC,
	CSI0_TRIGGER,
	CSI0_ENABLE,
#endif
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
	GPIO_INIT_DATA_ENTRY(USB_OTG_ID, "GPIO1_10", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(USB_OTG_PWR, "GPIO1_12", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(USB_RST_B, "GPIO1_11",
			     GPIOD_IS_OUT | GPIOD_ACTIVE_LOW |
			     GPIOD_IS_OUT_ACTIVE),

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
	GPIO_INIT_DATA_ENTRY(PMIC_IRQ, "GPIO1_8", GPIOD_IS_IN),
#if CONFIG_IS_ENABLED(TQMA8MPXL_REV_0200)
	GPIO_INIT_DATA_ENTRY(ENET0_INT, "GPIO4_1", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(ENET1_INT, "GPIO4_3", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(TEMP_EVENT, "GPIO4_21", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(RTC_EVENT, "GPIO4_28", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(PCIE_CLKREQ, "GPIO4_22", GPIOD_IS_OUT | GPIOD_ACTIVE_LOW),
	GPIO_INIT_DATA_ENTRY(FAN_PWR, "GPIO4_27", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(CODEC_RST, "GPIO4_29", GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(VCC12V_EN, "GPIO2_6", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(PERST, "GPIO2_7", GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(CLKREQ, "GPIO2_14", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(PEWAKE, "GPIO2_15", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(CSI0_RESET, "GPIO5_6", GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(CSI0_SYNC, "GPIO5_7", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(CSI0_TRIGGER, "GPIO5_8", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(CSI0_ENABLE, "GPIO5_9", GPIOD_IS_OUT),
#endif
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

#if defined(CONFIG_USB)

int board_usb_init(int index, enum usb_init_type init)
{
	int ret = 0;
	int otg_id;
	u32 *usb_nc_reg;
	struct gpio_desc *gpio;

	imx8m_usb_power(index, true);

	switch (index) {
	case 0:
		/* Set DIG_ID_SEL to muxable PIN for ID detect */
		usb_nc_reg = (u32 *)(ulong)(USB1_BASE_ADDR +
					    USB_CTRL0_OFFSET);
		setbits_le32(usb_nc_reg, USB_CTRL0_IDDIG_SEL);
		/* Set polarity for OC & PWR pins */
		usb_nc_reg = (u32 *)(ulong)(USB1_BASE_ADDR +
					    USB_CTRL1_OFFSET);
		setbits_le32(usb_nc_reg, USB_CTRL1_OC_POLARITY);
		clrbits_le32(usb_nc_reg, USB_CTRL1_PWR_POLARITY);
		/* pullup ID pin - needed for ID Handling */
		usb_nc_reg = (u32 *)(ulong)(USB1_BASE_ADDR +
					    PHY_CTRL2_OFFSET);
		setbits_le32(usb_nc_reg, PHY_CTRL2_IDPULLUP);

		/*
		 * TODO:
		 * use USB_STS0_OFFSET / USB_STS0_IDDIG, saves a GPIO pin
		 */
		otg_id = dm_gpio_get_value(&mba8mpxl_gid[USB_OTG_ID].desc);
		printf("USB0/OTG: ID = %d\n", otg_id);
		gpio = &mba8mpxl_gid[USB_OTG_PWR].desc;
		switch (init) {
		case USB_INIT_DEVICE:
			if (otg_id) {
				dm_gpio_set_value(gpio, 0);
#ifdef CONFIG_USB_DWC3_GADGET
				ret = tqma8mpxl_usb_dwc3_gadget_init();
#endif
			} else {
				ret = -ENODEV;
			}
			break;
		case USB_INIT_HOST:
			if (!otg_id)
				dm_gpio_set_value(gpio, 1);
			else
				ret = -ENODEV;
			break;
		default:
			printf("USB0/OTG: unknown init type\n");
			ret = -EINVAL;
		}
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
		puts("USB0/OTG\n");
		gpio = &mba8mpxl_gid[USB_OTG_PWR].desc;
		switch (init) {
		case USB_INIT_DEVICE:
			break;
		case USB_INIT_HOST:
			break;
		default:
			printf("USB0/OTG: unknown init type\n");
			ret = -EINVAL;
		}
		dm_gpio_set_value(gpio, 0);
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

int tqc_bb_board_init(void)
{
	tqc_board_gpio_init(mba8mpxl_gid, ARRAY_SIZE(mba8mpxl_gid));

	setup_enet();

#if defined(CONFIG_USB)
	init_usb_clk();
#endif

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
