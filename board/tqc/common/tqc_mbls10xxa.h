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

void tqc_mbls10xxa_retimer_init(void);

int tqc_mbls10xxa_clk_cfg_init(void);

enum {
	TQC_MBLS10xxA_SRDS_PROTO_UNUSED,
	TQC_MBLS10xxA_SRDS_PROTO_SGMII,
	TQC_MBLS10xxA_SRDS_PROTO_SGMII2G5,
	TQC_MBLS10xxA_SRDS_PROTO_QSGMII,
	TQC_MBLS10xxA_SRDS_PROTO_XFI,
	TQC_MBLS10xxA_SRDS_PROTO_PCIEx1,
	TQC_MBLS10xxA_SRDS_PROTO_PCIEx2,
	TQC_MBLS10xxA_SRDS_PROTO_PCIEx4,
	TQC_MBLS10xxA_SRDS_PROTO_SATA
};

enum {
	TQC_MBLS10xxA_SRDS_CLK_1_1,
	TQC_MBLS10xxA_SRDS_CLK_1_2,
	TQC_MBLS10xxA_SRDS_CLK_2_1,
	TQC_MBLS10xxA_SRDS_CLK_2_2
};

struct tqc_mbls10xxa_serdes {
	int port;
	int lane;
	int proto;
};

int tqc_mbls10xxa_serdes_init(struct tqc_mbls10xxa_serdes lanes[8]);
int tqc_mbls10xxa_serdes_clk_get(int clk);

int tqc_mbls10xxa_board_phy_config(struct phy_device *phydev);

#endif /* __TQC_MBLS10xxA_H__ */
