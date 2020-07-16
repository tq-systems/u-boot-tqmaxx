/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration settings for the TQ Systems MBLS1012AL Carrier board
 * for TQMLS1012AL
 *
 * Copyright (C) 2018 TQ Systems GmbH
 * Max Merchel <Max.Merchel@tq-group.com>
 */

#ifndef __CONFIG_TQMLS1012AL_MBLS1012AL_H__
#define __CONFIG_TQMLS1012AL_MBLS1012AL_H__

/* PCIE */
#define CONFIG_PCIE1		/* PCIE controller 1 */

#define CONFIG_PCI_SCAN_SHOW

#define BOOT_ENV_BOARD                                                         \
	"console=ttyS0,115200\0"                                               \
	""

#endif /* __CONFIG_TQMLS1012AL_MBLS1012AL_H__ */
