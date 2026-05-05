/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Max Merchel
 */

#ifndef __TQ_I2C_H__
#define __TQ_I2C_H__

#include <log.h>
#include <linux/types.h>

/*
 * tq_i2c_detect_fixup_temp_compatible - detect tempsensor and adjust compatible
 *
 * @blob:	Pointer to device tree
 * @bus:	Bus number to examine
 * @addr:	Chip address for the new device
 * @offset_len:	Length of a register offset in bytes (normally 1)
 * @fdt_path:	Path to device tree node
 */
int tq_i2c_detect_fixup_temp_compatible(void *blob, int bus, int addr,
					uint offset_len,
					const char *fdt_path);

#endif /* __TQ_I2C_H__ */
