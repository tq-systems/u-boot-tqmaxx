/*
 * Copyright 2018-2019 TQ Systems GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <dm.h>
#include <environment.h>
#include <errno.h>
#include <fdt_support.h>
#include <fsl_esdhc.h>
#include <fsl_ifc.h>
#include <linux/libfdt.h>
#include <malloc.h>
#include <netdev.h>
#include <power-domain.h>

#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/mach-imx/sci/sci.h>
#include <asm/arch/imx8-pins.h>
#include <asm/arch/iomux.h>
#include <asm/arch/lpcg.h>
#include <asm/arch/sys_proto.h>
#include <power-domain.h>

#include "../common/tqc_bb.h"

DECLARE_GLOBAL_DATA_PTR;

#define MB_SMARC_2_BOARD_NAME "MB-SMARC-2"

#define FSPI_PAD_CTRL	((SC_PAD_CONFIG_NORMAL << PADRING_CONFIG_SHIFT) | (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) \
						| (SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define UART_PAD_CTRL	((SC_PAD_CONFIG_OUT_IN << PADRING_CONFIG_SHIFT) | (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) \
						| (SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

static iomux_cfg_t uart0_pads[] = {
	SC_P_UART1_RX | MUX_MODE_ALT(0) | MUX_PAD_CTRL(UART_PAD_CTRL),
	SC_P_UART1_TX | MUX_MODE_ALT(0) | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx8_iomux_setup_multiple_pads(uart0_pads, ARRAY_SIZE(uart0_pads));
}

int tqc_bb_board_early_init_f(void)
{
	sc_ipc_t ipcHndl = 0;
	sc_err_t sciErr = 0;

	ipcHndl = gd->arch.ipc_channel_handle;

	/* Power up UART0 */
	sciErr = sc_pm_set_resource_power_mode(ipcHndl, SC_R_UART_0, SC_PM_PW_MODE_ON);
	if (sciErr != SC_ERR_NONE)
		return 0;

	/* Set UART0 clock root to 80 MHz */
	sc_pm_clock_rate_t rate = 80000000;
	sciErr = sc_pm_set_clock_rate(ipcHndl, SC_R_UART_0, 2, &rate);
	if (sciErr != SC_ERR_NONE)
		return 0;

	/* Enable UART0 clock root */
	sciErr = sc_pm_clock_enable(ipcHndl, SC_R_UART_0, 2, true, false);
	if (sciErr != SC_ERR_NONE)
		return 0;

	LPCG_AllClockOn(LPUART_0_LPCG);

	setup_iomux_uart();

	return 0;
}

int tqc_bb_checkboard(void)
{
	return 0;
}

int tqc_bb_board_init(void)
{
	return 0;
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

const char *tqc_bb_get_boardname(void)
{
	return MB_SMARC_2_BOARD_NAME;
}

int tqc_bb_board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", MB_SMARC_2_BOARD_NAME);
	env_set("board_rev", "iMX8QXP");
#endif

#ifdef CONFIG_ENV_IS_IN_MMC
	board_late_mmc_env_init();
#endif

	return 0;
}

void board_quiesce_devices()
{
	const char *power_on_devices[] = {
		"dma_lpuart0",

		/* HIFI DSP boot */
		"audio_sai0",
		"audio_ocram",
	};

	power_off_pd_devices(power_on_devices, ARRAY_SIZE(power_on_devices));
}
