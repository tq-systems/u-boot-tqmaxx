// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 TQ-Systems GmbH
 */

#include <common.h>
#include <asm/io.h>
#include <netdev.h>
#include <fm_eth.h>
#include <fsl_dtsec.h>
#include <fsl_mdio.h>
#include <malloc.h>
#include <asm/arch/immap_lsch2.h>
#include "../common/tqc_bb.h"
#include "../common/tqc_mbls10xxa.h"


#ifndef CONFIG_SPL_BUILD
#define TQMLS1046A_SRDS1_PROTO(cfg, lane)	((cfg >> 4*(3-lane)) & 0xF)
#define TQMLS1046A_SRDS2_PROTO(cfg, lane)	((cfg >> 4*(3-lane)) & 0xF)

static int _tqmls1046a_bb_set_lane(struct tqc_mbls10xxa_serdes *serdes, int port, int lane, int proto)
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
		case 0xa: /* SGMII (alternate) */
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

static void _tqmls1046a_bb_serdes_cfg(void)
{
	u32 srds_s1, srds_s2;
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	struct tqc_mbls10xxa_serdes lanes[8];
	int clk_freq;

	/* read SerDes configuration from RCW */
	srds_s1 = in_be32(&gur->rcwsr[4]) &
			FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_MASK;
	srds_s1 >>= FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_SHIFT;
	srds_s2 = in_be32(&gur->rcwsr[4]) &
			FSL_CHASSIS2_RCWSR4_SRDS2_PRTCL_MASK;
	srds_s2 >>= FSL_CHASSIS2_RCWSR4_SRDS2_PRTCL_SHIFT;

	/* fill lane usage table */
	_tqmls1046a_bb_set_lane(&(lanes[0]), 1, 0, TQMLS1046A_SRDS1_PROTO(srds_s1, 0));
	_tqmls1046a_bb_set_lane(&(lanes[1]), 1, 1, TQMLS1046A_SRDS1_PROTO(srds_s1, 1));
	_tqmls1046a_bb_set_lane(&(lanes[2]), 1, 2, TQMLS1046A_SRDS1_PROTO(srds_s1, 2));
	_tqmls1046a_bb_set_lane(&(lanes[3]), 1, 3, TQMLS1046A_SRDS1_PROTO(srds_s1, 3));
	_tqmls1046a_bb_set_lane(&(lanes[4]), 2, 0, TQMLS1046A_SRDS2_PROTO(srds_s2, 0));
	_tqmls1046a_bb_set_lane(&(lanes[5]), 2, 1, TQMLS1046A_SRDS2_PROTO(srds_s2, 1));
	_tqmls1046a_bb_set_lane(&(lanes[6]), 2, 2, TQMLS1046A_SRDS2_PROTO(srds_s2, 2));
	_tqmls1046a_bb_set_lane(&(lanes[7]), 2, 3, TQMLS1046A_SRDS2_PROTO(srds_s2, 3));

	/* check and init lane usage */
	tqc_mbls10xxa_serdes_init(lanes);

	/* check serdes clock muxing */
	clk_freq = tqc_mbls10xxa_serdes_clk_get(TQC_MBLS10xxA_SRDS_CLK_1_2);
	if((clk_freq == 125000000) &&
	   ((TQMLS1046A_SRDS1_PROTO(srds_s1, 0) == 0x1) ||
	    (TQMLS1046A_SRDS1_PROTO(srds_s1, 1) == 0x1))) {
		printf("!!! ATTENTON: SerDes1 RefClk2 is not 156.25MHz,\n");
		printf("!!!  but this is needed for XFI operation!\n");
	} else if((clk_freq == 156250000) && 
	   ((TQMLS1046A_SRDS1_PROTO(srds_s1, 0) != 0x1) &&
	    (TQMLS1046A_SRDS1_PROTO(srds_s1, 1) != 0x1))) {
		printf("!!! ATTENTON: SerDes1 RefClk2 is 156.25MHz,\n");
		printf("!!!  but this is only valid for XFI operation!\n");
	}

	tqc_mbls10xxa_retimer_init();
}
#endif /* !CONFIG_SPL_BUILD */


