/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2017 - 2022 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#ifndef __TQ_EMMC_H__
#define __TQ_EMMC_H__

int tq_emmc_need_dsr(const struct mmc *mmc);
int tq_ft_fixup_emmc_dsr(void *blob, const char *path, u32 value);

#endif /* __TQ_EMMC_H__ */
