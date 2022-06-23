/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Definitions shared between TQMa64XxL module and mainboards
 *
 * Copyright (c) 2022 TQ-Systems GmbH <license@tq-group.com>, D-82229 Seefeld, Germany.
 */

#ifndef __TQ_TQMA64XXL_H
#define __TQ_TQMA64XXL_H

u32 tqma64xxl_get_boot_device(void);
void tqma64xxl_parse_eeprom(void);

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int tqma64xxl_ft_board_setup(void *blob, struct bd_info *bd);
#endif

#endif
