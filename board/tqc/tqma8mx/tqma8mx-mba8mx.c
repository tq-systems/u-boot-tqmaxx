/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2018 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm-generic/gpio.h>
#include <fsl_esdhc.h>
#include <mmc.h>
#include <asm/arch/imx8mq_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/arch/clock.h>
#include <spl.h>
#include <dm.h>
#include <usb.h>
#include <dwc3-uboot.h>

#include "../common/tqc_bb.h"
#include "../common/tqc_board_gpio.h"

DECLARE_GLOBAL_DATA_PTR;

#define TQMA8MX_BB_NAME "MBa8Mx"

enum {
	UART1_MUX,
	UART2_MUX,
	SD_MUX,
	DSI_MUX,
	SPI_MUX,
	AVCC1V8_LVDS_EN,
	LVDS_BRIDGE_EN,
	LVDS_BRIDGE_IRQ,
	SD_MUX_EN_B,
	EN_HDMI_TERM,
	DSI_MUX_OE_B,
	EN_DP_BRIDGE_3V3,
	BOOT_CFG_OE_B,
	RST_USB_HUB_B,
	PCIE_RST_B,
	PCIE_WAKE_B,
	BOOT_CFG0,
	BOOT_CFG1,
	BOOT_CFG2,
	BOOT_CFG3,
	BOOT_CFG4,
	BOOT_CFG5,
	BOOT_CFG6,
	BOOT_CFG7,
	BOOT_CFG8,
	BOOT_CFG9,
	BOOT_CFG10,
	BOOT_CFG11,
	BOOT_CFG12,
	BOOT_CFG13,
	BOOT_CFG14,
	BOOT_CFG15,
	AUDIO_CODEC_RESET_B,
	LCD_RESET_B,
	LCD_BLT_EN,
	LCD_PWR_EN,
	EDP_IRQ,
	HDMI_FAULT_B,
	MPCIE_WAKE_B,
	SIM_CARD_DETECT,
	CAMX_PWR_B,
	INT_MIKRO_MODULE,
	MPCIE_DIS_B,
	MPCIE_RST_B,
	CAMX_RST_B,
	RESET_MIKRO_MODULE_B,
};

#define UART_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_FSEL1)

#define WDOG_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE)

static iomux_v3_cfg_t const wdog_pads[] = {
	IMX8MQ_PAD_GPIO1_IO02__WDOG1_WDOG_B | MUX_PAD_CTRL(WDOG_PAD_CTRL),
};

