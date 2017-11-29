/*
 * Copyright 2016 TQ Systems GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "tqmls102xa_bb.h"


int checkboard(void)
{
	puts("Board: MBLS102xA.0200\n");

	return 0;
}

#ifdef CONFIG_TSEC_ENET

int board_eth_init(bd_t *bis)
{
	struct fsl_pq_mdio_info mdio_info;
	struct tsec_info_struct tsec_info[4];
	int num = 0;
	struct phy_device *phy;
	int regval;

#ifdef CONFIG_TSEC1
	SET_STD_TSEC_INFO(tsec_info[num], 1);
	tsec_info[num].interface = PHY_INTERFACE_MODE_RGMII_ID;
	num++;
#endif
#ifdef CONFIG_TSEC2
	SET_STD_TSEC_INFO(tsec_info[num], 2);
	num++;
#endif
#ifdef CONFIG_TSEC3
	SET_STD_TSEC_INFO(tsec_info[num], 3);
	tsec_info[num].interface = PHY_INTERFACE_MODE_RGMII_ID;
	num++;
#endif
	if (!num) {
		printf("No TSECs initialized\n");
		return 0;
	}

#ifdef CONFIG_FSL_SGMII_RISER
	fsl_sgmii_riser_init(tsec_info, num);
#endif

	mdio_info.regs = (struct tsec_mii_mng *)CONFIG_SYS_MDIO_BASE_ADDR;
	mdio_info.name = DEFAULT_MII_NAME;
	fsl_pq_mdio_init(bis, &mdio_info);

	tsec_eth_init(bis, tsec_info, num);

#ifdef CONFIG_TSEC1
	phy = mdio_phydev_for_ethname(CONFIG_TSEC1_NAME);
	if (phy) {
		/* set GPIO to out low */
		phy_write_mmd_indirect(phy, 0x0171, DP83867_DEVADDR,
				       TSEC1_PHY_ADDR, 0x8888);
		phy_write_mmd_indirect(phy, 0x0172, DP83867_DEVADDR,
				       TSEC1_PHY_ADDR, 0x0888);
		/* LED configuration */
		phy_write(phy, TSEC1_PHY_ADDR, 0x18, 0x6b90);
		phy_write(phy, TSEC1_PHY_ADDR, 0x19, 0x0000);
	} else {
		printf("Unregistering %s\n", CONFIG_TSEC1_NAME);
		eth_unregister(eth_get_dev_by_name(CONFIG_TSEC1_NAME));
	}
#endif
#ifdef CONFIG_TSEC2
	phy = mdio_phydev_for_ethname(CONFIG_TSEC2_NAME);
	if (phy) {
		/* set GPIO to out low */
		phy_write_mmd_indirect(phy, 0x0171, DP83867_DEVADDR,
				       TSEC2_PHY_ADDR, 0x8888);
		phy_write_mmd_indirect(phy, 0x0172, DP83867_DEVADDR,
				       TSEC2_PHY_ADDR, 0x0888);
		/*
		 * workaround:
		 * RX_CTRL mode 0 strapping had to be mode 3
		 * clear bit[7] of CFG4
		 */
		regval = phy_read_mmd_indirect(phy, 0x0031, DP83867_DEVADDR,
					       TSEC1_PHY_ADDR);
		phy_write_mmd_indirect(phy, 0x0031, DP83867_DEVADDR,
				       TSEC1_PHY_ADDR, regval & ~(0x0080));
		printf("DP83867: Cleared bit[7] of CFG4 (RX_CTRL mode fix)\n");
		/* LED configuration */
		phy_write(phy, TSEC2_PHY_ADDR, 0x18, 0x6b90);
		phy_write(phy, TSEC2_PHY_ADDR, 0x19, 0x0000);
	}
#endif
#ifdef CONFIG_TSEC3
	phy = mdio_phydev_for_ethname(CONFIG_TSEC3_NAME);
	if (phy) {
		/* set GPIO to out low */
		phy_write_mmd_indirect(phy, 0x0171, DP83867_DEVADDR,
				       TSEC3_PHY_ADDR, 0x8888);
		phy_write_mmd_indirect(phy, 0x0172, DP83867_DEVADDR,
				       TSEC3_PHY_ADDR, 0x0888);
		/* enable clock out */
		regval = phy_read_mmd_indirect(phy, 0x0170, DP83867_DEVADDR,
					       TSEC3_PHY_ADDR);
		phy_write_mmd_indirect(phy, 0x0170, DP83867_DEVADDR,
				       TSEC3_PHY_ADDR, regval | 0x040);
		/* LED configuration */
		phy_write(phy, TSEC3_PHY_ADDR, 0x18, 0x6b90);
		phy_write(phy, TSEC3_PHY_ADDR, 0x19, 0x0000);
	}
#endif

	return pci_eth_init(bis);
}

void tqmls102xa_bb_early_init(void)
{
#ifdef CONFIG_TSEC_ENET
	struct ccsr_scfg *scfg = (struct ccsr_scfg *)CONFIG_SYS_FSL_SCFG_ADDR;

	/* clear BD & FR bits for BE BD's and frame data */
	clrbits_be32(&scfg->etsecdmamcr, SCFG_ETSECDMAMCR_LE_BD_FR);
	out_be32(&scfg->etsecmcr, SCFG_ETSECCMCR_GE2_CLK125);
#endif
}

void mbls102xa_cpld_config(void)
{
	unsigned int oldbus, i;
	uchar pca_in, pca_out, pca_cfg = 0x0f;

	/* display cpld_mux dip-switch-values on 4 LEDs */
	oldbus = i2c_get_bus_num();
	i2c_set_bus_num(0);
	i2c_read(0x22, 0, 1, &pca_in, 1);
	/* assign input values to led outputs */
	pca_out = (pca_in & 0x01) << 7 | (pca_in & 0x02) << 5 |
		  (pca_in & 0x04) << 3 | (pca_in & 0x08) << 1;
	/* set direction */
	i2c_write(0x22, 3, 1, &pca_cfg, 1);
	/* set output */
	i2c_write(0x22, 1, 1, &pca_out, 1);

	printf("CPLD:  ");
	for(i = 0; i <= 3; i++)
		printf("%d",(pca_in>>i) & 0x01);
	printf("\n");

	i2c_set_bus_num(oldbus);
}

void tqmls102xa_bb_late_init(void)
{
	mbls102xa_cpld_config();

	ls1021a_sata_init();
}
#endif
