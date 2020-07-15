// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 TQ Systems GmbH
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/uclass-internal.h>
#ifdef CONFIG_IMX8MN
#include <asm/arch/imx8mn_pins.h>
#elif defined(CONFIG_IMX8MM)
#include <asm/arch/imx8mm_pins.h>
#else
#error
#endif
#include <malloc.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm-generic/gpio.h>
#include <fsl_esdhc.h>
#include <mmc.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/arch/clock.h>
#include <spl.h>

#if defined(CONFIG_FSL_FSPI)
#include <spi.h>
#include <spi_flash.h>
#endif

#include "../common/tqc_bb.h"
#include "../common/tqc_eeprom.h"

#define WDOG_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS)

#ifdef CONFIG_IMX8MN

static iomux_v3_cfg_t const wdog_pads[] = {
	IMX8MN_PAD_GPIO1_IO02__WDOG1_WDOG_B | MUX_PAD_CTRL(WDOG_PAD_CTRL),
};

#elif defined(CONFIG_IMX8MM)

static iomux_v3_cfg_t const wdog_pads[] = {
	IMX8MM_PAD_GPIO1_IO02_WDOG1_WDOG_B | MUX_PAD_CTRL(WDOG_PAD_CTRL),
};

#else
#error
#endif

DECLARE_GLOBAL_DATA_PTR;
int board_early_init_f(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;

	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));
	set_wdog_reset(wdog);

	tqc_bb_board_early_init_f();

	return 0;
}

int dram_init(void)
{
	/* rom_pointer[1] contains the size of TEE occupies */
	if (rom_pointer[1])
		gd->ram_size = PHYS_SDRAM_SIZE - rom_pointer[1];
	else
		gd->ram_size = PHYS_SDRAM_SIZE;

	return 0;
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, bd_t *bd)
{
	return tqc_bb_ft_board_setup(blob, bd);
}
#endif

#ifdef CONFIG_FSL_FSPI
int board_qspi_init(void)
{
	return set_clk_qspi();
}
#endif

int board_init(void)
{
#ifdef CONFIG_FSL_FSPI
	board_qspi_init();
#endif
	return tqc_bb_board_init();
}

static const char *tqma8mxx_get_boardname(void)
{
	switch (get_cpu_type()) {
	case MXC_CPU_IMX8MM:
		return "TQMa8MQML";
	case MXC_CPU_IMX8MML:
		return "TQMa8MQMLL";
	case MXC_CPU_IMX8MMD:
		return "TQMa8MDML";
	case MXC_CPU_IMX8MMDL:
		return "TQMa8MDMLL";
	case MXC_CPU_IMX8MMS:
		return "TQMa8MSML";
	case MXC_CPU_IMX8MMSL:
		return "TQMa8MSMLL";
	case MXC_CPU_IMX8MN:
		return "TQMa8MQNL";
	case MXC_CPU_IMX8MNL:
		return "TQMa8MQNLL";
	case MXC_CPU_IMX8MND:
		return "TQMa8MDNL";
	case MXC_CPU_IMX8MNDL:
		return "TQMa8MDNLL";
	case MXC_CPU_IMX8MNS:
		return "TQMa8MSNL";
	case MXC_CPU_IMX8MNSL:
		return "TQMa8MSNLL";
	default:
		return "??";
	};
	return "UNKNOWN";
}

int print_bootinfo(void)
{
	enum boot_device bt_dev;

	bt_dev = get_boot_device();

	puts("Boot:  ");
	switch (bt_dev) {
	case SD1_BOOT:
		puts("SD0\n");
		break;
	case SD2_BOOT:
		puts("SD1\n");
		break;
	case MMC1_BOOT:
		puts("MMC0\n");
		break;
	case MMC2_BOOT:
		puts("MMC1\n");
		break;
	case USB_BOOT:
		puts("USB\n");
		break;
	default:
		printf("Unknown/Unsupported device %u\n", bt_dev);
		break;
	}

	return 0;
}

int board_late_init(void)
{
#if !defined(CONFIG_SPL_BUILD)
	struct tqc_eeprom_data eeprom;
	const char *bname = tqma8mxx_get_boardname();

	if (!tqc_read_eeprom_at(0, 0x53, 1, 0, &eeprom))
		tqc_board_handle_eeprom_data(bname, &eeprom);
	else
		puts("EEPROM: read error\n");

	/* set quartz load to 7.000 femtofarads */
	tqc_pcf85063_adjust_capacity(0, 0x51, 7000);
#endif

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", tqc_bb_get_boardname());
	env_set("board_rev", tqma8mxx_get_boardname());
#endif

	return tqc_bb_board_late_init();
}

int checkboard(void)
{
	print_bootinfo();
	printf("Board: %s on a %s\n", tqma8mxx_get_boardname(),
	       tqc_bb_get_boardname());
	return 0;
}