int tqc_bb_board_early_init_f(void)
{
	struct ccsr_gpio *ccsr = (void *)(GPIO4_BASE_ADDR);
	uint32_t reg;
	int ret = 0;

	/* configure sd card detect and write protect GPIOs as input */
	/* CD = GPIO4_2, WP = GPIO4_3 */
	reg = in_be32(&ccsr->gpdir);
	reg &= ~(0x30000000);
	out_be32(&ccsr->gpdir, reg);

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
#ifdef CONFIG_HAS_FSL_XHCI_USB
	struct ccsr_scfg *scfg = (struct ccsr_scfg *)CONFIG_SYS_FSL_SCFG_ADDR;
	u32 usb_pwrfault;

	/* USB3 pwr-ctrl is not used, configure mux to IIC4_SCL/IIC4_SDA */
	out_be32(&scfg->rcwpmuxcr0, 0x3300);
	out_be32(&scfg->usbdrvvbus_selcr, SCFG_USBDRVVBUS_SELCR_USB1);
#if 0 
	usb_pwrfault = (SCFG_USBPWRFAULT_INACTIVE <<
			SCFG_USBPWRFAULT_USB3_SHIFT) |
			(SCFG_USBPWRFAULT_DEDICATED <<
			SCFG_USBPWRFAULT_USB2_SHIFT) |
			(SCFG_USBPWRFAULT_SHARED <<
			SCFG_USBPWRFAULT_USB1_SHIFT);
#else
	/* power-fault signal from baseboard is inverted, ignore */
	usb_pwrfault = 0;
#endif
	out_be32(&scfg->usbpwrfault_selcr, usb_pwrfault);
#endif

	/* configure baseboard clock buffer */
	tqc_mbls10xxa_clk_cfg_init();

	/* initialize baseboard io's */
	ret = tqc_mbls10xxa_i2c_gpios_init();

	/* check if SerDes mux settings match RCW and configure */
	if(!ret)
		_tqmls1046a_bb_serdes_cfg();

	/* enable USB-C power-controller */
	tqc_mbls10xxa_i2c_gpio_set("usb_c_pwron", 0);
	mdelay(10);
	tqc_mbls10xxa_i2c_gpio_set("usb_c_pwron", 1);

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
#ifdef CONFIG_FMAN_ENET
	struct memac_mdio_info dtsec_mdio1_info;
	struct memac_mdio_info dtsec_mdio2_info;
	struct mii_dev *dev_mdio1;
	u32 srds_s1, srds_s2;
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	int moddef_det;

	/* read SerDes configuration from RCW */
	srds_s1 = in_be32(&gur->rcwsr[4]) &
			FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_MASK;
	srds_s1 >>= FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_SHIFT;
	srds_s2 = in_be32(&gur->rcwsr[4]) &
			FSL_CHASSIS2_RCWSR4_SRDS2_PRTCL_MASK;
	srds_s2 >>= FSL_CHASSIS2_RCWSR4_SRDS2_PRTCL_SHIFT;

	/* register the MDIO bus 1 */
	dtsec_mdio1_info.regs =
		(struct memac_mdio_controller *)CONFIG_SYS_FM1_DTSEC_MDIO_ADDR;
	dtsec_mdio1_info.name = DEFAULT_FM_MDIO_NAME;
	fm_memac_mdio_init(bis, &dtsec_mdio1_info);

	/* register the MDIO bus 2 */
	dtsec_mdio2_info.regs =
		(struct memac_mdio_controller *)CONFIG_SYS_FM1_TGEC_MDIO_ADDR;
	dtsec_mdio2_info.name = DEFAULT_FM_TGEC_MDIO_NAME;
	fm_memac_mdio_init(bis, &dtsec_mdio2_info);

	/* get MDIO bus device */
	dev_mdio1 = miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME);

	/* set the two on-board RGMII PHY address */
	fm_info_set_phy_address(FM1_DTSEC3, RGMII_PHY1_ADDR);
	fm_info_set_mdio(FM1_DTSEC3, dev_mdio1);
	fm_info_set_phy_address(FM1_DTSEC4, RGMII_PHY2_ADDR);
	fm_info_set_mdio(FM1_DTSEC4, dev_mdio1);

	/* set SGMII/QSGMII PHY addresses based on RCW */
	if(TQMLS1046A_SRDS1_PROTO(srds_s1, 3) == 0x3) {
		/* SD1 - LANE A in SGMII.6 mode */
		fm_info_set_phy_address(FM1_DTSEC6, QSGMII_PHY1_ADDR_BASE+0);
		fm_info_set_mdio(FM1_DTSEC6, dev_mdio1);
	}
	if(TQMLS1046A_SRDS1_PROTO(srds_s1, 2) == 0x3) {
		/* SD1 - LANE B in SGMII.5 mode */
		fm_info_set_phy_address(FM1_DTSEC5, QSGMII_PHY2_ADDR_BASE+0);
		fm_info_set_mdio(FM1_DTSEC5, dev_mdio1);
	} else if(TQMLS1046A_SRDS1_PROTO(srds_s1, 2) == 0x4) {
		/* SD1 - LANE B in QSGMII.6,5,10,1 mode */
		fm_info_set_phy_address(FM1_DTSEC6, QSGMII_PHY2_ADDR_BASE+0);
		fm_info_set_mdio(FM1_DTSEC6, dev_mdio1);
		fm_info_set_phy_address(FM1_DTSEC5, QSGMII_PHY2_ADDR_BASE+1);
		fm_info_set_mdio(FM1_DTSEC5, dev_mdio1);
		fm_info_set_phy_address(FM1_DTSEC10, QSGMII_PHY2_ADDR_BASE+2);
		fm_info_set_mdio(FM1_DTSEC1, dev_mdio1);
		fm_info_set_phy_address(FM1_DTSEC1, QSGMII_PHY2_ADDR_BASE+3);
		fm_info_set_mdio(FM1_DTSEC10, dev_mdio1);
	}
	if(TQMLS1046A_SRDS1_PROTO(srds_s1, 1) == 0x1) {
		/* SD1 - LANE C in XFI.10 mode */
		/* enable XFI transmitter when XFI selected and SFP is available
		 * (MOD-DEF[0] (SFP pin 6) is grounded)
		 */
		moddef_det = tqc_mbls10xxa_i2c_gpio_get("xfi1_moddef_det");
		if(!moddef_det)
			tqc_mbls10xxa_i2c_gpio_set("xfi1_tx_dis", 0);
		else
			tqc_mbls10xxa_i2c_gpio_set("xfi1_tx_dis", 1);
	}
	if(TQMLS1046A_SRDS1_PROTO(srds_s1, 0) == 0x3) {
		/* SD1 - LANE D in SGMII.9 mode */
		fm_info_set_phy_address(FM1_DTSEC9, QSGMII_PHY2_ADDR_BASE+1);
		fm_info_set_mdio(FM1_DTSEC9, dev_mdio1);
	} else if(TQMLS1046A_SRDS1_PROTO(srds_s1, 0) == 0x1) {
		/* SD1 - LANE D in XFI.9 mode */
		/* enable XFI transmitter when XFI selected and SFP is available
		 * (MOD-DEF[0] (SFP pin 6) is grounded)
		 */
		moddef_det = tqc_mbls10xxa_i2c_gpio_get("xfi2_moddef_det");
		if(!moddef_det)
			tqc_mbls10xxa_i2c_gpio_set("xfi2_tx_dis", 0);
		else
			tqc_mbls10xxa_i2c_gpio_set("xfi2_tx_dis", 1);
	}
	if(TQMLS1046A_SRDS2_PROTO(srds_s2, 1) == 0xA) {
		/* SD2 - LANE B in SGMII.2 mode */
		fm_info_set_phy_address(FM1_DTSEC2, QSGMII_PHY1_ADDR_BASE+1);
		fm_info_set_mdio(FM1_DTSEC2, dev_mdio1);
	}
