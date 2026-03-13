// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <env.h>
#include <fdt_support.h>
#include <init.h>
#include <mtd_node.h>
#include <asm/global_data.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <jffs2/load_kernel.h>
#include <linux/errno.h>

#include "../common/tq_bb.h"

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	return tq_bb_board_early_init_f();
}

#if !IS_ENABLED(CONFIG_XPL_BUILD)

int board_init(void)
{
	return tq_bb_board_init();
}

int board_late_init(void)
{
	if (IS_ENABLED(CONFIG_ENV_IS_IN_MMC))
		board_late_mmc_env_init();

	if (IS_ENABLED(CONFIG_AHAB_BOOT))
		env_set("sec_boot", "yes");
	else
		env_set("sec_boot", "no");

	return tq_bb_board_late_init();
}

#if IS_ENABLED(CONFIG_OF_BOARD_SETUP)

int ft_board_setup(void *blob, struct bd_info *bd)
{
	if (IS_ENABLED(CONFIG_FDT_FIXUP_PARTITIONS)) {
		const char * const path = "/soc/bus@42000000/spi@425e0000";
		const struct node_info nodes[] = {
			{ "jedec,spi-nor",	MTD_DEV_TYPE_NOR, },
		};

		/*
		 * Update MTD partition nodes using info from
		 * mtdparts env var
		 * for [Q]SPI this needs the device probed.
		 */
		tq_ft_spi_setup(blob, path, nodes,
				ARRAY_SIZE(nodes));
	}

	return tq_bb_ft_board_setup(blob, bd);
}
#endif

void board_quiesce_devices(void)
{
	tq_bb_board_quiesce_devices();
}

/**
 * Translate detected CPU variant into TQ-Systems SOM variant short name
 *
 * return: string consisting of the name or string indicating unknown variant
 */
const char *tq_get_boardname(void)
{
	switch (get_cpu_type()) {
	case MXC_CPU_IMX95:
		return "TQMa95xxLA";
	default:
		return "??";
	}
}

static int print_bootinfo(void)
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
		puts("USDHC1(eMMC)\n");
		break;
	case MMC2_BOOT:
		puts("USDHC2(eMMC)\n");
		break;
	case MMC3_BOOT:
		puts("USDHC3(eMMC)\n");
		break;
	case USB_BOOT:
		puts("USB\n");
		break;
	case USB2_BOOT:
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

int checkboard(void)
{
	print_bootinfo();
	printf("Board: %s on a %s\n", tq_get_boardname(),
	       tq_bb_get_boardname());

	return tq_bb_checkboard();
}

#endif /* !IS_ENABLED(CONFIG_XPL_BUILD) */
