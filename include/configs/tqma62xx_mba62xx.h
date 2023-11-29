/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Configuration header file for TQ-Systems TQMa62xx on MBa62xx baseboard
 *
 * Copyright (c) 2020-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>, D-82229 Seefeld, Germany.
 */

#ifndef __CONFIG_TQMA62XX_MBA62XX_H
#define __CONFIG_TQMA62XX_MBA62XX_H

#include "tqma62xx.h"

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1) \
	func(SF, sf, 0) \
	func(USB, usb, 0) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

/* Default targets are managed by board code */
#undef BOOTENV_BOOT_TARGETS
#define BOOTENV_BOOT_TARGETS ""

#define CFG_EXTRA_ENV_SETTINGS \
	BOOTENV \
	BOOTENV_SF \
	"console=ttyS0\0"

#endif /* __CONFIG_TQMA62XX_MBA62XX_H */
