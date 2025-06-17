/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Common sysinfo helpers for TQ-Systems SOMs
 *
 * Copyright (c) 2025 TQ-Systems GmbH <u-boot@ew.tq-group.com>, D-82229 Seefeld, Germany.
 * Author: Nora Schiffer
 */

#ifndef __BOARD_TQ_COMMON_SYSINFO_H

#ifndef CONFIG_XPL_BUILD

size_t tq_common_sysinfo_macaddr_num(void);
void tq_common_sysinfo_setup(void);
void tq_common_sysinfo_ft_board_setup(void *fdt);

#endif

#endif
