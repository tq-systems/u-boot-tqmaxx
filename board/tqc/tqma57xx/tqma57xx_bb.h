/*
 * Copyright (C) 2017 TQ-Systems GmbH
 *
 * Author: Stefan Lange <s.lange@gateware.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __TQMA57XX_BB__
#define __TQMA57XX_BB__

#include <common.h>

const char *tqma57xx_bb_get_boardname(void);
int tqma57xx_bb_recalibrate_iodelay(void);
int tqma57xx_bb_board_init(void);
int tqma57xx_bb_board_mmc_init(bd_t *bis);
int tqma57xx_bb_board_eth_init(bd_t *bis);
int tqma57xx_bb_board_usb_init(void);
void tqma57xx_bb_board_late_init(void);
int tqma57xx_bb_ft_board_setup(void *blob, bd_t *bd);

#endif /* __TQMA57XX_BB__ */
