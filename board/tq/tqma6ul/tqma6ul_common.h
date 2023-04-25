/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2020-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Matthias Schiffer
 */

#ifndef __TQMA6UL_COMMON__
#define __TQMA6UL_COMMON__

const char *tqma6ul_common_get_boardname(void);
int tqma6ul_common_checkboard(void);

int tqma6ul_common_early_init_f(void);
int tqma6ul_common_late_init(void);

#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
int tqma6ul_common_ft_board_setup(void *blob, struct bd_info *bd);
#endif

#endif
