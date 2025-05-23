// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <common.h>
#include <fdt_support.h>

#include "tq_som_features.h"

/* All this code must be called from ft_board_setup */
#if defined(CONFIG_OF_BOARD_SETUP) && !defined(CONFIG_SPL_BUILD)

__weak struct tq_som_feature_list *tq_board_detect_features(void)
{
	return NULL;
}

void tq_ft_fixup_features(void *blob,
			  const struct tq_som_feature_list *features)
{
	size_t i;
	int off;

	if (!features || !blob)
		return;

	for (i = 0; i < features->entries; ++i) {
		bool present = features->list[i].present;
		const char *path = features->list[i].dt_path;

		if (path) {
			off = fdt_path_offset(blob, path);
			if (off >= 0) {
				pr_info("%s %s\n",
					(present) ? "enable" : "disable",
					path);
				fdt_set_node_status(blob, off, present ?
						    FDT_STATUS_OKAY : FDT_STATUS_DISABLED,
						    0);
			}
		}
	}
}

#endif /* defined(CONFIG_OF_BOARD_SETUP) && !defined(CONFIG_SPL_BUILD) */
