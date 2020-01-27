/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2020 TQ Systems GmbH
 * Markus Niebel <Markus.Niebel@tq-group.com>
 */

#ifndef __TQC_SCU_H__
#define __TQC_SCU_H__

enum {
	SCU_IOCTL_TEST = 0xfffffffe,		/* NXP example code */
	SCU_IOCTL_PMICINFO = 0xfffffffd,	/* PMIC ID / VERSION / PROG_ID */
};

int tqc_scu_checkpmic(bool dual);

#endif
