// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 TQ-Systems GmbH
 */

#include <common.h>
#include <asm/io.h>
#include <netdev.h>
#include <fm_eth.h>
#include <fsl_dtsec.h>
#include <fsl_mdio.h>
#include <malloc.h>
#include <fsl-mc/fsl_mc.h>
#include <fsl-mc/ldpaa_wriop.h>
#include "../common/tqc_bb.h"
#include "../common/tqc_mbls10xxa.h"


#ifndef CONFIG_SPL_BUILD
#define TQMLS1088A_SRDS1_PROTO(cfg, lane)	((cfg >> 4*lane) & 0xF)
#define TQMLS1088A_SRDS2_PROTO(cfg, lane)	((cfg >> 4*lane) & 0xF)

static int _tqmls1088a_bb_set_lane(struct tqc_mbls10xxa_serdes *serdes, int port, int lane, int proto)
{
	serdes->port = port;
	serdes->lane = lane;
	switch(proto) {
		case 0x0: /* Unused */
			serdes->proto = TQC_MBLS10xxA_SRDS_PROTO_UNUSED;
			break;
		case 0x1: /* XFI */
			serdes->proto = TQC_MBLS10xxA_SRDS_PROTO_XFI;
			break;
		case 0x2: /* 2.5G SGMII */
			serdes->proto = TQC_MBLS10xxA_SRDS_PROTO_SGMII2G5;
			break;
		case 0x3: /* SGMII */
			serdes->proto = TQC_MBLS10xxA_SRDS_PROTO_SGMII;
			break;
		case 0x4: /* QSGMII */
			serdes->proto = TQC_MBLS10xxA_SRDS_PROTO_QSGMII;
			break;
		case 0x5: /* PCIe x1 */
		case 0x6: /* PCIe x1 (alternate) */
			serdes->proto = TQC_MBLS10xxA_SRDS_PROTO_PCIEx1;
			break;
		case 0x7: /* PCIe x2 */
			serdes->proto = TQC_MBLS10xxA_SRDS_PROTO_PCIEx2;
			break;
		case 0x8: /* PCIe x4 */
			serdes->proto = TQC_MBLS10xxA_SRDS_PROTO_PCIEx4;
			break;
		case 0x9: /* SATA */
			serdes->proto = TQC_MBLS10xxA_SRDS_PROTO_SATA;
			break;
		default: /* Undefined */
			serdes->proto = -1;
			break;
	}

	return 0;
}

