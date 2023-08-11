/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Definitions shared between TQMa62xx module and mainboards
 *
 * Copyright (c) 2022-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>, D-82229 Seefeld, Germany.
 * Author: Matthias Schiffer
 */

#ifndef __TQ_TQMA62XX_H
#define __TQ_TQMA62XX_H

#define TQMA62XX_BOOT_DEVICE_EMMC 0x100

u32 tqma62xx_get_boot_device(void);
void tqma62xx_setup_sysinfo(void);

#if IS_ENABLED(CONFIG_OF_LIBFDT) && IS_ENABLED(CONFIG_OF_BOARD_SETUP)
int tqma62xx_ft_board_setup(void *blob, struct bd_info *bd);
#endif

#endif
