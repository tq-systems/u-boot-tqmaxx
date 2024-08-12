// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016-2024 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel, Steffen Doster
 */

#include <common.h>
#include <env.h>
#include <fdt_support.h>
#include <mtd_node.h>
#include <spi_flash.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>

#include "../common/tq_bb.h"
#include "../common/tq_eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();
	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	if (IS_ENABLED(CONFIG_FSL_QSPI))
		set_clk_qspi();

	return tq_bb_board_init();
}

static const char *tqma7_get_boardname(void)
{
	u32 cpurev = get_cpu_rev();

	switch ((cpurev & 0xFF000) >> 12) {
	case MXC_CPU_MX7S:
		return "TQMa7S";
	case MXC_CPU_MX7D:
		return "TQMa7D";
	default:
		return "??";
	};
}

int board_late_init(void)
{
	int ret;
	struct tq_eeprom_data eeprom;

	env_set_runtime("board_name", tqma7_get_boardname());

	ret = tq_read_module_eeprom(&eeprom);
	if (!ret)
		tq_board_handle_eeprom_data(tqma7_get_boardname(), &eeprom);
	else
		printf("EEPROM: err %d\n", ret);

	return tq_bb_board_late_init();
}

u32 get_board_rev(void)
{
	/* REV.0100 is unsupported */
	return 200;
}

int checkboard(void)
{
	printf("Board: %s rev %04u on %s\n", tqma7_get_boardname(),
	       get_board_rev(), tq_bb_get_boardname());
	return 0;
}

/*
 * Device Tree Support
 */
#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)

int ft_board_setup(void *blob, struct bd_info *bd)
{
	if (IS_ENABLED(CONFIG_TQ_SPI_NOR)) {
		const char * const path = "/soc@0/bus@30800000/spi@30bb0000";
		static const struct node_info nodes[] = {
			{ "jedec,spi-nor",	MTD_DEV_TYPE_NOR, },
		};
		/*
		 * Update MTD partition nodes using info
		 * from mtdparts env var
		 * for [Q]SPI this needs the device probed.
		 */
		puts("   Updating SPI NOR status...\n");
		tq_ft_spi_setup(blob, path, nodes, ARRAY_SIZE(nodes));
	}

	tq_bb_ft_board_setup(blob, bd);

	return 0;
}
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */
