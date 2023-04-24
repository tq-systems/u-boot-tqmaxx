// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 * Copyright (c) 2018-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <common.h>
#include <env.h>
#include <mmc.h>

#include "tq_sdmmc.h"

static bool check_mmc_autodetect(void)
{
	char *autodetect_str = env_get("mmcautodetect");

	if (autodetect_str && (strcmp(autodetect_str, "yes") == 0))
		return true;

	return false;
}

void tq_sdmmc_env_init(void)
{
	if (check_mmc_autodetect())
		env_set_ulong("mmcdev", mmc_get_env_dev());
}
