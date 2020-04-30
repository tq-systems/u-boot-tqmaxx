/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2020 TQ Systems GmbH
 * Markus Niebel <Markus.Niebel@tq-group.com>
 */

#ifndef __TQC_SCU_H__
#define __TQC_SCU_H__

enum {
	/* NXP example code */
	SCU_IOCTL_TEST = 0xfffffffe,
	/* PMIC ID / VERSION / PROG_ID */
	SCU_IOCTL_PMICINFO = 0xfffffffd,
};

int tqc_scu_checkpmic(bool dual);

#endif