#endif

	return 0;
}
#endif

int tqc_bb_board_mmc_init(bd_t *bis)
{
	struct ccsr_gpio *ccsr = (void *)(GPIO4_BASE_ADDR);
	uint32_t reg;

	/* configure sd card detect and write protect GPIOs as input */
	/* CD = GPIO4_2, WP = GPIO4_3 */
	reg = in_be32(&ccsr->gpdir);
	reg &= ~(0x30000000);
	out_be32(&ccsr->gpdir, reg);

	return 0;
}

int tqc_bb_board_mmc_getcd(struct mmc *mmc)
{
	struct ccsr_gpio *ccsr = (void *)(GPIO4_BASE_ADDR);

	return !(in_be32(&ccsr->gpdat) & (1 << (31-2)));
}

int tqc_bb_board_mmc_getwp(struct mmc *mmc)
{
	struct ccsr_gpio *ccsr = (void *)(GPIO4_BASE_ADDR);

	return !!(in_be32(&ccsr->gpdat) & (1 << (31-3)));
}

int board_phy_config(struct phy_device *phydev)
{
	/* call baseboard common function */
	return tqc_mbls10xxa_board_phy_config(phydev);
}

static int fdt_set_phy_handle(void *fdt, char *compat, phys_addr_t addr,
			const char *alias)
{
	int offset;
	unsigned int ph;
	const char *path;

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

	offset = fdt_node_offset_by_compat_reg(fdt, compat, addr);
	if (offset < 0)
		return offset;

	return fdt_setprop(fdt, offset, "phy-handle", &ph, sizeof(ph));
}

