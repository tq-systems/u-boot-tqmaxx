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

#include "../common/tqc_bb.h"
#include "../common/tqc_board_gpio.h"
#include "../common/tqc_eeprom.h"

#define MBA8XX_BOARD_NAME "MBa8Xx"

DECLARE_GLOBAL_DATA_PTR;

enum {
	SWITCH_A_N,
	SWITCH_B_N,
	LED_A,
	LED_B,
	GPIO1_IO26,
	GPIO3_IO15,
	PE_INT_N,
	DSI_EN,
	USB_RESET_N,
	PCIE_DIS_N,
	LCD_RESET_N,
	LCD_BLT_EN,
	LCD_PWR_EN,
	ENET0_INT_N,
	ENET1_INT_N,
	MIPI_CSI_EN,
	MIPI_CSI_RST_N,
};

#define UART_PAD_CTRL	((SC_PAD_CONFIG_OUT_IN << PADRING_CONFIG_SHIFT) | \
			 (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
			 (SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | \
			 (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define PCIE_PAD_CTRL	((SC_PAD_CONFIG_OD_IN << PADRING_CONFIG_SHIFT))

static const iomux_cfg_t board_pcie_pins[] = {
	SC_P_PCIE_CTRL0_CLKREQ_B | MUX_MODE_ALT(0) | MUX_PAD_CTRL(PCIE_PAD_CTRL),
	SC_P_PCIE_CTRL0_WAKE_B | MUX_MODE_ALT(0) | MUX_PAD_CTRL(PCIE_PAD_CTRL),
	SC_P_PCIE_CTRL0_PERST_B | MUX_MODE_ALT(0) | MUX_PAD_CTRL(PCIE_PAD_CTRL),
};

static void setup_iomux_pcie(void)
{
	imx8_iomux_setup_multiple_pads(board_pcie_pins, ARRAY_SIZE(board_pcie_pins));
}

static iomux_cfg_t uart1_pads[] = {
	SC_P_UART1_RX | MUX_MODE_ALT(0) | MUX_PAD_CTRL(UART_PAD_CTRL),
	SC_P_UART1_TX | MUX_MODE_ALT(0) | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx8_iomux_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
}

int tqc_bb_board_early_init_f(void)
{
	int ret;
	/* Set UART1 clock root to 80 MHz */
	sc_pm_clock_rate_t rate = 80000000;

	/* Power up UART1 */
	ret = sc_pm_set_resource_power_mode(-1, SC_R_UART_1, SC_PM_PW_MODE_ON);
	if (ret)
		return ret;

	ret = sc_pm_set_clock_rate(-1, SC_R_UART_1, 2, &rate);
	if (ret)
		return ret;

	/* Enable UART1 clock root */
	ret = sc_pm_clock_enable(-1, SC_R_UART_1, 2, true, false);
	if (ret)
		return ret;

	lpcg_all_clock_on(LPUART_1_LPCG);

	setup_iomux_uart();

/* Dual bootloader feature will require CAAM access, but JR0 and JR1 will be
 * assigned to seco for imx8, use JR3 instead.
 */
#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_DUAL_BOOTLOADER)
	sc_pm_set_resource_power_mode(-1, SC_R_CAAM_JR3, SC_PM_PW_MODE_ON);
	sc_pm_set_resource_power_mode(-1, SC_R_CAAM_JR3_OUT, SC_PM_PW_MODE_ON);
#endif

	setup_iomux_pcie();

	return 0;
}

#if !defined(CONFIG_SPL_BUILD)

const char *tqc_bb_get_boardname(void)
{
	return MBA8XX_BOARD_NAME;
}

int tqc_bb_checkboard(void)
{
	return 0;
}

static struct tqc_gpio_init_data mba8xx_gid[] = {
	GPIO_INIT_DATA_ENTRY(SWITCH_A_N, "GPIO1_13", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(SWITCH_B_N, "GPIO1_14", GPIOD_IS_IN),

	GPIO_INIT_DATA_ENTRY(GPIO1_IO26, "GPIO1_26", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(GPIO3_IO15, "GPIO3_15", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(PE_INT_N, "GPIO4_19", GPIOD_IS_IN),

	GPIO_INIT_DATA_ENTRY(LED_A, "gpio@70_1", GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(LED_B, "gpio@70_2", GPIOD_IS_OUT),

	GPIO_INIT_DATA_ENTRY(DSI_EN, "gpio@70_4", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(USB_RESET_N, "gpio@70_5", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(PCIE_DIS_N, "gpio@70_7", GPIOD_IS_OUT),

	GPIO_INIT_DATA_ENTRY(LCD_RESET_N, "GPIO1_29", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(LCD_BLT_EN, "GPIO1_30", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(LCD_PWR_EN, "GPIO1_25", GPIOD_IS_OUT),

	GPIO_INIT_DATA_ENTRY(ENET0_INT_N, "GPIO3_0", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(ENET1_INT_N, "GPIO3_1", GPIOD_IS_IN),

	GPIO_INIT_DATA_ENTRY(MIPI_CSI_EN, "GPIO3_8", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(MIPI_CSI_RST_N, "GPIO3_7", GPIOD_IS_IN),
};

int tqc_bb_board_init(void)
{
	return tqc_board_gpio_init(mba8xx_gid, ARRAY_SIZE(mba8xx_gid));
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

#endif /* #if !defined(CONFIG_SPL_BUILD) */

void board_quiesce_devices(void)
{
	static const char *power_on_devices[] = {
		"dma_lpuart1",

		/* HIFI DSP boot */
		"audio_sai0",
		"audio_ocram",
	};

	power_off_pd_devices(power_on_devices, ARRAY_SIZE(power_on_devices));
}
