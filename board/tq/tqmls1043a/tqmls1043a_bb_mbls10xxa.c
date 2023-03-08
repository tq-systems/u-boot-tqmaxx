// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019,2023 TQ-Systems GmbH <u-boot@tq-group.com>, D-82229 Seefeld, Germany.
 * Author: Timo Herbrecher, Gregor Herburger
 *
 */

#include <common.h>
#include <asm/io.h>
#include <netdev.h>
#include <fdt_support.h>
#include <fm_eth.h>
#include <fsl_dtsec.h>
#include <fsl_mdio.h>
#include <malloc.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/immap_lsch2.h>
#include <linux/delay.h>
#include "../common/tq_bb.h"
#include "../common/tq_mbls10xxa.h"

#ifndef CONFIG_SPL_BUILD
#define TQMLS1043A_SRDS1_PROTO(cfg, lane)	(((cfg) >> 4 * (3 - (lane))) & 0xF)

static int _tqmls1043a_bb_set_lane(struct tq_mbls10xxa_serdes *serdes,
				   int port, int lane, enum tq_mbls10xxa_srds_proto proto)
{
	serdes->port = port;
	serdes->lane = lane;
	switch (proto) {
	case 0: /* Unused */
		serdes->proto = TQ_MBLS10XXA_SRDS_PROTO_UNUSED;
		break;
	case 1: /* XFI */
		serdes->proto = TQ_MBLS10XXA_SRDS_PROTO_XFI;
		break;
	case 2: /* 2.5G SGMII */
		serdes->proto = TQ_MBLS10XXA_SRDS_PROTO_SGMII2G5;
		break;
	case 3: /* SGMII */
		serdes->proto = TQ_MBLS10XXA_SRDS_PROTO_SGMII;
		break;
	case 4: /* QSGMII */
		serdes->proto = TQ_MBLS10XXA_SRDS_PROTO_QSGMII;
		break;
	case 5: /* PCIe x1 */
	case 9: /* PCIe x1 (alternate) */
		serdes->proto = TQ_MBLS10XXA_SRDS_PROTO_PCIEX1;
		break;
	case 6: /* PCIe x2 */
		serdes->proto = TQ_MBLS10XXA_SRDS_PROTO_PCIEX2;
		break;
	case 7: /* PCIe x4 */
		serdes->proto = TQ_MBLS10XXA_SRDS_PROTO_PCIEX4;
		break;
	case 8: /* SATA */
		serdes->proto = TQ_MBLS10XXA_SRDS_PROTO_SATA;
		break;
	default: /* Undefined */
		serdes->proto = -1;
		break;
	}
	return 0;
}

static void _tqmls1043a_bb_serdes_cfg(void)
{
	u32 srds_s1;
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	struct ccsr_serdes *serdes1_base = (void *)CONFIG_SYS_FSL_SERDES_ADDR;
	struct tq_mbls10xxa_serdes lanes[8];
	int moddef_det;
	int clk_freq;

	/* read SerDes configuration from RCW */
	srds_s1 = in_be32(&gur->rcwsr[4]) &
			FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_MASK;
	srds_s1 >>= FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_SHIFT;

	/* fill lane usage table */
	_tqmls1043a_bb_set_lane(&lanes[0], 1, 0, TQMLS1043A_SRDS1_PROTO(srds_s1, 0));
	_tqmls1043a_bb_set_lane(&lanes[1], 0, 0, 0);
	_tqmls1043a_bb_set_lane(&lanes[2], 1, 1, TQMLS1043A_SRDS1_PROTO(srds_s1, 1));
	_tqmls1043a_bb_set_lane(&lanes[3], 0, 0, 0);
	_tqmls1043a_bb_set_lane(&lanes[4], 0, 0, 0);
	_tqmls1043a_bb_set_lane(&lanes[5], 1, 2, TQMLS1043A_SRDS1_PROTO(srds_s1, 2));
	_tqmls1043a_bb_set_lane(&lanes[6], 0, 0, 0);
	_tqmls1043a_bb_set_lane(&lanes[7], 1, 3, TQMLS1043A_SRDS1_PROTO(srds_s1, 3));

	/* check and init lane usage */
	tq_mbls10xxa_serdes_init(lanes);

	/* check serdes clock muxing */
	clk_freq = tq_mbls10xxa_serdes_clk_get(TQ_MBLS10XXA_SRDS_CLK_1_2);
	if (clk_freq == 125000000 && TQMLS1043A_SRDS1_PROTO(srds_s1, 0) == 0x1) {
		printf("!!! ATTENTION: SerDes1 RefClk1 is not 156.25MHz,\n");
		printf("!!!  but this is needed for XFI operation!\n");
	} else if (clk_freq == 156250000 && TQMLS1043A_SRDS1_PROTO(srds_s1, 0) != 0x1) {
		printf("!!! ATTENTION: SerDes1 RefClk1 is 156.25MHz,\n");
		printf("!!!  but this is only valid for XFI operation!\n");
	}

	/* Configure TQ equalization for XFI */
	if (TQMLS1043A_SRDS1_PROTO(srds_s1, 0) == 0x1) {
		moddef_det = tq_mbls10xxa_i2c_gpio_get(XFI2_MODDEF_DET);
		if (!moddef_det)
			tq_mbls10xxa_i2c_gpio_set(XFI2_TX_DIS, 0);
		else
			tq_mbls10xxa_i2c_gpio_set(XFI2_TX_DIS, 1);

		tq_mbls10xxa_xfi_init(serdes1_base, 0);
	}

	tq_mbls10xxa_retimer_init();
}
#endif /* !CONFIG_SPL_BUILD */

