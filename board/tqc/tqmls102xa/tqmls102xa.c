/*
 * Copyright 2016 TQ Systems GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <i2c.h>
#include <asm/io.h>
#include <asm/arch/immap_ls102xa.h>
#include <asm/arch/clock.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/ls102xa_soc.h>
#include <asm/arch/ls102xa_sata.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <mmc.h>
#include <fsl_csu.h>
#include <fsl_esdhc.h>
#include <fsl_immap.h>
#include <fsl_ifc.h>
#include <netdev.h>
#include <fsl_mdio.h>
#include <tsec.h>
#include <fsl_sec.h>
#include <spl.h>
#include "../common/sleep.h"
#ifdef CONFIG_U_QE
#include <fsl_qe.h>
#endif
#include "tqmls102xa_eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	unsigned int svr, ver;

	svr = in_be32(&gur->svr);

	puts("Board: TQM");
	ver = SVR_SOC_VER(svr);
	switch (ver) {
	case SOC_VER_LS1020:
		puts("LS1020A");
		break;
	case SOC_VER_LS1021:
		puts("LS1021A");
		break;
	case SOC_VER_LS1022:
		puts("LS1022A");
		break;
	default:
		puts("Unknown");
		break;
	}

	puts(" on MBLS102xa\n");
	return 0;
}

void ddrmc_init(void)
{
	struct ccsr_ddr *ddr = (struct ccsr_ddr *)CONFIG_SYS_FSL_DDR_ADDR;
	u32 temp_sdram_cfg;

	out_be32(&ddr->sdram_cfg, DDR_SDRAM_CFG);

	out_be32(&ddr->cs0_bnds, DDR_CS0_BNDS);
	out_be32(&ddr->cs0_config, DDR_CS0_CONFIG);

	out_be32(&ddr->timing_cfg_0, DDR_TIMING_CFG_0);
	out_be32(&ddr->timing_cfg_1, DDR_TIMING_CFG_1);
	out_be32(&ddr->timing_cfg_2, DDR_TIMING_CFG_2);
	out_be32(&ddr->timing_cfg_3, DDR_TIMING_CFG_3);
	out_be32(&ddr->timing_cfg_4, DDR_TIMING_CFG_4);
	out_be32(&ddr->timing_cfg_5, DDR_TIMING_CFG_5);

#ifdef CONFIG_DEEP_SLEEP
	if (is_warm_boot()) {
		out_be32(&ddr->sdram_cfg_2,
				 DDR_SDRAM_CFG_2 & ~SDRAM_CFG2_D_INIT);
		out_be32(&ddr->init_addr, CONFIG_SYS_SDRAM_BASE);
		out_be32(&ddr->init_ext_addr, (1 << 31));

		/* DRAM VRef will not be trained */
		out_be32(&ddr->ddr_cdr2,
				 DDR_DDR_CDR2 & ~DDR_CDR2_VREF_TRAIN_EN);
	} else
#endif
	{
		out_be32(&ddr->sdram_cfg_2, DDR_SDRAM_CFG_2);
		out_be32(&ddr->ddr_cdr2, DDR_DDR_CDR2);
	}

	out_be32(&ddr->sdram_mode, DDR_SDRAM_MODE);
	out_be32(&ddr->sdram_mode_2, DDR_SDRAM_MODE_2);

	out_be32(&ddr->sdram_interval, DDR_SDRAM_INTERVAL);

	out_be32(&ddr->ddr_wrlvl_cntl, DDR_DDR_WRLVL_CNTL);

	out_be32(&ddr->ddr_wrlvl_cntl_2, DDR_DDR_WRLVL_CNTL_2);
	out_be32(&ddr->ddr_wrlvl_cntl_3, DDR_DDR_WRLVL_CNTL_3);

	out_be32(&ddr->ddr_cdr1, DDR_DDR_CDR1);

	out_be32(&ddr->sdram_clk_cntl, DDR_SDRAM_CLK_CNTL);
	out_be32(&ddr->ddr_zq_cntl, DDR_DDR_ZQ_CNTL);

	out_be32(&ddr->cs0_config_2, DDR_CS0_CONFIG_2);
	udelay(1);

