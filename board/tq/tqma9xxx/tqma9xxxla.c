// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <common.h>
#include <bloblist.h>
#include <env.h>
#include <init.h>
#include <spl.h>
#include <jffs2/load_kernel.h>
#include <asm/global_data.h>
#include <asm/arch-imx9/ccm_regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/arch-imx9/imx93_pins.h>
#include <asm/arch/clock.h>
#include <mtd_node.h>
#include <spl.h>
#include <dm/device.h>
#include <dm/uclass.h>

#include "../common/tq_bb.h"
#include "../common/tq_blob.h"
#include "../common/tq_eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	tq_bb_board_early_init_f();

	return 0;
}

#if !defined(CONFIG_SPL_BUILD)

/**
 * board specific version to enable support for multiple RAM size/type
 */
int board_phys_sdram_size(phys_size_t *size)
{
	struct tq_raminfo *raminfo;

	if (!size)
		return -EINVAL;

	raminfo = bloblist_find(BLOBLISTT_TQ_RAMSIZE, sizeof(*raminfo));
	if (!raminfo) {
		printf("Unable to find RAMSIZE blob from SPL\n");
		return -ENOENT;
	}

	*size = raminfo->memsize;

	return 0;
}

/**
 * Translate detected CPU variant into TQ-Systems SOM variant short name
 *
 * return: string consisting of the name or string indicating unknown variant
 */
static const char *tq_get_boardname(void)
{
	switch (get_cpu_type()) {
	/* all normal dual core variants */
	case MXC_CPU_IMX93:
		return "TQMa9352LA";
	/* add more if new variants will be assembled */
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

int checkboard(void)
{
	print_bootinfo();
	printf("Board: %s on a %s\n", tq_get_boardname(),
	       tq_bb_get_boardname());

	return tq_bb_checkboard();
}

int board_init(void)
{
	tq_bb_board_init();

	return 0;
}

#if CONFIG_IS_ENABLED(OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	const char * const path = "/soc@0/bus@42000000/spi@425e0000";
	static const struct node_info nodes[] = {
		{ "jedec,spi-nor",	MTD_DEV_TYPE_NOR, },
		{ "nxp,imx8qxp-fspi",	MTD_DEV_TYPE_NOR, },
	};

	tq_ft_spi_setup(blob, path, nodes, ARRAY_SIZE(nodes));

	return tq_bb_ft_board_setup(blob, bd);
}
#endif

int board_late_init(void)
{
	struct tq_eeprom_data eeprom;
	const char *bname = tq_get_boardname();
	int ret;

	ret = tq_read_module_eeprom(&eeprom);

	if (!ret) {
		tq_board_handle_eeprom_data(bname, &eeprom);
		if (tq_vard_valid(&eeprom.tq_hw_data.vard)) {
			/*
			 * set quartz load to 7.000 femtofarads
			 * only if RTC is assembled, to prevent warnings
			 */
			if (tq_vard_has_rtc(&eeprom.tq_hw_data.vard)) {
				/* set quartz load to 7.000 femtofarads */
				if (tq_pcf85063_adjust_capacity(0, 0x51,
								7000))
					puts("PCF85063: adjust error\n");
			}
		}
	} else {
		puts("EEPROM: read error\n");
	}

	if (CONFIG_IS_ENABLED(ENV_VARS_UBOOT_RUNTIME_CONFIG)) {
		env_set("board_name", tq_bb_get_boardname());
		env_set("board_rev", tq_get_boardname());

		if (CONFIG_IS_ENABLED(AHAB_BOOT))
			env_set("sec_boot", "yes");
		else
			env_set("sec_boot", "no");
	}

	tq_bb_board_late_init();

	return 0;
}

#endif
