/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Common helpers for TQ-Systems SOMs
 *
 * Copyright (c) 2025 TQ-Systems GmbH <u-boot@ew.tq-group.com>, D-82229 Seefeld, Germany.
 * Author: Nora Schiffer
 */

#ifndef __BOARD_TQ_COMMON_COMMON_H

#ifdef CONFIG_XPL_BUILD

#if IS_ENABLED(CONFIG_TQ_COMMON_SPL_BOARD_SETUP)
/* Implemented by board code */
void tq_common_spl_board_setup(void *blob);
#endif

#else

void tq_common_setup_boot_targets(void);

#endif

#endif
