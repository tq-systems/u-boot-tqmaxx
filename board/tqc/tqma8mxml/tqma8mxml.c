// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Martin Schmiedel
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

#if !defined(CONFIG_SPL_BUILD)

#ifdef CONFIG_OF_BOARD_SETUP

static void tqma8mxml_ft_qspi_setup(void *blob)
{
	int off;
	bool enable_flash = false;
	const char *path = "/soc@0/bus@30800000/spi@30bb0000";

	if (get_boot_device() == QSPI_BOOT) {
		enable_flash = true;
	} else {
#if defined(CONFIG_FSL_FSPI) && defined(CONFIG_SPI_FLASH)
		unsigned int bus = CONFIG_SF_DEFAULT_BUS;
		unsigned int cs = CONFIG_SF_DEFAULT_CS;
		unsigned int speed = CONFIG_SF_DEFAULT_SPEED;
		unsigned int mode = CONFIG_SF_DEFAULT_MODE;
#ifdef CONFIG_DM_SPI_FLASH
		struct udevice *new, *bus_dev;
		int ret;

		/* Remove the old device, otherwise probe will just be a nop */
		ret = spi_find_bus_and_cs(bus, cs, &bus_dev, &new);
		if (!ret)
			device_remove(new, DM_REMOVE_NORMAL);

		ret = spi_flash_probe_bus_cs(bus, cs, speed, mode, &new);
		if (!ret) {
			device_remove(new, DM_REMOVE_NORMAL);
			enable_flash = 1;
		}
#else
		struct spi_flash *new;

		new = spi_flash_probe(bus, cs, speed, mode);
		if (new) {
			spi_flash_free(new);
			enable_flash = 1;
		}
#endif
#endif
	}

	off = fdt_path_offset(blob, path);
	if (off >= 0) {
		printf("%s %s\n", (enable_flash) ? "enable" : "disable",
		       path);
		fdt_set_node_status(blob, off, (enable_flash) ?
				    FDT_STATUS_OKAY : FDT_STATUS_DISABLED,
				    0);
	}
}

int ft_board_setup(void *blob, bd_t *bd)
{
	tqma8mxml_ft_qspi_setup(blob);

	return tqc_bb_ft_board_setup(blob, bd);
}
#endif

int board_init(void)
{
	return tqc_bb_board_init();
}

static const char *tqc_get_boardname(void)
{
	switch (get_cpu_type()) {
	case MXC_CPU_IMX8MM:
		return "TQMa8MQML";
	case MXC_CPU_IMX8MML:
		return "TQMa8MQLML";
	case MXC_CPU_IMX8MMD:
		return "TQMa8MDML";
	case MXC_CPU_IMX8MMDL:
		return "TQMa8MDLML";
	case MXC_CPU_IMX8MMS:
		return "TQMa8MSML";
	case MXC_CPU_IMX8MMSL:
		return "TQMa8MSLML";
	case MXC_CPU_IMX8MN:
		return "TQMa8MQNL";
	case MXC_CPU_IMX8MNL:
		return "TQMa8MQLNL";
	case MXC_CPU_IMX8MND:
		return "TQMa8MDNL";
	case MXC_CPU_IMX8MNDL:
		return "TQMa8MDLNL";
	case MXC_CPU_IMX8MNS:
		return "TQMa8MSNL";
	case MXC_CPU_IMX8MNSL:
		return "TQMa8MSLNL";
	default:
		return "??";
	}
	return "UNKNOWN";
}

int print_bootinfo(void)
{
	enum boot_device bt_dev;

	bt_dev = get_boot_device();

	puts("Boot:  ");
	switch (bt_dev) {
	case SD1_BOOT:
		puts("USDHC1(SD)\n");
		break;
	case SD2_BOOT:
		puts("USDHC2(SD)\n");
		break;
	case SD3_BOOT:
		puts("USDHC3(SD)\n");
		break;
	case MMC1_BOOT:
		puts("USDHC1(e-MMC)\n");
		break;
	case MMC2_BOOT:
		puts("USDHC2(e-MMC)\n");
		break;
	case MMC3_BOOT:
		puts("USDHC3(e-MMC)\n");
		break;
	case USB_BOOT:
		puts("USB\n");
		break;
	case QSPI_BOOT:
		puts("FlexSPI\n");
		break;
	default:
		printf("Unknown/Unsupported device %u\n", bt_dev);
		break;
	}

	return 0;
}

int board_late_init(void)
{
	struct tqc_eeprom_data eeprom;
	const char *bname = tqc_get_boardname();

	if (!tqc_read_eeprom_at(0, 0x53, 1, 0, &eeprom))
		tqc_board_handle_eeprom_data(bname, &eeprom);
	else
		puts("EEPROM: read error\n");

	/* set quartz load to 7.000 femtofarads */
	tqc_pcf85063_adjust_capacity(0, 0x51, 7000);

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", tqc_bb_get_boardname());
	env_set("board_rev", tqc_get_boardname());
#endif

	return tqc_bb_board_late_init();
}

int checkboard(void)
{
	print_bootinfo();
	printf("Board: %s on a %s\n", tqc_get_boardname(),
	       tqc_bb_get_boardname());
	return 0;
}

#endif
