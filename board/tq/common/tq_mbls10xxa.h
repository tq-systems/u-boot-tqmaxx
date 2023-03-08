/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2018-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Timo Herbrecher, Gregor Herburger
 */

#ifndef __TQ_MBLS10XXA_H__
#define __TQ_MBLS10XXA_H__

#include <asm-generic/gpio.h>
#include "tq_board_gpio.h"

#if defined(CONFIG_FSL_LSCH3)
#define serdes_in32(a)       in_le32(a)
#define serdes_out32(a, v)   out_le32(a, v)
#elif defined(CONFIG_FSL_LSCH2)
#define serdes_in32(a)       in_be32(a)
#define serdes_out32(a, v)   out_be32(a, v)
#endif

void tq_mbls10xxa_retimer_init(void);
void tq_mbls10xxa_xfi_init(struct ccsr_serdes *serdes, int lane);

int tq_mbls10xxa_clk_cfg_init(void);

enum tq_mbls10xxa_srds_proto {
	TQ_MBLS10XXA_SRDS_PROTO_UNUSED,
	TQ_MBLS10XXA_SRDS_PROTO_SGMII,
	TQ_MBLS10XXA_SRDS_PROTO_SGMII2G5,
	TQ_MBLS10XXA_SRDS_PROTO_QSGMII,
	TQ_MBLS10XXA_SRDS_PROTO_XFI,
	TQ_MBLS10XXA_SRDS_PROTO_PCIEX1,
	TQ_MBLS10XXA_SRDS_PROTO_PCIEX2,
	TQ_MBLS10XXA_SRDS_PROTO_PCIEX4,
	TQ_MBLS10XXA_SRDS_PROTO_SATA,
	TQ_MBLS10XXA_SRDS_PROTO_PCIEX1_ALT,
	TQ_MBLS10XXA_SRDS_PROTO_SGMII_ALT,
};

enum tq_mbls10xxa_srds_clk {
	TQ_MBLS10XXA_SRDS_CLK_1_1,
	TQ_MBLS10XXA_SRDS_CLK_1_2,
	TQ_MBLS10XXA_SRDS_CLK_2_1,
	TQ_MBLS10XXA_SRDS_CLK_2_2
};

struct tq_mbls10xxa_serdes {
	int port;
	int lane;
	enum tq_mbls10xxa_srds_proto proto;
};

enum tq_mbls10xxa_gpios {
	SD1_3_LANE_A_MUX,
	SD1_2_LANE_B_MUX,
	SD1_0_LANE_D_MUX,
	SD2_1_LANE_B_MUX,
	SD2_3_LANE_D_MUX1,
	SD2_3_LANE_D_MUX2,
	SD_MUX_SHDN,
	SD1_REF_CLK2_SEL,
	MPCIE1_DISABLE,
	MPCIE1_WAKE,
	MPCIE2_DISABLE,
	MPCIE2_WAKE,
	PRSNT,
	PCIE_PWR_EN,
	DCDC_PGOOD_1V8,
	XFI1_TX_FAULT,
	XFI1_TX_DIS,
	XFI1_MODDEF_DET,
	XFI1_RX_LOSS,
	RETIMER1_LOSS,
	XFI1_ENSMB,
	QSGMII1_CLK_SEL0,
	QSGMII_PHY1_CONFIG3,
	XFI2_TX_FAULT,
	XFI2_TX_DIS,
	XFI2_MODDEF_DET,
	XFI2_RX_LOSS,
	RETIMER2_LOSS,
	XFI2_ENSMB,
	QSGMII2_CLK_SEL0,
	QSGMII_PHY2_CONFIG3,
	EC1_PHY_PWDN,
	EC2_PHY_PWDN,
	USB_C_PWRON,
	USB_EN_OC_3V3,
	USB_H_GRST,
	GPIO_BUTTON0,
	GPIO_BUTTON1,
	SDA_PWR_EN,
	QSGMII_PHY1_INT,
	QSGMII_PHY2_INT,
	SPI_CLKO_SOF,
	SPI_INT,
	CAN_SEL,
	LED,
	PCIE_RST_3V3,
	PCIE_WAKE_3V3,
};

int tq_mbls10xxa_i2c_gpios_init(void);
int tq_mbls10xxa_i2c_gpio_get(enum tq_mbls10xxa_gpios gpio);
int tq_mbls10xxa_i2c_gpio_set(enum tq_mbls10xxa_gpios gpio, int val);

int tq_mbls10xxa_serdes_init(struct tq_mbls10xxa_serdes lanes[8]);
int tq_mbls10xxa_serdes_clk_get(enum tq_mbls10xxa_srds_clk clk);

int tq_mbls10xxa_board_phy_config(struct phy_device *phydev);
int tq_mbls10xxa_fixup_phy_to_enet(void *fdt, char *enet_alias, char *phy_alias, char *connection);
int tq_mbls10xxa_fixup_enet_fixed_link(void *fdt, char *enet_alias, int id, char *connection);

#endif /* __TQ_MBLS10XXA_H__ */
