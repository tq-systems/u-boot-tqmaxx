// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Definitions shared between TQMa65xx module and mainboards
 *
 * Copyright (C) 2022 TQ-Systems GmbH
 */

#ifndef __TQ_TQMA65XX_H
#define __TQ_TQMA65XX_H

u32 tqma65xx_get_boot_device(void);
void tqma65xx_parse_eeprom(void);

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int tqma65xx_ft_board_setup(void *blob, bd_t *bd);
#endif

#endif
