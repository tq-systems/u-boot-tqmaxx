// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2023 TQ-Systems GmbH
 * Copyright (c) 2023 TQ-Systems GmbH <license@tq-group.com>, D-82229 Seefeld, Germany.
 * Author: Timo Herbrecher, Gregor Herburger
 *
 */
#include <common.h>
#include <asm/io.h>
#include <netdev.h>
#include <fdt_support.h>
#include <fm_eth.h>
#include <fsl_dtsec.h>
#include <fsl-mc/fsl_mc.h>
#include <fsl_mdio.h>
#include <malloc.h>
#include <linux/delay.h>
#include <asm/arch/fsl_serdes.h>
#include "../common/tq_bb.h"
#include "../common/tq_mbls10xxa.h"

#define TQMLS1088A_SRDS1_PROTO(cfg, lane)	(((cfg) >> 4 * (lane)) & 0xF)
#define TQMLS1088A_SRDS2_PROTO(cfg, lane)	(((cfg) >> 4 * (lane)) & 0xF)

static int tqmls1088a_bb_set_lane(struct tq_mbls10xxa_serdes *serdes, int port,
				  int lane, enum tq_mbls10xxa_srds_proto proto)
{
	serdes->port = port;
	serdes->lane = lane;
	switch (proto) {
	case 0x0: /* Unused */
		serdes->proto = TQ_MBLS10XXA_SRDS_PROTO_UNUSED;
		break;
	case 0x1: /* XFI */
		serdes->proto = TQ_MBLS10XXA_SRDS_PROTO_XFI;
		break;
	case 0x2: /* 2.5G SGMII */
		serdes->proto = TQ_MBLS10XXA_SRDS_PROTO_SGMII2G5;
		break;
	case 0x3: /* SGMII */
		serdes->proto = TQ_MBLS10XXA_SRDS_PROTO_SGMII;
		break;
	case 0x4: /* QSGMII */
		serdes->proto = TQ_MBLS10XXA_SRDS_PROTO_QSGMII;
		break;
	case 0x5: /* PCIe x1 */
	case 0x6: /* PCIe x1 (alternate) */
		serdes->proto = TQ_MBLS10XXA_SRDS_PROTO_PCIEX1;
		break;
	case 0x7: /* PCIe x2 */
		serdes->proto = TQ_MBLS10XXA_SRDS_PROTO_PCIEX2;
		break;
	case 0x8: /* PCIe x4 */
		serdes->proto = TQ_MBLS10XXA_SRDS_PROTO_PCIEX4;
		break;
	case 0x9: /* SATA */
		serdes->proto = TQ_MBLS10XXA_SRDS_PROTO_SATA;
		break;
	default: /* Undefined */
		serdes->proto = -1;
		break;
	}

	return 0;
}