static int _tqmls1088a_bb_check_serdes_mux(void)
{
	u32 srds_s1, srds_s2;
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	struct ccsr_serdes *serdes1_base = (void *)CONFIG_SYS_FSL_LSCH3_SERDES_ADDR;
	struct tqc_mbls10xxa_serdes lanes[8];
	int clk_freq;

	/* read SerDes configuration from RCW */
	srds_s1 = in_le32(&gur->rcwsr[FSL_CHASSIS3_SRDS1_REGSR - 1]) &
	                  FSL_CHASSIS3_SRDS1_PRTCL_MASK;
	srds_s1 >>= FSL_CHASSIS3_SRDS1_PRTCL_SHIFT;
	srds_s2 = in_le32(&gur->rcwsr[FSL_CHASSIS3_SRDS2_REGSR - 1]) &
	                  FSL_CHASSIS3_SRDS2_PRTCL_MASK;
	srds_s2 >>= FSL_CHASSIS3_SRDS2_PRTCL_SHIFT;

	/* fill lane usage table */
	_tqmls1088a_bb_set_lane(&(lanes[0]), 1, 0, TQMLS1088A_SRDS1_PROTO(srds_s1, 0));
	_tqmls1088a_bb_set_lane(&(lanes[1]), 1, 1, TQMLS1088A_SRDS1_PROTO(srds_s1, 1));
	_tqmls1088a_bb_set_lane(&(lanes[2]), 1, 2, TQMLS1088A_SRDS1_PROTO(srds_s1, 2));
	_tqmls1088a_bb_set_lane(&(lanes[3]), 1, 3, TQMLS1088A_SRDS1_PROTO(srds_s1, 3));
	_tqmls1088a_bb_set_lane(&(lanes[4]), 2, 0, TQMLS1088A_SRDS2_PROTO(srds_s2, 0));
	_tqmls1088a_bb_set_lane(&(lanes[5]), 2, 1, TQMLS1088A_SRDS2_PROTO(srds_s2, 1));
	_tqmls1088a_bb_set_lane(&(lanes[6]), 2, 2, TQMLS1088A_SRDS2_PROTO(srds_s2, 2));
	_tqmls1088a_bb_set_lane(&(lanes[7]), 2, 3, TQMLS1088A_SRDS2_PROTO(srds_s2, 3));

	/* check and init lane usage */
	tqc_mbls10xxa_serdes_init(lanes);

	/* check serdes clock muxing */
	clk_freq = tqc_mbls10xxa_serdes_clk_get(TQC_MBLS10xxA_SRDS_CLK_1_2);
	if((clk_freq == 125000000) &&
	   ((TQMLS1088A_SRDS1_PROTO(srds_s1, 0) == 0x1) ||
	    (TQMLS1088A_SRDS1_PROTO(srds_s1, 1) == 0x1))) {
	   printf("!!! ATTENTON: SerDes1 RefClk2 is not 156.25MHz,\n");
	   printf("!!!  but this is needed for XFI operation!\n");
	} else if((clk_freq == 156250000) && 
	   ((TQMLS1088A_SRDS1_PROTO(srds_s1, 0) != 0x1) &&
	    (TQMLS1088A_SRDS1_PROTO(srds_s1, 1) != 0x1))) {
	   printf("!!! ATTENTON: SerDes1 RefClk2 is 156.25MHz,\n");
	   printf("!!!  but this is only valid for XFI operation!\n");
	}
	if (TQMLS1088A_SRDS1_PROTO(srds_s1, 0) == 0x1)
		tqc_mbls10xxa_xfi_init(serdes1_base, 3);
	if (TQMLS1088A_SRDS1_PROTO(srds_s1, 1) == 0x1)
		tqc_mbls10xxa_xfi_init(serdes1_base, 2);

	tqc_mbls10xxa_retimer_init();

	return 0;
}
#endif /* !CONFIG_SPL_BUILD */

int tqc_bb_board_early_init_f(void)
{
	struct ccsr_gpio *ccsr;
	uint32_t reg;
	int ret = 0;

	/* configure sd card detect and write protect GPIOs as input */
	/* CD = GPIO3_12, WP = GPIO3_13 */
	ccsr = (void *)(GPIO3_BASE_ADDR);
	reg = in_le32(&ccsr->gpdir);
	reg &= ~(0x000C0000);
	out_le32(&ccsr->gpdir, reg);
	reg = in_le32(&ccsr->gpibe);
	reg |= 0x000C0000;
	out_le32(&ccsr->gpibe, reg);


	/* TODO: check if hw USB power control could be used */
	/* configure USB DRVVBUS GPIOs as output, default low */
	/* USB_DRVVBUS = GPIO4_2, USB2_DRVVBUS = GPIO4_28 */
	ccsr = (void *)(GPIO4_BASE_ADDR);
	reg = in_le32(&ccsr->gpdir);
	reg |= 0x20000008;
	out_le32(&ccsr->gpdir, reg);
	reg = in_le32(&ccsr->gpdat);
	reg &= ~(0x20000008);
	out_le32(&ccsr->gpdat, reg);
	reg = in_le32(&ccsr->gpibe);
	reg |= 0x20000008;
	out_le32(&ccsr->gpibe, reg);

	/* nothing to do */
	return ret;
}

#ifndef CONFIG_SPL_BUILD
int tqc_bb_board_init(void)
{
	int ret = 0;

	/* nothing to do */
	return ret;
}

int tqc_bb_misc_init_r(void)
{
	int ret = 0;
	struct ccsr_gpio *ccsr = (void *)(GPIO4_BASE_ADDR);
	uint32_t reg;

	/* TODO: check if hw USB power control could be used */
	/* enable power of USB ports by setting GPIOs high */
	/* USB_DRVVBUS = GPIO4_2, USB2_DRVVBUS = GPIO4_28 */
	reg = in_le32(&ccsr->gpdat);
	reg |= 0x20000008;
	out_le32(&ccsr->gpdat, reg);

	/* configure baseboard clock buffer */
	tqc_mbls10xxa_clk_cfg_init();

	/* initialize baseboard io's */
	ret = tqc_mbls10xxa_i2c_gpios_init();

	/* check if SerDes mux settings match RCW */
	if(!ret)
		ret = _tqmls1088a_bb_check_serdes_mux();

	/* reset usb-hub */
	tqc_mbls10xxa_i2c_gpio_set("usb_h_grst#", 0);
	mdelay(10);
	tqc_mbls10xxa_i2c_gpio_set("usb_h_grst#", 1);

	return 0;
}

