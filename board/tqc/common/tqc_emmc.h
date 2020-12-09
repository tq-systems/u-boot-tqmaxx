// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2017 TQ-Systems GmbH
 * Markus Niebel <Markus.Niebel@tq-group.com>
 */
#ifndef __TQC_EMMC_H__
#define __TQC_EMMC_H__

int tqc_emmc_need_dsr(struct mmc *mmc);
int tqc_ft_fixup_emmc_dsr(void *blob, const char *path, u32 value);

#endif /* __TQC_EMMC_H__ */
