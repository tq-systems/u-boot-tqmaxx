// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/uclass-internal.h>
#include <malloc.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm-generic/gpio.h>
#include <asm/arch/imx8mq_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/arch/clock.h>
#include <spl.h>

#if defined(CONFIG_FSL_QSPI)
#include <spi.h>
#include <spi_flash.h>
#endif

#include "../common/tqc_bb.h"
#include "../common/tqc_eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	return tqc_bb_board_early_init_f();
}

#if !defined(CONFIG_SPL_BUILD)

#if defined(CONFIG_FSL_QSPI) && defined(CONFIG_DM_SPI)

static int board_qspi_init(void)
{
	struct udevice *bus;
	struct uclass *uc;
	int count = 0;
	int ret;

	set_clk_qspi();

	ret = uclass_get(UCLASS_SPI, &uc);
	if (ret)
		return ret;

	uclass_foreach_dev(bus, uc) {
		/* init SPI controllers */
		printf("SPI%d:   ", count);
		count++;

		ret = device_probe(bus);
		if (ret == -ENODEV) {	/* No such device. */
			puts("SPI not available.\n");
			continue;
		}

		if (ret) {		/* Other error. */
			printf("probe failed, error %d\n", ret);
			continue;
		}

		puts("\n");
	}

	return 0;
}
#else
static inline int board_qspi_init(void) { return 0; }
#endif

#ifdef CONFIG_OF_BOARD_SETUP
static void tqma8mx_ft_qspi_setup(void *blob)
{
	int off;
	bool enable_flash = false;
	const char *path = "/soc@0/bus@30800000/spi@30bb0000";

	if (get_boot_device() == QSPI_BOOT) {
		enable_flash = true;
	} else {
#if defined(CONFIG_FSL_QSPI) && defined(CONFIG_SPI_FLASH)
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
	if (env_get("fdt_noauto")) {
		printf("   Skipping %s (fdt_noauto defined)\n", __func__);
		return 0;
	}

	tqma8mx_ft_qspi_setup(blob);

	return tqc_bb_ft_board_setup(blob, bd);
}
#endif

int board_init(void)
{
	board_qspi_init();

	return tqc_bb_board_init();
}

static const char *tqc_get_boardname(void)
{
	switch (get_cpu_type()) {
	case MXC_CPU_IMX8MD:
		return "TQMa8MD";
	case MXC_CPU_IMX8MQ:
		return "TQMa8MQ";
	case MXC_CPU_IMX8MQL:
		return "TQMa8MQL";
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
