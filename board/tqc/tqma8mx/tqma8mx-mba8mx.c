// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 - 2020 TQ Systems GmbH
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
	USB1_OTG_PWR,
	USB1_OTG_ID,
	GPIO_BTN1,
	GPIO_BTN2,
	GPIO_BTN3,
	GPIO_LED1,
	GPIO_LED2,
	GPIO_LED3,
};

#define UART_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_FSEL1)

#define WDOG_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE)

static iomux_v3_cfg_t const wdog_pads[] = {
	IMX8MQ_PAD_GPIO1_IO02__WDOG1_WDOG_B | MUX_PAD_CTRL(WDOG_PAD_CTRL),
};

#if (CONFIG_MXC_UART_BASE == UART1_BASE_ADDR)
static iomux_v3_cfg_t const uart_pads[] = {
	IMX8MQ_PAD_UART1_RXD__UART1_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	IMX8MQ_PAD_UART1_TXD__UART1_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};
static const u32 uart_index = 0;
#elif (CONFIG_MXC_UART_BASE == UART3_BASE_ADDR)
static iomux_v3_cfg_t const uart_pads[] = {
	IMX8MQ_PAD_UART3_RXD__UART3_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	IMX8MQ_PAD_UART3_TXD__UART3_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};
static const u32 uart_index = 2;
#else
#error
#endif

/*
 * NOTE: this is also used by SPL
 */
int tqc_bb_board_early_init_f(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;

	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));

	set_wdog_reset(wdog);
	 /* Init UART<n> clock */
	init_uart_clk(uart_index);

	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));

	return 0;
}

#if !defined(CONFIG_SPL_BUILD)