static int tqmls1088a_bb_check_serdes_mux(void)
{
	u32 srds_s1, srds_s2;
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	struct ccsr_serdes *serdes1_base = (void *)CONFIG_SYS_FSL_LSCH3_SERDES_ADDR;
	struct tq_mbls10xxa_serdes lanes[8];
	int moddef_det;
	int clk_freq;

	/* read SerDes configuration from RCW */
	srds_s1 = in_le32(&gur->rcwsr[FSL_CHASSIS3_SRDS1_REGSR - 1]) &
			  FSL_CHASSIS3_SRDS1_PRTCL_MASK;
	srds_s1 >>= FSL_CHASSIS3_SRDS1_PRTCL_SHIFT;
	srds_s2 = in_le32(&gur->rcwsr[FSL_CHASSIS3_SRDS2_REGSR - 1]) &
			  FSL_CHASSIS3_SRDS2_PRTCL_MASK;
	srds_s2 >>= FSL_CHASSIS3_SRDS2_PRTCL_SHIFT;

	/* fill lane usage table */
	tqmls1088a_bb_set_lane(&lanes[0], 1, 0, TQMLS1088A_SRDS1_PROTO(srds_s1, 0));
	tqmls1088a_bb_set_lane(&lanes[1], 1, 1, TQMLS1088A_SRDS1_PROTO(srds_s1, 1));
	tqmls1088a_bb_set_lane(&lanes[2], 1, 2, TQMLS1088A_SRDS1_PROTO(srds_s1, 2));
	tqmls1088a_bb_set_lane(&lanes[3], 1, 3, TQMLS1088A_SRDS1_PROTO(srds_s1, 3));
	tqmls1088a_bb_set_lane(&lanes[4], 2, 0, TQMLS1088A_SRDS2_PROTO(srds_s2, 0));
	tqmls1088a_bb_set_lane(&lanes[5], 2, 1, TQMLS1088A_SRDS2_PROTO(srds_s2, 1));
	tqmls1088a_bb_set_lane(&lanes[6], 2, 2, TQMLS1088A_SRDS2_PROTO(srds_s2, 2));
	tqmls1088a_bb_set_lane(&lanes[7], 2, 3, TQMLS1088A_SRDS2_PROTO(srds_s2, 3));

	/* check and init lane usage */
	tq_mbls10xxa_serdes_init(lanes);

	/* check serdes clock muxing */
	clk_freq = tq_mbls10xxa_serdes_clk_get(TQ_MBLS10XXA_SRDS_CLK_1_2);
	if (clk_freq == 125000000 &&
	    (TQMLS1088A_SRDS1_PROTO(srds_s1, 0) == 0x1 ||
	    TQMLS1088A_SRDS1_PROTO(srds_s1, 1) == 0x1)) {
		printf("!!! ATTENTON: SerDes1 RefClk2 is not 156.25MHz,\n");
		printf("!!!  but this is needed for XFI operation!\n");
	} else if ((clk_freq == 156250000) &&
		   (TQMLS1088A_SRDS1_PROTO(srds_s1, 0) != 0x1 &&
		   TQMLS1088A_SRDS1_PROTO(srds_s1, 1) != 0x1)) {
		printf("!!! ATTENTON: SerDes1 RefClk2 is 156.25MHz,\n");
		printf("!!!  but this is only valid for XFI operation!\n");
	}

	if (TQMLS1088A_SRDS1_PROTO(srds_s1, 0) == 0x1) {
		moddef_det = tq_mbls10xxa_i2c_gpio_get(XFI2_MODDEF_DET);
		if (!moddef_det)
			tq_mbls10xxa_i2c_gpio_set(XFI2_TX_DIS, 0);
		else
			tq_mbls10xxa_i2c_gpio_set(XFI2_TX_DIS, 1);
		tq_mbls10xxa_xfi_init(serdes1_base, 3);
	}

	if (TQMLS1088A_SRDS1_PROTO(srds_s1, 1) == 0x1) {
		moddef_det = tq_mbls10xxa_i2c_gpio_get(XFI1_MODDEF_DET);
		if (!moddef_det)
			tq_mbls10xxa_i2c_gpio_set(XFI1_TX_DIS, 0);
		else
			tq_mbls10xxa_i2c_gpio_set(XFI1_TX_DIS, 1);
		tq_mbls10xxa_xfi_init(serdes1_base, 2);
	}

	tq_mbls10xxa_retimer_init();

	return 0;
}