#ifdef CONFIG_DEEP_SLEEP
	if (is_warm_boot()) {
		/* enter self-refresh */
		temp_sdram_cfg = in_be32(&ddr->sdram_cfg_2);
		temp_sdram_cfg |= SDRAM_CFG2_FRC_SR;
		out_be32(&ddr->sdram_cfg_2, temp_sdram_cfg);

		temp_sdram_cfg = (DDR_SDRAM_CFG_MEM_EN | SDRAM_CFG_BI);
	} else
#endif
		temp_sdram_cfg = (DDR_SDRAM_CFG_MEM_EN & ~SDRAM_CFG_BI);

	out_be32(&ddr->sdram_cfg, DDR_SDRAM_CFG | temp_sdram_cfg);

#ifdef CONFIG_DEEP_SLEEP
	if (is_warm_boot()) {
		/* exit self-refresh */
		temp_sdram_cfg = in_be32(&ddr->sdram_cfg_2);
		temp_sdram_cfg &= ~SDRAM_CFG2_FRC_SR;
		out_be32(&ddr->sdram_cfg_2, temp_sdram_cfg);
	}
#endif
}

int dram_init(void)
{
#if (!defined(CONFIG_SPL) || defined(CONFIG_SPL_BUILD))
	ddrmc_init();
#endif

	gd->ram_size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE);

#if defined(CONFIG_DEEP_SLEEP) && !defined(CONFIG_SPL_BUILD)
	fsl_dp_resume();
#endif

	return 0;
}

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg esdhc_cfg[1] = {
	{CONFIG_SYS_FSL_ESDHC_ADDR},
};

int board_mmc_init(bd_t *bis)
{
	esdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);

	return fsl_esdhc_initialize(bis, &esdhc_cfg[0]);
}
#endif

#ifdef CONFIG_TSEC_ENET
extern int phy_read_mmd_indirect(struct phy_device *phydev, int prtad,
			  int devad, int addr);
extern void phy_write_mmd_indirect(struct phy_device *phydev, int prtad,
			    int devad, int addr, u32 data);
#define DP83867_DEVADDR		0x1f

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
	/* set GPIO to out low */
	phy_write_mmd_indirect(phy, 0x0171, DP83867_DEVADDR, TSEC1_PHY_ADDR,
			       0x8888);
	phy_write_mmd_indirect(phy, 0x0172, DP83867_DEVADDR, TSEC1_PHY_ADDR,
			       0x0888);
	/* LED configuration */
	phy_write(phy, TSEC1_PHY_ADDR, 0x18, 0x6b90);
	phy_write(phy, TSEC1_PHY_ADDR, 0x19, 0x0000);
#endif
#ifdef CONFIG_TSEC2
	phy = mdio_phydev_for_ethname(CONFIG_TSEC2_NAME);
	/* set GPIO to out low */
	phy_write_mmd_indirect(phy, 0x0171, DP83867_DEVADDR, TSEC2_PHY_ADDR,
			       0x8888);
	phy_write_mmd_indirect(phy, 0x0172, DP83867_DEVADDR, TSEC2_PHY_ADDR,
			       0x0888);
	/* LED configuration */
	phy_write(phy, TSEC2_PHY_ADDR, 0x18, 0x6b90);
	phy_write(phy, TSEC2_PHY_ADDR, 0x19, 0x0000);
#endif
#ifdef CONFIG_TSEC3
	phy = mdio_phydev_for_ethname(CONFIG_TSEC3_NAME);
	/* set GPIO to out low */
	phy_write_mmd_indirect(phy, 0x0171, DP83867_DEVADDR, TSEC3_PHY_ADDR,
			       0x8888);
	phy_write_mmd_indirect(phy, 0x0172, DP83867_DEVADDR, TSEC3_PHY_ADDR,
			       0x0888);
	/* enable clock out */
	regval = phy_read_mmd_indirect(phy, 0x0170, DP83867_DEVADDR,
				       TSEC3_PHY_ADDR);
	phy_write_mmd_indirect(phy, 0x0170, DP83867_DEVADDR, TSEC3_PHY_ADDR,
			       regval | 0x040);
	/* LED configuration */
	phy_write(phy, TSEC3_PHY_ADDR, 0x18, 0x6b90);
	phy_write(phy, TSEC3_PHY_ADDR, 0x19, 0x0000);
