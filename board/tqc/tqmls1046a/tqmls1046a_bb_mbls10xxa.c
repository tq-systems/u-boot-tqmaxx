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
#include "tqmls1046a_bb.h"
#include "../common/tqc_mbls10xxa.h"


#define TQMLS1046A_SRDS1_PROTO(cfg, lane)	((cfg >> 4*(3-lane)) & 0xF)
#define TQMLS1046A_SRDS2_PROTO(cfg, lane)	((cfg >> 4*(3-lane)) & 0xF)

const char *serdes_rcw_str[] = {
	"Unused    ",   // 0
	"XFI       ",   // 1
	"2.5G SGMII",   // 2
	"SGMII     ",   // 3
	"QSGMII    ",   // 4
	"PCIe x1   ",   // 5
	"PCIe x1   ",   // 6
	"PCIe x2   ",   // 7
	"PCIe x4   ",   // 8
	"SATA      ",   // 9
	"SGMII     ",   // a
	"Undefined ",   // b
	"Undefined ",   // c
	"Undefined ",   // d
	"Undefined ",   // e
	"Undefined "    // f
};

#ifndef CONFIG_SPL_BUILD
static int _tqmls1046a_bb_check_serdes_mux(void)
{
	u32 srds_s1, srds_s2;
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	int mux_val1, mux_val2;
	int mux_stat = 0;
	int rcw_proto;

	/* read SerDes configuration from RCW */
	srds_s1 = in_be32(&gur->rcwsr[4]) &
			FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_MASK;
	srds_s1 >>= FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_SHIFT;
	srds_s2 = in_be32(&gur->rcwsr[4]) &
			FSL_CHASSIS2_RCWSR4_SRDS2_PRTCL_MASK;
	srds_s2 >>= FSL_CHASSIS2_RCWSR4_SRDS2_PRTCL_SHIFT;

	printf("Checking baseboard SerDes muxing:\n");

	/* check state of SD_MUX_SHDN */
	mux_val1 = tqc_mbls10xxa_i2c_gpio_get("sd_mux_shdn");
	if(mux_val1) {
		printf("!!! ATTENTION: SerDes MUXes disabled,\n");
		printf("!!!  muxed SerDes interfaces won't work\n");
	}
	
	/* check config for SD1 - LANE A */
	mux_val1 = tqc_mbls10xxa_i2c_gpio_get("sd1_3_lane_a_mux");
	rcw_proto = TQMLS1046A_SRDS1_PROTO(srds_s1, 3);
	if(mux_val1 >= 0) {
		printf("  SD1-3 Lane A: MUX=%s | RCW=%s -> ",
		   (mux_val1)?("QSGMII "):("SGMII  "),
		   serdes_rcw_str[rcw_proto]);

		if((rcw_proto == 0x0) ||
		   (!mux_val1 && (rcw_proto == 0x3))
		  ) {
			printf(" OK\n");
		} else {
			mux_stat++;
			printf(" FAIL\n");
		}
	}

	/* check config for SD1 - LANE B */
	mux_val1 = tqc_mbls10xxa_i2c_gpio_get("sd1_2_lane_b_mux");
	rcw_proto = TQMLS1046A_SRDS1_PROTO(srds_s1, 2);
	if(mux_val1 >= 0) {
		printf("  SD1-2 Lane B: MUX=%s | RCW=%s -> ",
		   (mux_val1)?("QSGMII "):("SGMII  "),
		   serdes_rcw_str[rcw_proto]);

		if((rcw_proto == 0x0) ||
		   (!mux_val1 && (rcw_proto == 0x3)) ||
		   (mux_val1 && (rcw_proto == 0x4))
		  ) {
			printf(" OK\n");
		} else {
			mux_stat++;
			printf(" FAIL\n");
		}
	}

	/* check config for SD1 - LANE C */
	rcw_proto = TQMLS1046A_SRDS1_PROTO(srds_s1, 1);
	printf("  SD1-1 Lane C: XFI         | RCW=%s -> ",
	   serdes_rcw_str[rcw_proto]);

	if((rcw_proto == 0x0) ||
	   (rcw_proto == 0x1)
	  ) {
		printf(" OK\n");
	} else {
		mux_stat++;
		printf(" FAIL\n");
	}

	/* check config for SD1 - LANE D */
	mux_val1 = tqc_mbls10xxa_i2c_gpio_get("sd1_0_lane_d_mux");
	rcw_proto = TQMLS1046A_SRDS1_PROTO(srds_s1, 0);
	if(mux_val1 >= 0) {
		printf("  SD1-0 Lane D: MUX=%s | RCW=%s -> ",
		   (mux_val1)?("XFI    "):("SGMII  "),
		   serdes_rcw_str[rcw_proto]);

		if((rcw_proto == 0x0) ||
		   (!mux_val1 && (rcw_proto == 0x3)) ||
		   (mux_val1 && (rcw_proto == 0x1))
		  ) {
			printf(" OK\n");
		} else {
			mux_stat++;
			printf(" FAIL\n");
		}
	}

	/* check config for SD2 - LANE A */
	rcw_proto = TQMLS1046A_SRDS2_PROTO(srds_s2, 0);
	printf("  SD2-0 Lane A: PCIe        | RCW=%s -> ",
	   serdes_rcw_str[rcw_proto]);

	if((rcw_proto == 0x0) ||
	   (rcw_proto == 0x5)
	  ) {
		printf(" OK\n");
	} else {
		mux_stat++;
		printf(" FAIL\n");
	}

	/* check config for SD2 - LANE B */
	mux_val1 = tqc_mbls10xxa_i2c_gpio_get("sd2_1_lane_b_mux");
	rcw_proto = TQMLS1046A_SRDS2_PROTO(srds_s2, 1);
	if(mux_val1 >= 0) {
		printf("  SD2-1 Lane B: MUX=%s | RCW=%s -> ",
		   (mux_val1)?("SGMII  "):("PCIe   "),
		   serdes_rcw_str[rcw_proto]);

		if((rcw_proto == 0x0) ||
		   (!mux_val1 && (rcw_proto == 0x5)) ||
		   (mux_val1 && (rcw_proto == 0xA))
		  ) {
			printf(" OK\n");
		} else {
			mux_stat++;
			printf(" FAIL\n");
		}

		/* enable miniPCIe-Slot when selected */
		if(!mux_val1 && (rcw_proto == 0x5))
			tqc_mbls10xxa_i2c_gpio_set("mpcie2_disable#", 1);
		else
			tqc_mbls10xxa_i2c_gpio_set("mpcie2_disable#", 0);
	}

	/* check config for SD2 - LANE C */
	rcw_proto = TQMLS1046A_SRDS2_PROTO(srds_s2, 2);
	printf("  SD2-2 Lane C: PCIe        | RCW=%s -> ",
	   serdes_rcw_str[rcw_proto]);

	if((rcw_proto == 0x0) ||
	   (rcw_proto == 0x5) ||
	   (rcw_proto == 0x7)
	  ) {
		printf(" OK\n");
	} else {
		mux_stat++;
		printf(" FAIL\n");
	}

	/* get M.2 slot out of reset when lane (SD2-2) is configured as PCIe */
	if((rcw_proto == 0x5) || (rcw_proto == 0x7))
		tqc_mbls10xxa_i2c_gpio_set("pcie_rst_3v3#", 1);
	else
		tqc_mbls10xxa_i2c_gpio_set("pcie_rst_3v3#", 0);

	/* check config for SD2 - LANE D */
	mux_val1 = tqc_mbls10xxa_i2c_gpio_get("sd2_3_lane_d_mux1");
	mux_val2 = tqc_mbls10xxa_i2c_gpio_get("sd2_3_lane_d_mux2");
	rcw_proto = TQMLS1046A_SRDS2_PROTO(srds_s2, 3);
	if((mux_val1 >= 0) && (mux_val2 >= 0)){
		printf("  SD2-3 Lane D: MUX=%s | RCW=%s -> ",
		   (mux_val1)?((mux_val2)?("PCIe   "):("SATA   ")):("PCIe x2"),
		   serdes_rcw_str[rcw_proto]);

		if((rcw_proto == 0x0) ||
		   (!mux_val1 && (rcw_proto == 0x7)) ||
		   (mux_val1 && !mux_val2 && (rcw_proto == 0x9)) ||
		   (mux_val1 && mux_val2 && (rcw_proto == 0x6))
		  ) {
			printf(" OK\n");
		} else {
			mux_stat++;
			printf(" FAIL\n");
		}

		/* enable miniPCIe-Slot when selected */
		if(mux_val1 && mux_val2 && (rcw_proto == 0x6))
			tqc_mbls10xxa_i2c_gpio_set("mpcie1_disable#", 1);
		else
			tqc_mbls10xxa_i2c_gpio_set("mpcie1_disable#", 0);
	}

	/* print error message when muxing is invalid */
	if(mux_stat) {
		printf("!!! ATTENTION: Some SerDes lanes are misconfigured,\n");
		printf("!!!  this may cause some interfaces to be inoperable.\n");
		printf("!!!  Check SerDes muxing DIP switch settings!\n");

		return -1;
	}

	return 0;
}
#endif /* !CONFIG_SPL_BUILD */


