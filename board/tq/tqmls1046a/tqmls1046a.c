// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2023 TQ-Systems GmbH
 * Copyright (c) 2023 TQ-Systems GmbH <license@tq-group.com>, D-82229 Seefeld, Germany.
 * Author: Timo Herbrecher, Gregor Herburger
 *
 */

#include <common.h>
#include <env.h>
#include <init.h>
#include <asm/io.h>
#include <asm/arch/soc.h>
#include <fsl_sec.h>
#include <net.h>
#include "../common/tq_bb.h"
#include "../common/tq_eeprom.h"
#include "../common/tq_tqmls10xxa_common.h"

#ifndef CONFIG_SPL_BUILD
int checkboard(void)
{
	printf("Board: TQMLS1046A on a %s ", tq_bb_get_boardname());

	tq_tqmls10xx_checkboard();

	return tq_bb_checkboard();
}

int board_init(void)
{
#ifdef CONFIG_FSL_CAAM
	sec_init();
#endif
	pci_init();

	return tq_bb_board_init();
}

int fsl_board_late_init(void)
{
	struct tq_eeprom_data eeprom;
	char *bname = "TQMLS1046A";
	int ret;

	ret = tq_read_module_eeprom(&eeprom);

	if (!ret) {
		tq_board_handle_eeprom_data(bname, &eeprom);
		if (is_valid_ethaddr(eeprom.mac))
			tq_tqmls10xxa_set_macaddrs(eeprom.mac, 8);
		else
			puts("EEPROM: error: invalid mac address.\n");
	} else {
		printf("EEPROM: read error: %d\n", ret);
	}

	if (CONFIG_IS_ENABLED(ENV_VARS_UBOOT_RUNTIME_CONFIG))
		env_set("board_name", bname);

	return 0;
}
#endif