int tqc_bb_checkboard(void)
{
	/* nothing to do */
	return 0;
}

const char *tqc_bb_get_boardname(void)
{
	return "MBLS10xxA";
}

#ifdef CONFIG_NET
int tqc_bb_board_eth_init(bd_t *bis)
{
#ifdef CONFIG_FSL_MC_ENET
	struct memac_mdio_info mdio_info;
	struct mii_dev *dev_mdio;
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	struct memac_mdio_controller *reg;
	u32 srds_s1;
	int moddef_det;

	/* read SerDes configuration from RCW */
	srds_s1 = in_le32(&gur->rcwsr[FSL_CHASSIS3_SRDS1_REGSR - 1]) &
	                  FSL_CHASSIS3_SRDS1_PRTCL_MASK;
	srds_s1 >>= FSL_CHASSIS3_SRDS1_PRTCL_SHIFT;

    /* Register the EMI 1 */
    reg = (struct memac_mdio_controller *)CONFIG_SYS_FSL_WRIOP1_MDIO1;
    mdio_info.regs = reg;
    mdio_info.name = DEFAULT_WRIOP_MDIO1_NAME;
    fm_memac_mdio_init(bis, &mdio_info);

    /* Register the EMI 2 */
    reg = (struct memac_mdio_controller *)CONFIG_SYS_FSL_WRIOP1_MDIO2;
    mdio_info.regs = reg;
    mdio_info.name = DEFAULT_WRIOP_MDIO2_NAME;
    fm_memac_mdio_init(bis, &mdio_info);

	/* get MDIO bus devices */
	dev_mdio = miiphy_get_dev_by_name(DEFAULT_WRIOP_MDIO1_NAME);

	/* set xGMII PHY addresses based on RCW */
	if(TQMLS1088A_SRDS1_PROTO(srds_s1, 3) != 0x4) {
		/* set the two on-board RGMII PHY address */
		wriop_set_phy_address(WRIOP1_DPMAC4, RGMII_PHY1_ADDR);
		wriop_set_mdio(WRIOP1_DPMAC4, dev_mdio);
		wriop_set_phy_address(WRIOP1_DPMAC5, RGMII_PHY2_ADDR);
		wriop_set_mdio(WRIOP1_DPMAC5, dev_mdio);
	}
	if(TQMLS1088A_SRDS1_PROTO(srds_s1, 3) == 0x3) {
		/* SD1 - LANE A in SGMII.3 mode */
		wriop_set_phy_address(WRIOP1_DPMAC3, QSGMII_PHY1_ADDR_BASE+0);
		wriop_set_mdio(WRIOP1_DPMAC3, dev_mdio);
	} else if(TQMLS1088A_SRDS1_PROTO(srds_s1, 3) == 0x4) {
		/* SD1 - LANE A in QSGMII.3,4,5,6 mode */
		wriop_set_phy_address(WRIOP1_DPMAC3, QSGMII_PHY1_ADDR_BASE+0);
		wriop_set_mdio(WRIOP1_DPMAC3, dev_mdio);
		wriop_set_phy_address(WRIOP1_DPMAC4, QSGMII_PHY1_ADDR_BASE+1);
		wriop_set_mdio(WRIOP1_DPMAC4, dev_mdio);
		wriop_set_phy_address(WRIOP1_DPMAC5, QSGMII_PHY1_ADDR_BASE+2);
		wriop_set_mdio(WRIOP1_DPMAC5, dev_mdio);
		wriop_set_phy_address(WRIOP1_DPMAC6, QSGMII_PHY1_ADDR_BASE+3);
		wriop_set_mdio(WRIOP1_DPMAC6, dev_mdio);
	}
	if(TQMLS1088A_SRDS1_PROTO(srds_s1, 2) == 0x3) {
		/* SD1 - LANE B in SGMII.7 mode */
		wriop_set_phy_address(WRIOP1_DPMAC7, QSGMII_PHY2_ADDR_BASE+0);
		wriop_set_mdio(WRIOP1_DPMAC7, dev_mdio);
	} else if(TQMLS1088A_SRDS1_PROTO(srds_s1, 2) == 0x4) {
		/* SD1 - LANE B in QSGMII.7,8,9,10 mode */
		wriop_set_phy_address(WRIOP1_DPMAC7, QSGMII_PHY2_ADDR_BASE+0);
		wriop_set_mdio(WRIOP1_DPMAC7, dev_mdio);
		wriop_set_phy_address(WRIOP1_DPMAC8, QSGMII_PHY2_ADDR_BASE+1);
		wriop_set_mdio(WRIOP1_DPMAC8, dev_mdio);
		wriop_set_phy_address(WRIOP1_DPMAC9, QSGMII_PHY2_ADDR_BASE+2);
		wriop_set_mdio(WRIOP1_DPMAC9, dev_mdio);
		wriop_set_phy_address(WRIOP1_DPMAC10, QSGMII_PHY2_ADDR_BASE+3);
		wriop_set_mdio(WRIOP1_DPMAC10, dev_mdio);
	}
	if(TQMLS1088A_SRDS1_PROTO(srds_s1, 1) == 0x1) {
		/* SD1 - LANE C in XFI.1 mode */

		/* Add dummy PHY address when XFI used */
		wriop_set_phy_address(WRIOP1_DPMAC1, 0x0a);

		/* enable XFI transmitter when XFI selected and SFP is available
		 * (MOD-DEF[0] (SFP pin 6) is grounded)
		 */
		moddef_det = tqc_mbls10xxa_i2c_gpio_get("xfi1_moddef_det");
		if(!moddef_det)
			tqc_mbls10xxa_i2c_gpio_set("xfi1_tx_dis", 0);
		else
			tqc_mbls10xxa_i2c_gpio_set("xfi1_tx_dis", 1);
	}
	if(TQMLS1088A_SRDS1_PROTO(srds_s1, 0) == 0x3) {
		/* SD1 - LANE D in SGMII.2 mode */
		wriop_set_phy_address(WRIOP1_DPMAC2, QSGMII_PHY2_ADDR_BASE+1);
		wriop_set_mdio(WRIOP1_DPMAC2, dev_mdio);
	} else if(TQMLS1088A_SRDS1_PROTO(srds_s1, 0) == 0x1) {
		/* SD1 - LANE D in XFI.2 mode */

		/* Add dummy PHY address when XFI used */
		wriop_set_phy_address(WRIOP1_DPMAC2, 0x0b);

		/* enable XFI transmitter when XFI selected and SFP is available
		 * (MOD-DEF[0] (SFP pin 6) is grounded)
		 */
		moddef_det = tqc_mbls10xxa_i2c_gpio_get("xfi2_moddef_det");
		if(!moddef_det)
			tqc_mbls10xxa_i2c_gpio_set("xfi2_tx_dis", 0);
		else
			tqc_mbls10xxa_i2c_gpio_set("xfi2_tx_dis", 1);
	}
#endif

	return 0;
}
#endif
#endif

