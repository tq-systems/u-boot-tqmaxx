// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 TQ-Systems GmbH
 */
#ifndef __TQC_MBLS10xxA_H__
#define __TQC_MBLS10xxA_H__

#include <pca953x.h>

int tqc_mbls10xxa_i2c_gpios_init(void);

int tqc_mbls10xxa_i2c_gpio_get(const char *name);
int tqc_mbls10xxa_i2c_gpio_set(const char *name, int val);

int tqc_mbls10xxa_clk_cfg_init(void);

#endif /* __TQC_MBLS10xxA_H__ */
