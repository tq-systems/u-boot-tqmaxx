// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2020 TQ-Systems GmbH
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

int tqc_scu_commitid(void)
{
	u32 fn, p1, p2;
	int ret;
	u64 id = 0xffffffffffffffffULL;

	fn = SCU_IOCTL_COMMITID;
	ret = sc_misc_board_ioctl(-1, &fn, &p1, &p2);
	if (!ret) {
		id = (u64)p1 | (u64)p2 << 32ULL;
		printf("SCU:   %llx\n", id);
	} else {
		printf("ERROR: %d query SCU commit ID\n", ret);
	}

	return ret;
}

int tqc_scu_checksdram(void)
{
	u32 fn, p1, p2;
	int ret;

	fn = SCU_IOCTL_SDRAMINFO;
	ret = sc_misc_board_ioctl(-1, &fn, &p1, &p2);
	if (!ret) {
		char *ddr_type = "unknown";
		u32 type = (p2 >> 16U) & 0xffU;

		switch (type) {
		case SCU_DDRTYPE_DDR3L:
			ddr_type = "DDR3L";
			break;
		case SCU_DDRTYPE_LPDDR4:
			ddr_type = "LPDDR4";
			break;
		default:
			printf("WARN: unknown type %u\n", type);
		}

		printf("SDRAM: %u MiB %s ", p2 & 0xffffU, ddr_type);
		puts((p2 & SCU_DDRFEAT_ECC) ? "ECC " : "- ");
		puts((p2 & SCU_DDRFEAT_CBT) ? "CBT " : "- ");
		printf("@ %u MHz (RPA %04u)\n", p1 & 0xffffU, (p1 >> 16U) & 0xffffU);
	} else {
		printf("ERROR: %d query SDRAM info\n", ret);
	}

	return ret;
}
