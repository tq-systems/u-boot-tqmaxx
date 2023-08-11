/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Configuration header file for TQ-Systems TQMa64xxL
 *
 * Copyright (c) 2020-2023 TQ-Systems GmbH <u-boot@tq-group.com>, D-82229 Seefeld, Germany.
 */

#ifndef __CONFIG_TQMA64XXL_H
#define __CONFIG_TQMA64XXL_H

#define CFG_SYS_SDRAM_BASE	0x80000000
#define CFG_SYS_SDRAM_BASE1	0x880000000

# if IS_ENABLED(CONFIG_DISTRO_DEFAULTS)

#include <config_distro_bootcmd.h>
#include <environment/distro/sf.h>

#else /* CONFIG_DISTRO_DEFAULTS */

#define BOOTENV ""
#define BOOTENV_SF ""

#endif /* CONFIG_DISTRO_DEFAULTS */

#endif /* __CONFIG_TQMA64XXL_H */
