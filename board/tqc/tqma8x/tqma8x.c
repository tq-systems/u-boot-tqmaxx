// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <bootm.h>
#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <netdev.h>
#include <fsl_ifc.h>
#include <fdt_support.h>
#include <mtd_node.h>
#include <jffs2/load_kernel.h>
#include <linux/libfdt.h>
#include <env.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/lpcg.h>
#include <asm/arch/sci/sci.h>
#include <asm/arch/imx8-pins.h>
#include <dm.h>
#include <asm/arch/iomux.h>
#include <asm/arch/sys_proto.h>
#include <power-domain.h>

#include "../common/tqc_bb.h"
#include "../common/tqc_eeprom.h"
#include "../common/tqc_scu.h"

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	tqc_bb_board_early_init_f();

	return 0;
}

static const char *tqma8x_get_boardname(void)
{
	switch (get_cpu_type()) {
	case MXC_CPU_IMX8QM:
		return "TQMa8QM";
	default:
		return "??";
	}

	return "UNKNOWN";
}

int checkboard(void)
{
	print_bootinfo();

	printf("Board: %s on a %s\n", tqma8x_get_boardname(),
	       tqc_bb_get_boardname());

	tqc_scu_checkpmic(true);
	tqc_scu_commitid();

	return tqc_bb_checkboard();
}

int board_init(void)
{
	tqc_bb_board_init();

	return 0;
}

void board_add_ram_info(int use_default)
{
	puts("\n");
	tqc_scu_checksdram();
}

/*
 * Board specific reset that is system reset.
 */
void reset_cpu(ulong addr)
{
/* TODO */
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, bd_t *bd)
{
	const char * const path = "/bus@5d000000/spi@5d120000";
	static const struct node_info nodes[] = {
		{ "jedec,spi-nor",	MTD_DEV_TYPE_NOR, },
	};

	tqc_ft_spi_setup(blob, path, nodes, ARRAY_SIZE(nodes));

	return tqc_bb_ft_board_setup(blob, bd);
}
#endif

int board_late_init(void)
{
#if !defined(CONFIG_SPL_BUILD)
	struct tqc_eeprom_data eeprom;
	const char *bname = tqma8x_get_boardname();

	if (!tqc_read_eeprom_at(1, 0x53, 1, 0, &eeprom))
		tqc_board_handle_eeprom_data(bname, &eeprom);
	else
		puts("EEPROM: read error\n");

	/* set quartz load to 7.000 femtofarads */
	tqc_pcf85063_adjust_capacity(1, 0x51, 7000);
#endif

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", tqc_bb_get_boardname());
	env_set("board_rev", tqma8x_get_boardname());
#endif

	tqc_bb_board_late_init();

	env_set("sec_boot", "no");
#ifdef CONFIG_AHAB_BOOT
	env_set("sec_boot", "yes");
#endif

	return 0;
}

