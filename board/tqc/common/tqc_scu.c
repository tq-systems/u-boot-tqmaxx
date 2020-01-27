/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2020 TQ Systems GmbH
 * Markus Niebel <Markus.Niebel@tq-group.com>
 */

#include <common.h>
#include <asm/mach-imx/sci/sci.h>

#include "tqc_scu.h"

static inline void decode_and_show_pmic(uint32_t data, int id)
{
	printf("PMIC%d: ID 0x%02x REV 0x%02x PROG_ID 0x%04x\n", id,
	       (data & 0xff), (data & 0xff00) >> 8, (data & 0xffff0000) >> 16);
}

int tqc_scu_checkpmic(bool dual)
{
	uint32_t fn, p1,p2;
	int ret;

	fn = SCU_IOCTL_PMICINFO;
	ret = sc_misc_board_ioctl(SC_IPC_CH, &fn, &p1, &p2);
	if (!ret) {
		decode_and_show_pmic(p1, 0);
		if (dual)
			decode_and_show_pmic(p2, 1);
	} else {
		printf("ERROR: %d query PMIC info\n", ret);
	}

	return ret;
}
