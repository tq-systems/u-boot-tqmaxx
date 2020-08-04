// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2018-2020 TQ Systems GmbH
 */
#include <bootm.h>
#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <netdev.h>
#include <fsl_ifc.h>
#include <fdt_support.h>
#include <linux/libfdt.h>
#include <environment.h>
#include <fsl_esdhc.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/lpcg.h>
#include <asm/arch/sci/sci.h>
#include <asm/arch/imx8-pins.h>
#include <dm.h>
#include <asm/arch/iomux.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
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

static const char *tqma8xxs_get_boardname(void)
{
	switch (get_cpu_type()) {
	case MXC_CPU_IMX8QXP:
		return "TQMa8XQPS";
		break;
	default:
		return "??";
	}

	return "UNKNOWN";
}

int checkboard(void)
{
	print_bootinfo();

	printf("Board: %s on a %s\n", tqma8xxs_get_boardname(),
	       tqc_bb_get_boardname());

	tqc_scu_checkpmic(false);

	return tqc_bb_checkboard();
}

int board_init(void)
{
	tqc_bb_board_init();

	return 0;
}

void detail_board_ddr_info(void)
{
	puts("\nDDR    ");
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
	return tqc_bb_ft_board_setup(blob, bd);
}
#endif

int board_late_init(void)
{
#if !defined(CONFIG_SPL_BUILD)
	int ret;

	struct tqc_eeprom_data eeprom;
	const char *bname = tqma8xxs_get_boardname();

	if ((get_cpu_rev() & 0xfff) == CHIP_REV_C) {
		ret = tqc_read_eeprom_at(0, 0x53, 1, 0, &eeprom);
	} else
		ret = tqc_read_eeprom_at(1, 0x51, 1, 0, &eeprom);

	if (!ret)
		tqc_board_handle_eeprom_data(bname, &eeprom);
	else
		puts("EEPROM: read error\n");
#endif

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", tqc_bb_get_boardname());
	env_set("board_rev", tqma8xxs_get_boardname());
#endif

	tqc_bb_board_late_init();

	env_set("sec_boot", "no");
#ifdef CONFIG_AHAB_BOOT
	env_set("sec_boot", "yes");
#endif

	return 0;
}