int tqmls1046a_bb_board_early_init_f(void)
{
	int ret = 0;

	/* nothing to do */
	return ret;
}

#ifndef CONFIG_SPL_BUILD
int tqmls1046a_bb_board_init(void)
{
	int ret = 0;

	/* nothing to do */
	return ret;
}

int tqmls1046a_bb_misc_init_r(void)
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

	/* initialize baseboard io's */
	ret = tqc_mbls10xxa_i2c_gpios_init();

	/* check if SerDes mux settings match RCW */
	if(!ret)
		ret = _tqmls1046a_bb_check_serdes_mux();

	/* enable USB-C power-controller */
	tqc_mbls10xxa_i2c_gpio_set("usb_c_pwron", 1);

	return 0;
}

int tqmls1046a_bb_checkboard(void)
{
	/* nothing to do */
	return 0;
}

const char *tqmls1046a_bb_get_boardname(void)
{
	return "MBLS10xxA";
}

#ifdef CONFIG_NET
/* 
 * both MDIO busses on MBLS10xxA are used for gigabit PHYs (clause 22),
 * normally MDIO2 is used to access ten-gig PHYs (clause 45),
 * rename MDIO2 to address this issue.
 */
#define FM_MDIO1_NAME			"FSL_MDIO0" /* = DEFAULT_FM_MDIO_NAME */
#define FM_MDIO1_ADDR           CONFIG_SYS_FM1_DTSEC_MDIO_ADDR
#define FM_MDIO2_NAME           "FSL_MDIO1"
#define FM_MDIO2_ADDR           CONFIG_SYS_FM1_TGEC_MDIO_ADDR

