// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <env.h>
#include <asm/arch/sys_proto.h>
#include <linux/sizes.h>

/* see MAX_CMDLINE_SIZE in boot/bootm.c */
#define BOARD_CMDLINE_SIZE SZ_4K
static char board_bootargs[BOARD_CMDLINE_SIZE];

char *board_fdt_chosen_bootargs(void)
{
#if IS_ENABLED(CONFIG_IMX8MM)
	static const char *cortexm_args = "clk-imx8mm.mcore_booted=1";
#elif IS_ENABLED(CONFIG_IMX8MN)
	static const char *cortexm_args = "clk-imx8mn.mcore_booted=1";
#elif IS_ENABLED(CONFIG_IMX8MP)
	static const char *cortexm_args = "clk-imx8mp.mcore_booted=1";
#elif IS_ENABLED(CONFIG_IMX8MQ)
	static const char *cortexm_args = "clk-imx8mq.mcore_booted=1";
#elif IS_ENABLED(CONFIG_IMX93)
	static const char *cortexm_args = "clk-imx93.mcore_booted=1";
#else
#error "CPU not supported"
#endif
	char *bootargs = env_get("bootargs");

	if (arch_auxiliary_core_check_up(0)) {
		board_bootargs[0] = '\0';
		strlcat(board_bootargs, bootargs, sizeof(board_bootargs));
		strlcat(board_bootargs, " ", sizeof(board_bootargs));
		strlcat(board_bootargs, cortexm_args, sizeof(board_bootargs));

		return board_bootargs;
	}

	return bootargs;
}