int tqc_bb_board_mmc_getcd(struct mmc *mmc)
{
	struct ccsr_gpio *ccsr = (void *)(GPIO3_BASE_ADDR);

	return !(in_le32(&ccsr->gpdat) & (1 << (31-12)));
}

int tqc_bb_board_mmc_getwp(struct mmc *mmc)
{
	struct ccsr_gpio *ccsr = (void *)(GPIO3_BASE_ADDR);

	return !!(in_le32(&ccsr->gpdat) & (1 << (31-13)));
}

#ifndef CONFIG_SPL_BUILD
int board_phy_config(struct phy_device *phydev)
{
	/* call baseboard common function */
	return tqc_mbls10xxa_board_phy_config(phydev);
}

#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
static int fdt_set_phy_handle(void *fdt, int mac, const char *alias)
{
	int offset;
	unsigned int ph;
	const char *path;
	char name[8];
	int lenp;
	int found = 0;

	/* Get a path to the node that 'alias' points to */
	path = fdt_get_alias(fdt, alias);
	if (!path)
		return -FDT_ERR_BADPATH;

	/* Get the offset of that node */
	offset = fdt_path_offset(fdt, path);
	if (offset < 0)
		return offset;

	ph = fdt_create_phandle(fdt, offset);
	if (!ph)
		return -FDT_ERR_BADPHANDLE;

	ph = cpu_to_fdt32(ph);

	/* Get path to dpmacs node */
	offset = fdt_path_offset(fdt, "/soc/fsl-mc/dpmacs");
	if (offset < 0)
		return offset;
	
	/* Get relevant dpmac node */
	sprintf(name, "dpmac@%x", mac);
	fdt_for_each_subnode(offset, fdt, offset) {
		path = fdt_get_name(fdt, offset, &lenp);
		if(!strncmp(name, path, strlen(name))) {
			found++;
			break;
		}
	}
	if(!found)
		return -FDT_ERR_BADPATH;

	/* Cretate handle to phy */
	return fdt_setprop(fdt, offset, "phy-handle", &ph, sizeof(ph));
}