static struct tqc_gpio_init_data mba8mx_gid[] = {
	GPIO_INIT_DATA_ENTRY(UART1_MUX, "GPIO@23_0", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(UART2_MUX, "GPIO@23_1", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(SD_MUX, "GPIO@23_2", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(DSI_MUX, "GPIO@23_3", GPIOD_IS_IN),

	GPIO_INIT_DATA_ENTRY(SPI_MUX, "GPIO@23_4", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(AVCC1V8_LVDS_EN, "GPIO@23_5", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(LVDS_BRIDGE_EN, "GPIO@23_6", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(LVDS_BRIDGE_IRQ, "GPIO@23_7", GPIOD_IS_IN),

	GPIO_INIT_DATA_ENTRY(SD_MUX_EN_B, "GPIO@23_8", GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE),
#if defined(CONFIG_VIDEO_IMXDCSS)
	GPIO_INIT_DATA_ENTRY(EN_HDMI_TERM, "GPIO@23_9", GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE),
#else
	GPIO_INIT_DATA_ENTRY(EN_HDMI_TERM, "GPIO@23_9", GPIOD_IS_OUT),
#endif
	GPIO_INIT_DATA_ENTRY(DSI_MUX_OE_B, "GPIO@23_10", GPIOD_IS_OUT | GPIOD_ACTIVE_LOW),
	GPIO_INIT_DATA_ENTRY(EN_DP_BRIDGE_3V3, "GPIO@23_11", GPIOD_IS_OUT),

	GPIO_INIT_DATA_ENTRY(BOOT_CFG_OE_B, "GPIO@23_12", GPIOD_IS_OUT | GPIOD_ACTIVE_LOW),
	GPIO_INIT_DATA_ENTRY(RST_USB_HUB_B, "GPIO@23_13", GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(PCIE_RST_B, "GPIO@23_14", GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(PCIE_WAKE_B, "GPIO@23_15", GPIOD_IS_IN),

	GPIO_INIT_DATA_ENTRY(BOOT_CFG0, "GPIO@24_0", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(BOOT_CFG1, "GPIO@24_1", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(BOOT_CFG2, "GPIO@24_2", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(BOOT_CFG3, "GPIO@24_3", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(BOOT_CFG4, "GPIO@24_4", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(BOOT_CFG5, "GPIO@24_5", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(BOOT_CFG6, "GPIO@24_6", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(BOOT_CFG7, "GPIO@24_7", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(BOOT_CFG8, "GPIO@24_8", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(BOOT_CFG9, "GPIO@24_9", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(BOOT_CFG10, "GPIO@24_10", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(BOOT_CFG11, "GPIO@24_11", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(BOOT_CFG12, "GPIO@24_12", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(BOOT_CFG13, "GPIO@24_13", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(BOOT_CFG14, "GPIO@24_14", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(BOOT_CFG15, "GPIO@24_15", GPIOD_IS_IN),

	GPIO_INIT_DATA_ENTRY(AUDIO_CODEC_RESET_B, "GPIO@25_0", GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(LCD_RESET_B, "GPIO@25_1", GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(LCD_BLT_EN, "GPIO@25_2", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(LCD_PWR_EN, "GPIO@25_3", GPIOD_IS_OUT),

	GPIO_INIT_DATA_ENTRY(EDP_IRQ, "GPIO@25_4", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(HDMI_FAULT_B, "GPIO@25_5", GPIOD_IS_IN),

	GPIO_INIT_DATA_ENTRY(MPCIE_WAKE_B, "GPIO@25_8", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(SIM_CARD_DETECT, "GPIO@25_9", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(CAMX_PWR_B, "GPIO@25_10", GPIOD_IS_OUT | GPIOD_ACTIVE_LOW),
	GPIO_INIT_DATA_ENTRY(INT_MIKRO_MODULE, "GPIO@25_11", GPIOD_IS_IN),

	GPIO_INIT_DATA_ENTRY(MPCIE_DIS_B, "GPIO@25_12", GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(MPCIE_RST_B, "GPIO@25_13", GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(CAMX_RST_B, "GPIO@25_14", GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(RESET_MIKRO_MODULE_B, "GPIO@25_15", GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE),

	GPIO_INIT_DATA_ENTRY(USB1_OTG_ID, "GPIO1_10", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(USB1_OTG_PWR, "GPIO1_12", GPIOD_IS_OUT),

	GPIO_INIT_DATA_ENTRY(GPIO_BTN1, "GPIO1_5", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(GPIO_BTN2, "GPIO3_17", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(GPIO_BTN3, "GPIO1_7", GPIOD_IS_IN),

	GPIO_INIT_DATA_ENTRY(GPIO_LED1, "GPIO1_0", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(GPIO_LED2, "GPIO3_16", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(GPIO_LED3, "GPIO1_8", GPIOD_IS_OUT),
};

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

#if defined(CONFIG_USB)

int board_usb_init(int index, enum usb_init_type init)
{
	int ret = 0;
	int otg_id;
	struct gpio_desc *gpio;

	imx8m_usb_power(index, true);

	switch(index) {
	case 0:
		otg_id = dm_gpio_get_value(&mba8mx_gid[USB1_OTG_ID].desc);
		printf("USB0/OTG: ID = %d\n", otg_id);
		gpio = &mba8mx_gid[USB1_OTG_PWR].desc;
		switch (init) {
		case USB_INIT_DEVICE:
			if (otg_id)
				dm_gpio_set_value(gpio, 0);
			else
				ret = -ENODEV;
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
		gpio = &mba8mx_gid[RST_USB_HUB_B].desc;
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

	switch(index) {
	case 0:
		puts("USB0/OTG\n");
		gpio = &mba8mx_gid[USB1_OTG_PWR].desc;
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
		gpio = &mba8mx_gid[RST_USB_HUB_B].desc;
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
	int idx;
	unsigned cfg = 0x0;

	tqc_board_gpio_init(mba8mx_gid, ARRAY_SIZE(mba8mx_gid));
	for (idx = BOOT_CFG0; idx <= BOOT_CFG15; ++idx)
		cfg |= (dm_gpio_get_value(&mba8mx_gid[idx].desc) ? 1 : 0) <<
			(idx - BOOT_CFG0);
	env_set_ulong("boot_config", cfg);
	printf("BOOTCFG: %x\n", cfg);
	/* disable boot cfg signals on MCU bus */
	dm_gpio_set_value(&mba8mx_gid[BOOT_CFG_OE_B].desc, 0);

	puts("MUX: ");
	printf("UART1: %d ", dm_gpio_get_value(&mba8mx_gid[UART1_MUX].desc));
	printf("UART2: %d ", dm_gpio_get_value(&mba8mx_gid[UART2_MUX].desc));
	printf("SD: %d ", dm_gpio_get_value(&mba8mx_gid[SD_MUX].desc));
	printf("DSI: %d ", dm_gpio_get_value(&mba8mx_gid[DSI_MUX].desc));
	printf("SPI: %d\n", dm_gpio_get_value(&mba8mx_gid[SPI_MUX].desc));

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

#endif /* #if !defined(CONFIG_SPL_BUILD) */