int tq_bb_board_misc_init_r(void)
{
	int ret = 0;

#ifdef CONFIG_HAS_FSL_XHCI_USB
	struct ccsr_scfg *scfg = (struct ccsr_scfg *)CONFIG_SYS_FSL_SCFG_ADDR;
	u32 usb_pwrfault;

	/* USB3 pwr-ctrl is not used, configure mux to IIC4_SCL/IIC4_SDA */
	out_be32(&scfg->rcwpmuxcr0, 0x3300);
	out_be32(&scfg->usbdrvvbus_selcr, SCFG_USBDRVVBUS_SELCR_USB1);

	/* power-fault signal from baseboard is inverted, ignore */
	usb_pwrfault = 0;
	out_be32(&scfg->usbpwrfault_selcr, usb_pwrfault);
#endif

	/* configure baseboard clock buffer */
	tq_mbls10xxa_clk_cfg_init();

	/* initialize baseboard io's */
	ret = tq_mbls10xxa_i2c_gpios_init();

	/* check if SerDes mux settings match RCW and configure */
	if (!ret)
		_tqmls1043a_bb_serdes_cfg();

	/* enable USB-C power-controller */
	tq_mbls10xxa_i2c_gpio_set(USB_C_PWRON, 0);
	mdelay(10);
	tq_mbls10xxa_i2c_gpio_set(USB_C_PWRON, 1);

	/* reset usb-hub */
	tq_mbls10xxa_i2c_gpio_set(USB_H_GRST, 1);
	mdelay(10);
	tq_mbls10xxa_i2c_gpio_set(USB_H_GRST, 0);
	return 0;
}

int tq_bb_board_fix_fdt(void *fdt)
{
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 srds_s1;

	/* read SerDes configuration from RCW */
	srds_s1 = in_be32(&gur->rcwsr[4]) &
			FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_MASK;
	srds_s1 >>= FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_SHIFT;

	if (serdes_get_prtcl(0, srds_s1, 0) == SGMII_FM1_DTSEC9)
		tq_mbls10xxa_fixup_phy_to_enet(fdt, "ethernet6", "qsgmii_s2_p2", "sgmii");
	else if (serdes_get_prtcl(0, srds_s1, 0) == XFI_FM1_MAC9)
		tq_mbls10xxa_fixup_enet_fixed_link(fdt, "ethernet6", 0, "xgmii");

	if (serdes_get_prtcl(0, srds_s1, 1) == SGMII_FM1_DTSEC2) {
		tq_mbls10xxa_fixup_phy_to_enet(fdt, "ethernet1", "qsgmii_s2_p1", "sgmii");
	} else if (serdes_get_prtcl(0, srds_s1, 1) == QSGMII_FM1_A) {
		tq_mbls10xxa_fixup_phy_to_enet(fdt, "ethernet0", "qsgmii_s2_p1", "qsgmii");
		tq_mbls10xxa_fixup_phy_to_enet(fdt, "ethernet1", "qsgmii_s2_p2", "qsgmii");
		tq_mbls10xxa_fixup_phy_to_enet(fdt, "ethernet4", "qsgmii_s2_p3", "qsgmii");
		tq_mbls10xxa_fixup_phy_to_enet(fdt, "ethernet5", "qsgmii_s2_p4", "qsgmii");
	}

	if (serdes_get_prtcl(0, srds_s1, 3) == SGMII_FM1_DTSEC5)
		tq_mbls10xxa_fixup_phy_to_enet(fdt, "ethernet4", "qsgmii_s1_p2", "sgmii");

	return 0;
}

int tq_bb_ft_board_setup(void *blob, struct bd_info *bd)
{
	return tq_bb_board_fix_fdt(blob);
}
