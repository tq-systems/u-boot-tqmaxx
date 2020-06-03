// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 *
 */

#include <common.h>
#include <command.h>
#include <netdev.h>
#include <malloc.h>
#include <fsl_mdio.h>
#include <miiphy.h>
#include <phy.h>
#include <fm_eth.h>
#include <asm/io.h>
#include <exports.h>
#include <asm/arch/fsl_serdes.h>
#include <fsl-mc/fsl_mc.h>
#include <fsl-mc/ldpaa_wriop.h>

DECLARE_GLOBAL_DATA_PTR;

int board_eth_init(bd_t *bis)
{
#if defined(CONFIG_FSL_MC_ENET)
	struct memac_mdio_info mdio_info;
	struct memac_mdio_controller *reg;
	int i, interface;
	struct mii_dev *dev;
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 srds_s1;

	srds_s1 = in_le32(&gur->rcwsr[28]) &
				FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_MASK;
	srds_s1 >>= FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_SHIFT;

	reg = (struct memac_mdio_controller *)CONFIG_SYS_FSL_WRIOP1_MDIO1;
	mdio_info.regs = reg;
	mdio_info.name = DEFAULT_WRIOP_MDIO1_NAME;

	/* Register the EMI 1 */
	fm_memac_mdio_init(bis, &mdio_info);

	reg = (struct memac_mdio_controller *)CONFIG_SYS_FSL_WRIOP1_MDIO2;
	mdio_info.regs = reg;
	mdio_info.name = DEFAULT_WRIOP_MDIO2_NAME;

	/* Register the EMI 2 */
	fm_memac_mdio_init(bis, &mdio_info);

	wriop_set_phy_address(WRIOP1_DPMAC17, 0, 0x3);
	wriop_set_phy_address(WRIOP1_DPMAC18, 0, 0x4);
	dev = miiphy_get_dev_by_name(DEFAULT_WRIOP_MDIO2_NAME);
	wriop_set_mdio(WRIOP1_DPMAC17, dev);

	cpu_eth_init(bis);
#endif /* CONFIG_FSL_MC_ENET */

	return pci_eth_init(bis);
}

#if defined(CONFIG_RESET_PHY_R)
void reset_phy(void)
{
#if defined(CONFIG_FSL_MC_ENET)
	mc_env_boot();
#endif
}
#endif /* CONFIG_RESET_PHY_R */

extern int phy_read_mmd_indirect(struct phy_device *phydev, int prtad, int devad, int addr);
extern void phy_write_mmd_indirect(struct phy_device *phydev, int prtad, int devad, int addr, u32 data);

int board_phy_config(struct phy_device *phydev)
{
	int val;

	printf("init phy on addr 0x%x\n", phydev->addr);
	val = phy_read_mmd_indirect(phydev, 0x32, 0x1f, phydev->addr);
	val |= 0x0003;
	phy_write_mmd_indirect(phydev, 0x32,  0x1f, phydev->addr, val);

	/* set RGMII delay in both directions to 1,5ns */
	val = phy_read_mmd_indirect(phydev, 0x86, 0x1f, phydev->addr);
	val = (val & 0xFF00) | 0x0055;
	phy_write_mmd_indirect(phydev, 0x86,  0x1f, phydev->addr, val);

	return 0;
}

int fdt_fixup_board_phy(void *fdt)
{
	int mdio_offset;
	int ret;
	struct mii_dev *dev;

	ret = 0;

	return ret;
}
