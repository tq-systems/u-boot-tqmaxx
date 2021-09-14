/*
 * (C) Copyright 2017 TQ Systems GmbH
 * Markus Niebel <Markus.Niebel@tq-group.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __TQC_EMMC_H__
#define __TQC_EMMC_H__

int tqc_emmc_need_dsr(struct mmc *mmc);
void tqc_ft_fixup_emmc_dsr(void *blob, const char *path, u32 value);
int tqc_ft_try_fixup_emmc_dsr(void *blob, const char * const *path_list,
			      size_t list_len, u32 dsr_value);

#endif /* __TQC_EMMC_H__ */
