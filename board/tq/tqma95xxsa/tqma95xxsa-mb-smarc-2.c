// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */
#include <env.h>
#include <asm/arch/sys_proto.h>

#include "../common/tq_bb.h"

#define BB_BOARD_NAME "MB-SMARC-2"

#if !defined(CONFIG_XPL_BUILD)

const char *tq_bb_get_boardname(void)
{
	return BB_BOARD_NAME;
}

int tq_bb_board_late_init(void)
{
	if (IS_ENABLED(CONFIG_ENV_IS_IN_MMC))
		board_late_mmc_env_init();

	return 0;
}

#endif
