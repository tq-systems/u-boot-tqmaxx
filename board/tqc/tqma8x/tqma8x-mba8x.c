// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2018-2020 TQ-Systems GmbH
 */
#include <common.h>
#include <bootm.h>
#include <dm.h>
#include <errno.h>
#include <fdt_support.h>
#include <linux/libfdt.h>
#include <malloc.h>
#include <env.h>
#include <fsl_esdhc.h>

#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/sci/sci.h>
#include <asm/arch/imx8-pins.h>
#include <asm/arch/lpcg.h>
#include <asm/arch/iomux.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <power-domain.h>
#include <usb.h>

#include "../common/tqc_bb.h"
#include "../common/tqc_board_gpio.h"
#include "../common/tqc_eeprom.h"

#define MBA8X_BOARD_NAME "MBa8x"

DECLARE_GLOBAL_DATA_PTR;

enum {
	USB_RST_B,
	USB_OTG2_PWR,
	SWITCH_A,
	SWITCH_B,
	USER_LED0,
	USER_LED1,

	PCIE_CLK_PD,
	EN_3V3_MPCIE,
	EN_1V5_MPCIE,
	PCIE0_CLKREQ,
	PCIE0_PERST,
	PCIE0_WAKE,
	PCIE_DISABLE,
	PCIE1_CLKREQ,
	PCIE1_PERST,
	PCIE1_WAKE,

};

#define UART_PAD_CTRL	((SC_PAD_CONFIG_OUT_IN << PADRING_CONFIG_SHIFT) | \
			 (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
			 (SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | \
			 (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

static iomux_cfg_t uart0_pads[] = {
	SC_P_UART0_RX | MUX_MODE_ALT(0) | MUX_PAD_CTRL(UART_PAD_CTRL),
	SC_P_UART0_TX | MUX_MODE_ALT(0) | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx8_iomux_setup_multiple_pads(uart0_pads, ARRAY_SIZE(uart0_pads));
}

int tqc_bb_board_early_init_f(void)
{
	int ret;
	/* Set UART0 clock root to 80 MHz */
	sc_pm_clock_rate_t rate = 80000000;

	/* Power up UART0 */
	ret = sc_pm_set_resource_power_mode(-1, SC_R_UART_0, SC_PM_PW_MODE_ON);
	if (ret)
		return ret;

	ret = sc_pm_set_clock_rate(-1, SC_R_UART_0, 2, &rate);
	if (ret)
		return ret;

	/* Enable UART0 clock root */
	ret = sc_pm_clock_enable(-1, SC_R_UART_0, 2, true, false);
	if (ret)
		return ret;

	lpcg_all_clock_on(LPUART_0_LPCG);

	setup_iomux_uart();

/*
 * Dual bootloader feature will require CAAM access, but JR0 and JR1 will be
 * assigned to seco for imx8, use JR3 instead.
 */
#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_DUAL_BOOTLOADER)
	sc_pm_set_resource_power_mode(-1, SC_R_CAAM_JR3, SC_PM_PW_MODE_ON);
	sc_pm_set_resource_power_mode(-1, SC_R_CAAM_JR3_OUT, SC_PM_PW_MODE_ON);
#endif

	return 0;
}

#if !defined(CONFIG_SPL_BUILD)

const char *tqc_bb_get_boardname(void)
{
	return MBA8X_BOARD_NAME;
}

int tqc_bb_checkboard(void)
{
	return 0;
}

static struct tqc_gpio_init_data mba8x_gid[] = {
	GPIO_INIT_DATA_ENTRY(USB_RST_B, "GPIO2_7", GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(USB_OTG2_PWR, "GPIO4_4", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(SWITCH_A, "GPIO2_11", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(SWITCH_B, "GPIO1_0", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(USER_LED0, "GPIO5_20", GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(USER_LED1, "GPIO5_19", GPIOD_IS_OUT | GPIOD_ACTIVE_LOW),
	/* PCIE_CLK_PD#: keep clock initial low / Hi-Z */
	GPIO_INIT_DATA_ENTRY(PCIE_CLK_PD, "GPIO2_10", GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE),
	/* EN_3V3_MPCIE: off */
	GPIO_INIT_DATA_ENTRY(EN_3V3_MPCIE, "GPIO1_1", GPIOD_IS_OUT),
	/* EN_1V5_MPCIE: off */
	GPIO_INIT_DATA_ENTRY(EN_1V5_MPCIE, "GPIO0_31", GPIOD_IS_OUT),
	/* PCIE0_CLKREQ# */
	GPIO_INIT_DATA_ENTRY(PCIE0_CLKREQ, "GPIO4_27", GPIOD_IS_IN),
	/* PCIE0_PERST# */
	GPIO_INIT_DATA_ENTRY(PCIE0_PERST, "GPIO4_29", GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE),
	/* PCIE0_WAKE# */
	GPIO_INIT_DATA_ENTRY(PCIE0_WAKE, "GPIO4_28", GPIOD_IS_IN),
	/* PCIE_DISABLE# */
	GPIO_INIT_DATA_ENTRY(PCIE_DISABLE, "GPIO2_05", GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE),
	/* PCIE1_CLKREQ# */
	GPIO_INIT_DATA_ENTRY(PCIE1_CLKREQ, "GPIO4_30", GPIOD_IS_IN),
	/* PCIE1_PERST# */
	GPIO_INIT_DATA_ENTRY(PCIE1_PERST, "GPIO5_00", GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE),
	/* PCIE1_WAKE# */
	GPIO_INIT_DATA_ENTRY(PCIE1_WAKE, "GPIO4_31", GPIOD_IS_IN),
};

int tqc_bb_board_init(void)
{
	return tqc_board_gpio_init(mba8x_gid, ARRAY_SIZE(mba8x_gid));
}

#ifdef CONFIG_OF_BOARD_SETUP
int tqc_bb_ft_board_setup(void *blob, bd_t *bd)
{
	return 0;
}
#endif

/*
 * SD0 -> mmc0 / mmcblk0
 * SD1 -> mmc1 / mmcblk1
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

#if defined(CONFIG_USB)

int board_usb_init(int index, enum usb_init_type init)
{
	int ret = 0;
	struct gpio_desc *gpio;

	switch (index) {
	case 0:
		puts("USB_OTG1\n");
		break;
	case 1:
		puts("USB_OTG2/HUB\n");
		switch (init) {
		case USB_INIT_HOST:
			/* HUB reset */
			gpio = &mba8x_gid[USB_RST_B].desc;
			dm_gpio_set_value(gpio, 1);
			udelay(100);
			dm_gpio_set_value(gpio, 0);
			udelay(100);
			break;
		default:
			printf("USB_OTG2: only host supported on this board\n");
			ret = -EINVAL;
		}
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
	case 1:
		puts("USB_OTG2/HUB\n");
		/* deactivate HUB */
		gpio = &mba8x_gid[USB_RST_B].desc;
		dm_gpio_set_value(gpio, 1);
		break;
	case 0:
		puts("USB_OTG1\n");
		break;
	default:
		printf("invalid USB port %d\n", index);
		ret = -EINVAL;
	}

	return ret;
}
#endif

#endif /* #if !defined(CONFIG_SPL_BUILD) */

void board_quiesce_devices(void)
{
	static const char *power_on_devices[] = {
		"dma_lpuart0",
	};

	power_off_pd_devices(power_on_devices, ARRAY_SIZE(power_on_devices));
}