static iomux_v3_cfg_t const uart_pads[] = {
	IMX8MQ_PAD_UART1_RXD__UART1_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	IMX8MQ_PAD_UART1_TXD__UART1_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static struct tqc_gpio_init_data mba8mx_gid[] = {
	{
		.name = "GPIO@23_0",
		.label = __stringify(UART1_MUX),
		.flags = GPIOD_IS_IN,
	}, {
		.name = "GPIO@23_1",
		.label = __stringify(UART2_MUX),
		.flags = GPIOD_IS_IN,
	}, {
		.name = "GPIO@23_2",
		.label = __stringify(SD_MUX),
		.flags = GPIOD_IS_IN,
	}, {
		.name = "GPIO@23_3",
		.label = __stringify(DSI_MUX),
		.flags = GPIOD_IS_IN,
	}, {
		.name = "GPIO@23_4",
		.label = __stringify(SPI_MUX),
		.flags = GPIOD_IS_IN,
	}, {
		.name = "GPIO@23_5",
		.label = __stringify(AVCC1V8_LVDS_EN),
		.flags = GPIOD_IS_OUT,
	}, {
		.name = "GPIO@23_6",
		.label = __stringify(LVDS_BRIDGE_EN),
		.flags = GPIOD_IS_OUT,
	}, {
		.name = "GPIO@23_7",
		.label = __stringify(LVDS_BRIDGE_IRQ),
		.flags = GPIOD_IS_IN,
	}, {
		.name = "GPIO@23_8",
		.label = __stringify(SD_MUX_EN#),
		.flags = GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE,
	}, {
		.name = "GPIO@23_9",
		.label = __stringify(EN_HDMI_TERM),
		.flags = GPIOD_IS_OUT,
	}, {
		.name = "GPIO@23_10",
		.label = __stringify(DSI_MUX_OE#),
		.flags = GPIOD_IS_OUT | GPIOD_ACTIVE_LOW,
	}, {
		.name = "GPIO@23_11",
		.label = __stringify(EN_DP_BRIDGE_3V3),
		.flags = GPIOD_IS_OUT,
	}, {
		.name = "GPIO@23_12",
		.label = __stringify(BOOT_CFG_OE#),
		.flags = GPIOD_IS_OUT | GPIOD_ACTIVE_LOW,
	}, {
		.name = "GPIO@23_13",
		.label = __stringify(RST_USB_HUB#),
		.flags = GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE,
	}, {
		.name = "GPIO@23_14",
		.label = __stringify(PCIE_RST#),
		.flags = GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE,
	}, {
		.name = "GPIO@23_15",
		.label = __stringify(PCIE_WAKE#),
		.flags = GPIOD_IS_IN,
	}, {
		.name = "GPIO@24_0",
		.label = __stringify(BOOT_CFG0),
		.flags = GPIOD_IS_IN,
	}, {
		.name = "GPIO@24_1",
		.label = __stringify(BOOT_CFG1),
		.flags = GPIOD_IS_IN,
	}, {
		.name = "GPIO@24_2",
		.label = __stringify(BOOT_CFG2),
		.flags = GPIOD_IS_IN,
	}, {
		.name = "GPIO@24_3",
		.label = __stringify(BOOT_CFG3),
		.flags = GPIOD_IS_IN,
	}, {
		.name = "GPIO@24_4",
		.label = __stringify(BOOT_CFG4),
		.flags = GPIOD_IS_IN,
	}, {
		.name = "GPIO@24_5",
		.label = __stringify(BOOT_CFG5),
		.flags = GPIOD_IS_IN,
	}, {
		.name = "GPIO@24_6",
		.label = __stringify(BOOT_CFG6),
		.flags = GPIOD_IS_IN,
	}, {
		.name = "GPIO@24_7",
		.label = __stringify(BOOT_CFG7),
		.flags = GPIOD_IS_IN,
	}, {
		.name = "GPIO@24_8",
		.label = __stringify(BOOT_CFG8),
		.flags = GPIOD_IS_IN,
	}, {
		.name = "GPIO@24_9",
		.label = __stringify(BOOT_CFG9),
		.flags = GPIOD_IS_IN,
	}, {
		.name = "GPIO@24_10",
		.label = __stringify(BOOT_CFG10),
		.flags = GPIOD_IS_IN,
	}, {
		.name = "GPIO@24_11",
		.label = __stringify(BOOT_CFG11),
		.flags = GPIOD_IS_IN,
	}, {
		.name = "GPIO@24_12",
		.label = __stringify(BOOT_CFG12),
		.flags = GPIOD_IS_IN,
	}, {
		.name = "GPIO@24_13",
		.label = __stringify(BOOT_CFG13),
		.flags = GPIOD_IS_IN,
	}, {
		.name = "GPIO@24_14",
		.label = __stringify(BOOT_CFG14),
		.flags = GPIOD_IS_IN,
	}, {
		.name = "GPIO@24_15",
		.label = __stringify(BOOT_CFG15),
		.flags = GPIOD_IS_IN,
	}, {
		.name = "GPIO@25_0",
		.label = __stringify(AUDIO_CODEC_RESET#),
		.flags = GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE,
	}, {
		.name = "GPIO@25_1",
		.label = __stringify(LCD_RESET#),
		.flags = GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE,
	}, {
		.name = "GPIO@25_2",
		.label = __stringify(LCD_BLT_EN),
		.flags = GPIOD_IS_OUT,
	}, {
		.name = "GPIO@25_3",
		.label = __stringify(LCD_PWR_EN),
		.flags = GPIOD_IS_OUT,
	}, {
		.name = "GPIO@25_4",
		.label = __stringify(EDP_IRQ),
		.flags = GPIOD_IS_IN,
	}, {
		.name = "GPIO@25_5",
		.label = __stringify(HDMI_FAULT#),
		.flags = GPIOD_IS_IN,
	}, {
		.name = "GPIO@25_8",
		.label = __stringify(MPCIE_WAKE#),
		.flags = GPIOD_IS_IN,
	}, {
		.name = "GPIO@25_9",
		.label = __stringify(SIM_CARD_DETECT),
		.flags = GPIOD_IS_IN,
	}, {
		.name = "GPIO@25_10",
		.label = __stringify(CAMX_PWR#),
		.flags = GPIOD_IS_OUT | GPIOD_ACTIVE_LOW,
	}, {
		.name = "GPIO@25_11",
		.label = __stringify(INT_MIKRO_MODULE),
		.flags = GPIOD_IS_IN,
	}, {
		.name = "GPIO@25_12",
		.label = __stringify(MPCIE_DIS#),
		.flags = GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE,
	}, {
		.name = "GPIO@25_13",
		.label = __stringify(MPCIE_RST#),
		.flags = GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE,
	}, {
		.name = "GPIO@25_14",
		.label = __stringify(CAMX_RST#),
		.flags = GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE,
	}, {
		.name = "GPIO@25_15",
		.label = __stringify(RESET_MIKRO_MODULE#),
		.flags = GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE,
	},
};

/*
 * NOTE: this is also used by SPL
 */
int tqc_bb_board_early_init_f(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;

	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));

	set_wdog_reset(wdog);

	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));

	return 0;
}

#ifdef CONFIG_OF_BOARD_SETUP
int tqc_bb_ft_board_setup(void *blob, bd_t *bd)
{
	return 0;
}
#endif

#ifdef CONFIG_FEC_MXC
static int setup_fec(void)
{
	struct iomuxc_gpr_base_regs *const iomuxc_gpr_regs
		= (struct iomuxc_gpr_base_regs *) IOMUXC_GPR_BASE_ADDR;

	/* Use 125M anatop REF_CLK1 for ENET1, not from external */
	clrsetbits_le32(&iomuxc_gpr_regs->gpr[1],
			IOMUXC_GPR_GPR1_GPR_ENET1_TX_CLK_SEL_SHIFT, 0);
	return set_clk_enet(ENET_125MHZ);
}
#endif

#if defined(CONFIG_USB_DWC3) || defined(CONFIG_USB_XHCI_IMX8M)

int board_usb_init(int index, enum usb_init_type init)
{
	int ret = 0;
	struct gpio_desc *hub_reset_gpio;

	switch(index) {
	case 0:
		puts("USB0/OTG\n");
		switch (init) {
		case USB_INIT_DEVICE:
			break;
		case USB_INIT_HOST:
			imx8m_usb_power(index, true);
			break;
		default:
			printf("USB0/OTG: unknown init type\n");
			ret = -EINVAL;
		}
		break;
	case 1:
		puts("USB1/HUB\n");
		hub_reset_gpio = &mba8mx_gid[RST_USB_HUB_B].desc;
		dm_gpio_set_value(hub_reset_gpio, 1);
		udelay(100);
		dm_gpio_set_value(hub_reset_gpio, 0);
		udelay(100);
		imx8m_usb_power(index, true);
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
	struct gpio_desc *hub_reset_gpio;

	switch(index) {
	case 0:
		puts("USB0/OTG\n");
		switch (init) {
		case USB_INIT_DEVICE:
			break;
		case USB_INIT_HOST:
			imx8m_usb_power(index, false);
			break;
		default:
			printf("USB0/OTG: unknown init type\n");
			ret = -EINVAL;
		}
		break;
	case 1:
		puts("USB1/HUB\n");
		hub_reset_gpio = &mba8mx_gid[RST_USB_HUB_B].desc;
		dm_gpio_set_value(hub_reset_gpio, 1);
		imx8m_usb_power(index, false);
		break;
	default:
		printf("invalid USB port %d\n", index);
		ret = -EINVAL;
	}

	return ret;
}
#endif

int tqc_bb_board_init(void)
{
	tqc_board_gpio_init(mba8mx_gid, ARRAY_SIZE(mba8mx_gid));

#ifdef CONFIG_FEC_MXC
	setup_fec();
#endif

	return 0;
}

/*
 * SD1 -> mmc0 / mmcblk0
 * SD2 -> mmc1 / mmcblk1
 */
int board_mmc_get_env_dev(int devno)
{
	return devno;
}

int mmc_map_to_kernel_blk(int dev_no)
{
	return dev_no;
}

int tqc_bb_board_late_init(void)
{
#ifdef CONFIG_ENV_IS_IN_MMC
	board_late_mmc_env_init();
#endif

	return 0;
}

const char *tqc_bb_get_boardname(void)
{
	return TQMA8MX_BB_NAME;
}

#if defined(CONFIG_VIDEO_IMXDCSS)
#include <asm/mach-imx/video.h>
#include <asm/arch/video_common.h>

struct display_info_t const displays[] = {{
	.bus	= 0, /* Unused */
	.addr	= 0, /* Unused */
	.pixfmt	= GDF_32BIT_X888RGB,
	.detect	= NULL,
	.enable	= NULL,
#ifndef CONFIG_VIDEO_IMXDCSS_1080P
	.mode	= {
		.name           = "HDMI", /* 720P60 */
		.refresh        = 60,
		.xres           = 1280,
		.yres           = 720,
		.pixclock       = 13468, /* 74250  kHz */
		.left_margin    = 110,
		.right_margin   = 220,
		.upper_margin   = 5,
		.lower_margin   = 20,
		.hsync_len      = 40,
		.vsync_len      = 5,
		.sync           = FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
		.vmode          = FB_VMODE_NONINTERLACED
	}
#else
	.mode	= {
		.name           = "HDMI", /* 1080P60 */
		.refresh        = 60,
		.xres           = 1920,
		.yres           = 1080,
		.pixclock       = 6734, /* 148500 kHz */
		.left_margin    = 148,
		.right_margin   = 88,
		.upper_margin   = 36,
		.lower_margin   = 4,
		.hsync_len      = 44,
		.vsync_len      = 5,
		.sync           = FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
		.vmode          = FB_VMODE_NONINTERLACED
	}
#endif
} };
size_t display_count = ARRAY_SIZE(displays);

#endif /* CONFIG_VIDEO_IMXDCSS */
