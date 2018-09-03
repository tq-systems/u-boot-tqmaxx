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


int tqmls1046a_bb_board_early_init_f(void)
{
	/* nothing to do */
	return 0;
}

int tqmls1046a_bb_board_init(void)
{
	/* nothing to do */
	return 0;
}

int tqmls1046a_bb_misc_init_r(void)
{
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

	return 0;
}

int tqmls1046a_bb_checkboard(void)
{
	/*
	 * TODO: add further baseboard specific information (e.g. serdes clocks, ...)
	 */
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
	struct mii_dev *dev;

	/* Register the MDIO bus 1 */
	dtsec_mdio1_info.regs =
		(struct memac_mdio_controller *)FM_MDIO1_ADDR;
	dtsec_mdio1_info.name = FM_MDIO1_NAME;
	fm_memac_mdio_init(bis, &dtsec_mdio1_info);

	/* Register the MDIO bus 2 */
	dtsec_mdio2_info.regs =
		(struct memac_mdio_controller *)FM_MDIO2_ADDR;
	dtsec_mdio2_info.name = FM_MDIO2_NAME;
	fm_memac_mdio_init(bis, &dtsec_mdio2_info);

	/* Set the two on-board RGMII PHY address */
	fm_info_set_phy_address(FM1_DTSEC3, RGMII_PHY1_ADDR);
	fm_info_set_phy_address(FM1_DTSEC4, RGMII_PHY2_ADDR);

	/* DTSEC3 (RGMII1) on FM_MDIO */
	dev = miiphy_get_dev_by_name(FM_MDIO1_NAME);
	fm_info_set_mdio(FM1_DTSEC3, dev);

	/* DTSEC4 (RGMII2) on FM_TGEC_MDIO */
	dev = miiphy_get_dev_by_name(FM_MDIO2_NAME);
	fm_info_set_mdio(FM1_DTSEC4, dev);

	/* TODO: add settings for SerDes ethernet ports */
#endif

	return 0;
}
#endif

#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
int tqmls1046a_bb_ft_board_setup(void *blob, bd_t *bd)
{
	/* nothing to do */
	return 0;
}
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */

