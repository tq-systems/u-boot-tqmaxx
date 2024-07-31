// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2022-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <common.h>
#include <fdt_support.h>

#include "tq_som_features.h"

#if !defined(CONFIG_SPL_BUILD)

__weak const struct tq_som_feature_list *tq_board_detect_features(void)
{
	return NULL;
}

/* This code should be called from ft_board_setup */
void tq_ft_fixup_features(void *blob, const struct tq_som_feature_list *features)
{
	size_t i;
	int off;

	if (!features || !blob)
		return;

	for (i = 0; i < features->entries; ++i) {
		bool present = features->list[i].present;
		const char *path = features->list[i].dt_path;

		if (!present && path) {
			off = fdt_path_offset(blob, path);
			if (off >= 0) {
				pr_info("disable %s\n", path);
				fdt_set_node_status(blob, off, FDT_STATUS_DISABLED);
			}
		}
	}
}

#endif /* !defined(CONFIG_SPL_BUILD) */
