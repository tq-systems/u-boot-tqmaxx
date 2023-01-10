// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */
#ifndef __TQC_EMMC_H__
#define __TQC_EMMC_H__

int tqc_emmc_need_dsr(struct mmc *mmc);
int tqc_ft_fixup_emmc_dsr(void *blob, const char *path, u32 value);

#endif /* __TQC_EMMC_H__ */
