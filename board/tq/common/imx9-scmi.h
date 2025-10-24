/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#ifndef __IMX9_SCMI_H
#define __IMX9_SCMI_H

#include <compiler.h>

#if IS_ENABLED(CONFIG_IMX95)
#include "../dts/upstream/src/arm64/freescale/imx95-clock.h"
#include "../dts/upstream/src/arm64/freescale/imx95-power.h"
#endif

int imx9_scmi_power_domain_enable(u32 domain, bool enable);

#endif