static int fdt_set_phy_mode(void *fdt, int mac, int phy_mode)
{
	int offset;
	const char *path;
	char name[8];
	int lenp;
	int found = 0;
	struct fixed_link f_link;

	/* Get path to dpmacs node */
	offset = fdt_path_offset(fdt, "/soc/fsl-mc/dpmacs");
	if (offset < 0)
		return offset;
	
	/* Get relevant dpmac node */
	sprintf(name, "dpmac@%x", mac);
	fdt_for_each_subnode(offset, fdt, offset) {
		path = fdt_get_name(fdt, offset, &lenp);
		if(!strncmp(name, path, strlen(name))) {
			found++;
			break;
		}
	}
	if(!found)
		return -FDT_ERR_BADPATH;

	/* fixup phy connection type */
	fdt_delprop(fdt, offset, "phy-connection-type");
	if((phy_mode >= PHY_INTERFACE_MODE_RGMII) &&
	   (phy_mode <= PHY_INTERFACE_MODE_RGMII_TXID)) {
	   fdt_setprop_string(fdt, offset, "phy-connection-type",
	      phy_interface_strings[PHY_INTERFACE_MODE_RGMII]);
	   fdt_setprop_string(fdt, offset, "phy-mode",
	      phy_interface_strings[phy_mode]);
	} else if(phy_mode == PHY_INTERFACE_MODE_XGMII) {
		f_link.phy_id = cpu_to_fdt32(mac);
		f_link.duplex = cpu_to_fdt32(1);
		f_link.link_speed = cpu_to_fdt32(1000);
		f_link.pause = 0;
		f_link.asym_pause = 0;
		fdt_delprop(fdt, offset, "phy-handle");
		fdt_setprop(fdt, offset, "fixed-link", &f_link, sizeof(f_link));
		fdt_setprop_string(fdt, offset, "phy-connection-type",
		   phy_interface_strings[PHY_INTERFACE_MODE_XGMII]);
	} else {
	   fdt_setprop_string(fdt, offset, "phy-connection-type",
	      phy_interface_strings[phy_mode]);
	}

	return 0;
}

