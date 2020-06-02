// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 TQ Systems GmbH
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx8mm_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <errno.h>
#include <fsl_esdhc.h>
#include <mmc.h>
#include <spl.h>

#include "../common/tqc_bb.h"

DECLARE_GLOBAL_DATA_PTR;

#if (CONFIG_MXC_UART_BASE == UART1_BASE_ADDR)
static const u32 uart_index = 0;
#elif (CONFIG_MXC_UART_BASE == UART3_BASE_ADDR)
static const u32 uart_index = 2;
#elif (CONFIG_MXC_UART_BASE == UART2_BASE_ADDR)
static const u32 uart_index = 1;
#else
#error
#endif

#define USDHC2_CD_GPIO	IMX_GPIO_NR(2, 12)

int tqc_bb_board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	debug("tqc_bb_board_mmc_getcd\n");
	switch (cfg->esdhc_base) {
	case USDHC2_BASE_ADDR:
		ret = !gpio_get_value(USDHC2_CD_GPIO);
		return ret;
	}

	return ret;
}

#define USDHC_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS | \
			 PAD_CTL_FSEL2)
#define USDHC_GPIO_PAD_CTRL (PAD_CTL_HYS | PAD_CTL_DSE1)

#define USDHC2_VSELECT_GPIO	IMX_GPIO_NR(1, 4)

static iomux_v3_cfg_t const usdhc2_pads[] = {
	IMX8MM_PAD_SD2_CLK_USDHC2_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0xd6 */
	IMX8MM_PAD_SD2_CMD_USDHC2_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0xd6 */
	IMX8MM_PAD_SD2_DATA0_USDHC2_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0xd6 */
	IMX8MM_PAD_SD2_DATA1_USDHC2_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0xd6 */
	IMX8MM_PAD_SD2_DATA2_USDHC2_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0x16 */
	IMX8MM_PAD_SD2_DATA3_USDHC2_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0xd6 */
	IMX8MM_PAD_SD2_CD_B_GPIO2_IO12 | MUX_PAD_CTRL(USDHC_GPIO_PAD_CTRL),
	IMX8MM_PAD_GPIO1_IO04_USDHC2_VSELECT | MUX_PAD_CTRL(USDHC_GPIO_PAD_CTRL),
};

static struct fsl_esdhc_cfg usdhc2_cfg = {
	.esdhc_base = USDHC2_BASE_ADDR,
	.max_bus_width = 4,
};

int tqc_bb_board_mmc_init(bd_t *bis)
{
	int ret;

	debug("tqc_bb_board_mmc_init\n");
	/*
	 * According to the board_mmc_init() the following map is done:
	 * (U-Boot device node)    (Physical Port)
	 * mmc0                    USDHC1
	 * mmc1                    USDHC2
	 */
	init_clk_usdhc(1);
	usdhc2_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
	imx_iomux_v3_setup_multiple_pads(
		usdhc2_pads, ARRAY_SIZE(usdhc2_pads));
	/*
	 * even if we do not use VSELECT / UHS set to default state, requesting
	 * 3.3 Volts
	 */
	gpio_request(USDHC2_VSELECT_GPIO, "usdhc2_vselect");
	gpio_direction_output(USDHC2_VSELECT_GPIO, 0);
	udelay(500);

	ret = fsl_esdhc_initialize(bis, &usdhc2_cfg);
	if (ret)
		return ret;

	return 0;
}

/*
 * In case of boot from SD1 (e-MMC) or SD2 (SD-Card) we must correct the boot
 * device settings, since the i.MX8MM CPU code will return BOOT_DEVICE_MMC1
 * for SD1 and SD2 which is wrong for our board. Restore sane default with
 * our own mapping, BOOT_DEVICE_MMC1/2 is used in spl_mmc to query the index
 * of the MMC device.
 */
void board_boot_order(u32 *spl_boot_list)
{
	enum boot_device boot_device_spl = get_boot_device();

	switch (boot_device_spl) {
	case SD1_BOOT:
	case MMC1_BOOT:
		puts("board_boot_order SD1 -> MMC1\n");
		spl_boot_list[0] = BOOT_DEVICE_MMC1;
		break;
	case SD2_BOOT:
	case MMC2_BOOT:
		puts("board_boot_order SD2 -> MMC2\n");
		spl_boot_list[0] = BOOT_DEVICE_MMC2;
		break;
	default:
		spl_boot_list[0] = spl_boot_device();
	}
}

void tqc_bb_spl_board_init(void)
{
	debug("tqc_bb_spl_board_init\n");
}

void tqc_bb_board_init_f(ulong dummy)
{
	debug("tqc_bb_board_init_f\n");

	init_uart_clk(uart_index);
}