#endif

	return pci_eth_init(bis);
}
#endif

int board_early_init_f(void)
{
	struct ccsr_scfg *scfg = (struct ccsr_scfg *)CONFIG_SYS_FSL_SCFG_ADDR;

#ifdef CONFIG_TSEC_ENET
	/* clear BD & FR bits for BE BD's and frame data */
	clrbits_be32(&scfg->etsecdmamcr, SCFG_ETSECDMAMCR_LE_BD_FR);
	out_be32(&scfg->etsecmcr, SCFG_ETSECCMCR_GE2_CLK125);
#endif

#ifdef CONFIG_FSL_IFC
	init_early_memctl_regs();
#endif

	arch_soc_init();

#if defined(CONFIG_DEEP_SLEEP)
	if (is_warm_boot()) {
		timer_init();
		dram_init();
	}
#endif

	return 0;
}

#ifdef CONFIG_SPL_BUILD
void board_init_f(ulong dummy)
{
	/* Clear the BSS */
	memset(__bss_start, 0, __bss_end - __bss_start);

	get_clocks();

#if defined(CONFIG_DEEP_SLEEP)
	if (is_warm_boot())
		fsl_dp_disable_console();
#endif

	preloader_console_init();

	dram_init();

	/* Allow OCRAM access permission as R/W */
#ifdef CONFIG_LAYERSCAPE_NS_ACCESS
	enable_layerscape_ns_access();
	enable_layerscape_ns_access();
#endif

	board_init_r(NULL, 0);
}
#endif

int board_init(void)
{
#ifndef CONFIG_SYS_FSL_NO_SERDES
	fsl_serdes_init();
#endif

	ls102xa_smmu_stream_id_init();

#ifdef CONFIG_LAYERSCAPE_NS_ACCESS
	enable_layerscape_ns_access();
#endif

#ifdef CONFIG_U_QE
	u_qe_init();
#endif

	return 0;
}

#if defined(CONFIG_MISC_INIT_R)
int misc_init_r(void)
{
#ifdef CONFIG_FSL_CAAM
	return sec_init();
#endif
}
#endif

#if defined(CONFIG_DEEP_SLEEP)
void board_sleep_prepare(void)
{
#ifdef CONFIG_LAYERSCAPE_NS_ACCESS
	enable_layerscape_ns_access();
#endif
}
#endif

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	int ret;
	struct tqmls102xa_eeprom_data eedat;
	/* must hold largest field of eeprom data */
	char safe_string[0x41];

	ret = tqmls102xa_read_eeprom(CONFIG_SYS_EEPROM_BUS_NUM,
				     CONFIG_SYS_I2C_EEPROM_ADDR, &eedat);
	if (!ret) {
		/* ID */
		tqmls102xa_parse_eeprom_id(&eedat, safe_string,
					   ARRAY_SIZE(safe_string));
		if (0 == strncmp(safe_string, "TQM", 3))
			setenv("boardtype", safe_string);
		if (0 == tqmls102xa_parse_eeprom_serial(&eedat, safe_string,
							ARRAY_SIZE(safe_string)))
			setenv("serial#", safe_string);
		else
			setenv("serial#", "???");
		tqmls102xa_show_eeprom(&eedat, "TQM");
	} else {
		printf("EEPROM: err %d\n", ret);
	}

	ls1021a_sata_init();

	return 0;
}
#endif

int ft_board_setup(void *blob, bd_t *bd)
{
	int off;

	ft_cpu_setup(blob, bd);

#ifdef CONFIG_PCI
	ft_pci_setup(blob, bd);
#endif

	off = fdt_node_offset_by_compat_reg(blob, FSL_IFC_COMPAT,
					    CONFIG_SYS_IFC_ADDR);
	fdt_set_node_status(blob, off, FDT_STATUS_DISABLED, 0);

	off = fdt_node_offset_by_compat_reg(blob, FSL_QSPI_COMPAT,
					    QSPI0_BASE_ADDR);
	fdt_set_node_status(blob, off, FDT_STATUS_OKAY, 0);

	return 0;
}
