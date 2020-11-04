// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2020 TQ Systems GmbH
 * Markus Niebel <Markus.Niebel@tq-group.com>
 */

#include <common.h>
#include <asm/arch/sci/sci.h>

#include "tqc_scu.h"

static inline void decode_and_show_pmic(u32 data, int id)
{
	printf("PMIC%d: ID 0x%02x REV 0x%02x PROG_ID 0x%04x\n", id,
	       (data & 0xff), (data & 0xff00) >> 8, (data & 0xffff0000) >> 16);
}

int tqc_scu_checkpmic(bool dual)
{
	u32 fn, p1, p2;
	int ret;

	fn = SCU_IOCTL_PMICINFO;
	ret = sc_misc_board_ioctl(-1, &fn, &p1, &p2);
	if (!ret) {
		decode_and_show_pmic(p1, 0);
		if (dual)
			decode_and_show_pmic(p2, 1);
	} else {
		printf("ERROR: %d query PMIC info\n", ret);
	}

	return ret;
}

u64 tqc_scu_commitid(void)
{
	u32 fn, p1, p2;
	int ret;
	u64 id = 0xffffffffffffffff;

	fn = SCU_IOCTL_COMMITID;
	ret = sc_misc_board_ioctl(-1, &fn, &p1, &p2);
	if (!ret) {
		id = (u64)p1 | (u64)p2 << 32;
	} else {
		printf("ERROR: %d query SCU commit ID\n", ret);
	}

	return id;
}
