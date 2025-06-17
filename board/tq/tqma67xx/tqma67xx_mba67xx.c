// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 TQ-Systems GmbH <u-boot@ew.tq-group.com>, D-82229 Seefeld, Germany.
 * Author: Nora Schiffer
 */

#include <fdt_support.h>
#include <init.h>
#include <sysinfo.h>

#include "../common/common.h"
#include "../common/sysinfo.h"

int board_init(void)
{
	char revision[16] = "unknown";
	struct udevice *sysinfo;
	int ret;

	ret = sysinfo_get_by_seq_and_detect(&sysinfo, 1);
	if (!ret)
		sysinfo_get_str(sysinfo, SYSID_BOARD_MODEL, sizeof(revision), revision);

	printf("Board: MBa67xx REV %s\n", revision);

	return 0;
}

int board_late_init(void)
{
	tq_common_sysinfo_setup();
	tq_common_setup_boot_targets();
	return 0;
}

#if IS_ENABLED(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	tq_common_sysinfo_ft_board_setup(blob);
	return 0;
}
#endif