int tqc_bb_ft_board_setup(void *blob, bd_t *bd)
{
	u32 srds_s1;
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);

	/* read SerDes configuration from RCW */
	srds_s1 = in_le32(&gur->rcwsr[FSL_CHASSIS3_SRDS1_REGSR - 1]) &
	                  FSL_CHASSIS3_SRDS1_PRTCL_MASK;
	srds_s1 >>= FSL_CHASSIS3_SRDS1_PRTCL_SHIFT;


	/* fixup ethernet mapping */
	/* ethernet mapping SerDes 1 - Lane D */
	switch(TQMLS1088A_SRDS1_PROTO(srds_s1, 0)) {
		case 1:
			/* XFI.2 */
			fdt_set_phy_mode(blob, WRIOP1_DPMAC2, PHY_INTERFACE_MODE_XGMII);
			break;
		case 3:
			/* SGMII.2 */
			fdt_set_phy_handle(blob, WRIOP1_DPMAC2, "qsgmii_s2_p2");
			fdt_set_phy_mode(blob, WRIOP1_DPMAC2, PHY_INTERFACE_MODE_SGMII);
			break;
		default:
			/* not supported, nothing to do */
			break;
	}

	/* ethernet mapping SerDes 1 - Lane C */
	switch(TQMLS1088A_SRDS1_PROTO(srds_s1, 1)) {
		case 1:
			/* XFI.1 */
			fdt_set_phy_mode(blob, WRIOP1_DPMAC1, PHY_INTERFACE_MODE_XGMII);
			break;
		default:
			/* not supported, nothing to do */
			break;
	}

	/* ethernet mapping SerDes 1 - Lane B */
	switch(TQMLS1088A_SRDS1_PROTO(srds_s1, 2)) {
		case 3:
			/* SGMII.7 */
			fdt_set_phy_handle(blob, WRIOP1_DPMAC7, "qsgmii_s2_p1");
			fdt_set_phy_mode(blob, WRIOP1_DPMAC7, PHY_INTERFACE_MODE_SGMII);
			break;
		case 4:
			/* QSGMII.7,8,9,10 */
			fdt_set_phy_handle(blob, WRIOP1_DPMAC7, "qsgmii_s2_p1");
			fdt_set_phy_mode(blob, WRIOP1_DPMAC7, PHY_INTERFACE_MODE_QSGMII);
			fdt_set_phy_handle(blob, WRIOP1_DPMAC8, "qsgmii_s2_p2");
			fdt_set_phy_mode(blob, WRIOP1_DPMAC8, PHY_INTERFACE_MODE_QSGMII);
			fdt_set_phy_handle(blob, WRIOP1_DPMAC9, "qsgmii_s2_p3");
			fdt_set_phy_mode(blob, WRIOP1_DPMAC9, PHY_INTERFACE_MODE_QSGMII);
			fdt_set_phy_handle(blob, WRIOP1_DPMAC10, "qsgmii_s2_p4");
			fdt_set_phy_mode(blob, WRIOP1_DPMAC10, PHY_INTERFACE_MODE_QSGMII);
			break;
		default:
			/* not supported, nothing to do */
			break;
	}

	/* ethernet mapping SerDes 1 - Lane A */
	switch(TQMLS1088A_SRDS1_PROTO(srds_s1, 3)) {
		case 3:
			/* SGMII.3 */
			fdt_set_phy_handle(blob, WRIOP1_DPMAC3, "qsgmii_s1_p1");
			fdt_set_phy_mode(blob, WRIOP1_DPMAC3, PHY_INTERFACE_MODE_SGMII);
			/* MAC 4/5 routed to RGMII PHYs */
			fdt_set_phy_handle(blob, WRIOP1_DPMAC4, "rgmii_s1");
			fdt_set_phy_mode(blob, WRIOP1_DPMAC4, PHY_INTERFACE_MODE_RGMII_ID);
			fdt_set_phy_handle(blob, WRIOP1_DPMAC5, "rgmii_s2");
			fdt_set_phy_mode(blob, WRIOP1_DPMAC5, PHY_INTERFACE_MODE_RGMII_ID);
			break;
		case 4:
			/* QSGMII.3,4,5,6 */
			fdt_set_phy_handle(blob, WRIOP1_DPMAC3, "qsgmii_s1_p1");
			fdt_set_phy_mode(blob, WRIOP1_DPMAC3, PHY_INTERFACE_MODE_QSGMII);
			fdt_set_phy_handle(blob, WRIOP1_DPMAC4, "qsgmii_s1_p2");
			fdt_set_phy_mode(blob, WRIOP1_DPMAC4, PHY_INTERFACE_MODE_QSGMII);
			fdt_set_phy_handle(blob, WRIOP1_DPMAC5, "qsgmii_s1_p3");
			fdt_set_phy_mode(blob, WRIOP1_DPMAC5, PHY_INTERFACE_MODE_QSGMII);
			fdt_set_phy_handle(blob, WRIOP1_DPMAC6, "qsgmii_s1_p4");
			fdt_set_phy_mode(blob, WRIOP1_DPMAC6, PHY_INTERFACE_MODE_QSGMII);
			break;
		default:
			/* not supported, nothing to do */
			break;
	}

	return 0;
}
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */
#endif /* !CONFIG_SPL_BUILD */