int tq_bb_board_misc_init_r(void)
{
	int ret = 0;

	/* configure baseboard clock buffer */
	tq_mbls10xxa_clk_cfg_init();

	/* initialize baseboard io's */
	ret = tq_mbls10xxa_i2c_gpios_init();

	/* check if SerDes mux settings match RCW and configure */
	if (!ret)
		tqmls1088a_bb_check_serdes_mux();

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
	int srds_nr;
	int ret;

	/* read SerDes configuration from RCW */
	srds_s1 = in_le32(&gur->rcwsr[FSL_CHASSIS3_SRDS1_REGSR - 1]) &
			  FSL_CHASSIS3_SRDS1_PRTCL_MASK;
	srds_s1 >>= FSL_CHASSIS3_SRDS1_PRTCL_SHIFT;
	srds_nr = serdes_get_number(0, srds_s1);

	ret = fdt_increase_size(fdt, 1024);
	if (ret)
		return ret;

	if (serdes_get_prtcl(0, srds_nr, 0) == SGMII3) {
		ret = tq_mbls10xxa_fixup_phy_to_enet(fdt, "dpmac3", "qsgmii_s1_p1", "sgmii");
		tq_mbls10xxa_fixup_phy_to_enet(fdt, "dpmac4", "rgmii_s1", "rgmii-id");
		tq_mbls10xxa_fixup_phy_to_enet(fdt, "dpmac5", "rgmii_s2", "rgmii-id");
	} else if (serdes_get_prtcl(0, srds_nr, 0) == QSGMII_A) {
		tq_mbls10xxa_fixup_phy_to_enet(fdt, "dpmac3", "qsgmii_s1_p1", "qsgmii");
		tq_mbls10xxa_fixup_phy_to_enet(fdt, "dpmac4", "qsgmii_s1_p2", "qsgmii");
		tq_mbls10xxa_fixup_phy_to_enet(fdt, "dpmac5", "qsgmii_s1_p3", "qsgmii");
		tq_mbls10xxa_fixup_phy_to_enet(fdt, "dpmac6", "qsgmii_s1_p4", "qsgmii");
	}

	if (serdes_get_prtcl(0, srds_nr, 1) == SGMII7) {
		ret = tq_mbls10xxa_fixup_phy_to_enet(fdt, "dpmac7", "qsgmii_s2_p1", "sgmii");
	} else if (serdes_get_prtcl(0, srds_nr, 1) == QSGMII_B) {
		tq_mbls10xxa_fixup_phy_to_enet(fdt, "dpmac7", "qsgmii_s2_p1", "qsgmii");
		tq_mbls10xxa_fixup_phy_to_enet(fdt, "dpmac8", "qsgmii_s2_p2", "qsgmii");
		tq_mbls10xxa_fixup_phy_to_enet(fdt, "dpmac9", "qsgmii_s2_p3", "qsgmii");
		tq_mbls10xxa_fixup_phy_to_enet(fdt, "dpmac10", "qsgmii_s2_p4", "qsgmii");
	}

	if (serdes_get_prtcl(0, srds_nr, 2) == XFI1) {
		ret = tq_mbls10xxa_fixup_enet_sfp(fdt, "dpmac1", "/sfp1", "10gbase-r");
		if (ret)
			tq_mbls10xxa_fixup_enet_fixed_link(fdt, "dpmac1", 0, "xgmii");
	}

	if (serdes_get_prtcl(0, srds_nr, 3) == XFI2) {
		ret = tq_mbls10xxa_fixup_enet_sfp(fdt, "dpmac2", "/sfp2", "10gbase-r");
		if (ret)
			tq_mbls10xxa_fixup_enet_fixed_link(fdt, "dpmac2", 1, "xgmii");
	} else if (serdes_get_prtcl(0, srds_nr, 3) == SGMII2) {
		tq_mbls10xxa_fixup_phy_to_enet(fdt, "dpmac2", "qsgmii_s2_p2", "sgmii");
	}

	return 0;
}

void fdt_fixup_board_enet(void *fdt)
{
	int offset;

	offset = fdt_path_offset(fdt, "/fsl-mc");

	if (offset < 0)
		offset = fdt_path_offset(fdt, "/soc/fsl-mc");

	if (offset < 0) {
		printf("%s: ERROR: fsl-mc node not found in device tree (error %d)\n",
		       __func__, offset);
		return;
	}

	if (get_mc_boot_status() == 0 &&
	    (is_lazy_dpl_addr_valid() || get_dpl_apply_status() == 0))
		fdt_status_okay(fdt, offset);
	else
		fdt_status_fail(fdt, offset);
}

#if defined(CONFIG_RESET_PHY_R)
void reset_phy(void)
{
	mc_env_boot();
}
#endif /* CONFIG_RESET_PHY_R */

int tq_bb_ft_board_setup(void *blob, struct bd_info *bd)
{
	return tq_bb_board_fix_fdt(blob);
}
