/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2020 - 2022 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#ifndef __TQ_SCU_H__
#define __TQ_SCU_H__

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

int tq_scu_checkpmic(bool dual);
int tq_scu_commitid(void);
int tq_scu_checksdram(void);

#endif