int tqmls1046a_bb_board_eth_init(bd_t *bis)
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
	}
	if(TQMLS1046A_SRDS1_PROTO(srds_s1, 1) == 0x1) {
		/* SD1 - LANE C in XFI mode.10 */
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
		/* SD1 - LANE D in SGMII mode.9 */
		fm_info_set_phy_address(FM1_DTSEC9, QSGMII_PHY2_ADDR_BASE+1);
		fm_info_set_mdio(FM1_DTSEC9, dev_mdio1);
	} else if(TQMLS1046A_SRDS1_PROTO(srds_s1, 0) == 0x1) {
		/* SD1 - LANE D in XFI mode.9 */
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
		/* SD2 - LANE B in SGMII mode.2 */
		fm_info_set_phy_address(FM1_DTSEC2, QSGMII_PHY1_ADDR_BASE+1);
		fm_info_set_mdio(FM1_DTSEC2, dev_mdio1);
	}
#endif

	return 0;
}
#endif

static uint16_t _rgmii_phy_read_indirect(struct phy_device *phydev,
					uint8_t addr)
{
	phy_write(phydev, MDIO_DEVAD_NONE, 0x0d, 0x001f);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x0e, addr);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x0d, 0x401f);
	return phy_read(phydev, MDIO_DEVAD_NONE, 0x0e);
}

static void _rgmii_phy_write_indirect(struct phy_device *phydev,
					uint8_t addr, uint16_t value)
{
	phy_write(phydev, MDIO_DEVAD_NONE, 0x0d, 0x001f);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x0e, addr);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x0d, 0x401f);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x0e, value);
}

int board_phy_config(struct phy_device *phydev)
{
	uint16_t val;
	int ret = 0;

	if (phydev->drv->config)
		ret = phydev->drv->config(phydev);

	if(!ret) {
		switch(phydev->addr) {
		case RGMII_PHY1_ADDR:
		case RGMII_PHY2_ADDR:
			/* enable RGMII delay in both directions */
			val = _rgmii_phy_read_indirect(phydev, 0x32);
			val |= 0x0003;
			_rgmii_phy_write_indirect(phydev, 0x32, val);

			/* set RGMII delay in both directions to 1,5ns */
			val = _rgmii_phy_read_indirect(phydev, 0x86);
			val = (val & 0xFF00) | 0x0055;
			_rgmii_phy_write_indirect(phydev, 0x86, val);
			break;
		case (QSGMII_PHY1_ADDR_BASE & 0x1C):
		case (QSGMII_PHY2_ADDR_BASE & 0x1C):
			/* execute marvell specified initialization if not already done */
			/* see MV-S301615 release note */
			/* PHY initialization #1 */
			phy_write(phydev, MDIO_DEVAD_NONE, 0x16, 0x00ff);
			phy_write(phydev, MDIO_DEVAD_NONE, 0x17, 0x2001);
			val = phy_read(phydev, MDIO_DEVAD_NONE, 0x19);
			if(val != 0x2800) {
				phy_write(phydev, MDIO_DEVAD_NONE, 0x18, 0x2800);
				phy_write(phydev, MDIO_DEVAD_NONE, 0x17, 0x2001);
			}
			phy_write(phydev, MDIO_DEVAD_NONE, 0x16, 0x0000);
			/* PHY initialization #2 */
			phy_write(phydev, MDIO_DEVAD_NONE, 0x16, 0x0000);
			phy_write(phydev, MDIO_DEVAD_NONE, 0x1D, 0x0003);
			val = phy_read(phydev, MDIO_DEVAD_NONE, 0x1e);
			if(val != 0x0002) {
				phy_write(phydev, MDIO_DEVAD_NONE, 0x1E, 0x0002);
			}
			phy_write(phydev, MDIO_DEVAD_NONE, 0x16, 0x0000);
			break;
		default:
			break;
		}
	}

	return ret;
}


#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
int tqmls1046a_bb_ft_board_setup(void *blob, bd_t *bd)
{
	/* nothing to do */
	return 0;
}
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */
#endif /* !CONFIG_SPL_BUILD */