void board_ft_fman_fixup_port(void *fdt, char *compat, phys_addr_t addr,
			      enum fm_port port, int offset)
{
	struct fixed_link f_link;

	if (fm_info_get_enet_if(port) == PHY_INTERFACE_MODE_SGMII) {
		switch (port) {
		case FM1_DTSEC2:
			fdt_set_phy_handle(fdt, compat, addr, "qsgmii_s1_p2");
			break;
		case FM1_DTSEC5:
			fdt_set_phy_handle(fdt, compat, addr, "qsgmii_s2_p1");
			break;
		case FM1_DTSEC6:
			fdt_set_phy_handle(fdt, compat, addr, "qsgmii_s1_p1");
			break;
		case FM1_DTSEC9:
			fdt_set_phy_handle(fdt, compat, addr, "qsgmii_s2_p2");
			break;
		default:
			break;
		fdt_delprop(fdt, offset, "phy-connection-type");
		fdt_setprop_string(fdt, offset, "phy-connection-type", "sgmii");
		}
	} else if (fm_info_get_enet_if(port) == PHY_INTERFACE_MODE_QSGMII) {
		switch (port) {
		case FM1_DTSEC1:
			fdt_set_phy_handle(fdt, compat, addr, "qsgmii_s2_p4");
			break;
		case FM1_DTSEC5:
			fdt_set_phy_handle(fdt, compat, addr, "qsgmii_s2_p2");
			break;
		case FM1_DTSEC6:
			fdt_set_phy_handle(fdt, compat, addr, "qsgmii_s2_p1");
			break;
		case FM1_DTSEC10:
			fdt_set_phy_handle(fdt, compat, addr, "qsgmii_s2_p3");
			break;
		default:
			break;
		}
		fdt_delprop(fdt, offset, "phy-connection-type");
		fdt_setprop_string(fdt, offset, "phy-connection-type", "qsgmii");
	} else if (fm_info_get_enet_if(port) == PHY_INTERFACE_MODE_XGMII &&
		   (port == FM1_10GEC1 || port == FM1_10GEC2)) {
		/* XFI interface */
		f_link.phy_id = cpu_to_fdt32(port);
		f_link.duplex = cpu_to_fdt32(1);
		f_link.link_speed = cpu_to_fdt32(1000);
		f_link.pause = 0;
		f_link.asym_pause = 0;
		/* no PHY for XFI */
		fdt_delprop(fdt, offset, "phy-handle");
		fdt_delprop(fdt, offset, "phy-connection-type");
		fdt_setprop(fdt, offset, "fixed-link", &f_link, sizeof(f_link));
		fdt_setprop_string(fdt, offset, "phy-connection-type", "xgmii");
	}
}

#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
int tqc_bb_ft_board_setup(void *blob, bd_t *bd)
{
	/* nothing to do */
	return 0;
}
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */
#endif /* !CONFIG_SPL_BUILD */
