// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2020 TQ-Systems GmbH
 * Markus Niebel <Markus.Niebel@tq-group.com>
 */

#ifndef __TQC_SCU_H__
#define __TQC_SCU_H__

enum {
	/* NXP example code */
	SCU_IOCTL_TEST = 0xfffffffe,
	/* PMIC ID / VERSION / PROG_ID */
	SCU_IOCTL_PMICINFO = 0xfffffffd,
	/* SCU commit ID */
	SCU_IOCTL_COMMITID = 0xfffffffc,
	/* Memory info */
	SCU_IOCTL_SDRAMINFO = 0xfffffffa,
};

#define SCU_DDRTYPE_UNKNOWN	0U
#define SCU_DDRTYPE_DDR3L	30U
#define SCU_DDRTYPE_LPDDR4	40U

#define SCU_DDRFEAT_ECC		0x80000000U
#define SCU_DDRFEAT_CBT		0x40000000U

int tqc_scu_checkpmic(bool dual);
int tqc_scu_commitid(void);
int tqc_scu_checksdram(void);

#endif
