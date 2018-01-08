/*
 * Copyright (C) 2016 Markus Niebel <Markus.Niebel@tq-group.com>
 *
 * SPL DRAM init for the TQ Systems TQMa6S module.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>

#if !defined(CONFIG_MX6S)
#undef CONFIG_MX6QDL
#undef CONFIG_MX6Q
#undef CONFIG_MX6D
#define CONFIG_MX6S
#endif

#include <asm/io.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/mx6dl-ddr.h>
#include <asm/arch/iomux.h>
#include <asm/arch/crm_regs.h>

#include <common.h>

static inline void init_write_reg(uint32_t address, uint32_t value)
{
	__raw_writel(value, address);
}

static void tqma6s_init_ddr_controller(void)
{
	debug("spl: tqma6s ddr iom ....\n");
	/* TQMa6S DDR config Rev. 0300D */
	/* IOMUX configuration */
	init_write_reg(MX6_IOM_GRP_DDR_TYPE, 0x000C0000);
	init_write_reg(MX6_IOM_GRP_DDRPKE, 0x00000000);
	init_write_reg(MX6_IOM_DRAM_SDCLK_0, 0x00008000);
	init_write_reg(MX6_IOM_DRAM_SDCLK_1, 0x00008030);
	init_write_reg(MX6_IOM_DRAM_CAS, 0x00008030);
	init_write_reg(MX6_IOM_DRAM_RAS, 0x00008030);
	init_write_reg(MX6_IOM_GRP_ADDDS, 0x00000030);
	init_write_reg(MX6_IOM_DRAM_RESET, 0x00000030);
	init_write_reg(MX6_IOM_DRAM_SDCKE0, 0x00003000);
	init_write_reg(MX6_IOM_DRAM_SDCKE1, 0x00000000);
	init_write_reg(MX6_IOM_DRAM_SDBA2, 0x00000000);
	init_write_reg(MX6_IOM_DRAM_SDODT0, 0x00003030);
	init_write_reg(MX6_IOM_DRAM_SDODT1, 0x00003030);
	init_write_reg(MX6_IOM_GRP_CTLDS, 0x00000030);
	init_write_reg(MX6_IOM_DDRMODE_CTL, 0x00020000);
	
	init_write_reg(MX6_IOM_DRAM_SDQS0, 0x00000030);
	init_write_reg(MX6_IOM_DRAM_SDQS1, 0x00000030);
	init_write_reg(MX6_IOM_DRAM_SDQS2, 0x00000030);
	init_write_reg(MX6_IOM_DRAM_SDQS3, 0x00000030);
	init_write_reg(MX6_IOM_DRAM_SDQS4, 0x00000000);
	init_write_reg(MX6_IOM_DRAM_SDQS5, 0x00000000);
	init_write_reg(MX6_IOM_DRAM_SDQS6, 0x00000000);
	init_write_reg(MX6_IOM_DRAM_SDQS7, 0x00000000);
	init_write_reg(MX6_IOM_GRP_DDRMODE, 0x00020000);

	init_write_reg(MX6_IOM_GRP_B0DS, 0x00000030);
	init_write_reg(MX6_IOM_GRP_B1DS, 0x00000030);
	init_write_reg(MX6_IOM_GRP_B2DS, 0x00000030);
	init_write_reg(MX6_IOM_GRP_B3DS, 0x00000030);
	init_write_reg(MX6_IOM_GRP_B4DS, 0x00000000);
	init_write_reg(MX6_IOM_GRP_B5DS, 0x00000000);
	init_write_reg(MX6_IOM_GRP_B6DS, 0x00000000);
	init_write_reg(MX6_IOM_GRP_B7DS, 0x00000000);

	init_write_reg(MX6_IOM_DRAM_DQM0, 0x00000030);
	init_write_reg(MX6_IOM_DRAM_DQM1, 0x00000030);
	init_write_reg(MX6_IOM_DRAM_DQM2, 0x00000030);
	init_write_reg(MX6_IOM_DRAM_DQM3, 0x00000030);
	init_write_reg(MX6_IOM_DRAM_DQM4, 0x00000000);
	init_write_reg(MX6_IOM_DRAM_DQM5, 0x00000000);
	init_write_reg(MX6_IOM_DRAM_DQM6, 0x00000000);
	init_write_reg(MX6_IOM_DRAM_DQM7, 0x00000000);

	debug("spl: tqma6s ddr calibration ....\n");
	/* memory interface calibration values */
	init_write_reg(MX6_MMDC_P0_MPZQHWCTRL, 0xA1390003);
	init_write_reg(MX6_MMDC_P1_MPZQHWCTRL, 0xA1380000);
	init_write_reg(MX6_MMDC_P0_MPWLDECTRL0, 0x004C004A);
	init_write_reg(MX6_MMDC_P0_MPWLDECTRL1, 0x003F0048);
	init_write_reg(MX6_MMDC_P1_MPWLDECTRL0, 0x00000000);
	init_write_reg(MX6_MMDC_P1_MPWLDECTRL1, 0x00000000);
	init_write_reg(MX6_MMDC_P0_MPDGCTRL0, 0x42440240);
	init_write_reg(MX6_MMDC_P0_MPDGCTRL1, 0x022C022C);
	init_write_reg(MX6_MMDC_P1_MPDGCTRL0, 0x00000000);
	init_write_reg(MX6_MMDC_P1_MPDGCTRL1, 0x00000000);
	init_write_reg(MX6_MMDC_P0_MPRDDLCTL, 0x484A504C);
	init_write_reg(MX6_MMDC_P1_MPRDDLCTL, 0x00000000);
	init_write_reg(MX6_MMDC_P0_MPWRDLCTL, 0x34322832);
	init_write_reg(MX6_MMDC_P1_MPWRDLCTL, 0x00000000);
	init_write_reg(MX6_MMDC_P0_MPRDDQBY0DL, 0x33333333);
	init_write_reg(MX6_MMDC_P0_MPRDDQBY1DL, 0x33333333);
	init_write_reg(MX6_MMDC_P0_MPRDDQBY2DL, 0x33333333);
	init_write_reg(MX6_MMDC_P0_MPRDDQBY3DL, 0x33333333);
	init_write_reg(MX6_MMDC_P1_MPRDDQBY0DL, 0x00000000);
	init_write_reg(MX6_MMDC_P1_MPRDDQBY1DL, 0x00000000);
	init_write_reg(MX6_MMDC_P1_MPRDDQBY2DL, 0x00000000);
	init_write_reg(MX6_MMDC_P1_MPRDDQBY3DL, 0x00000000);
	init_write_reg(MX6_MMDC_P0_MPMUR0, 0x00000800);
	init_write_reg(MX6_MMDC_P1_MPMUR0, 0x00000000);

	debug("spl: tqma6s ddr config ....\n");
	/* configure memory interface */
	init_write_reg(MX6_MMDC_P0_MDPDC, 0x0002002D);
	init_write_reg(MX6_MMDC_P0_MDOTC, 0x00333030);
	init_write_reg(MX6_MMDC_P0_MDCFG0, 0x3F435333);
	init_write_reg(MX6_MMDC_P0_MDCFG1, 0xB68E8B63);
	init_write_reg(MX6_MMDC_P0_MDCFG2, 0x01FF00DB);
	init_write_reg(MX6_MMDC_P0_MDMISC, 0x00011740);
	init_write_reg(MX6_MMDC_P0_MDSCR, 0x00008000);

	debug("spl: tqma6s MX6_MMDC_P0_MDSCR %x ....\n", __raw_readl(MX6_MMDC_P0_MDSCR));

/* TODO: leave it in Power Up Default */
	init_write_reg(MX6_MMDC_P0_MDRWD, 0x000026D2);
	init_write_reg(MX6_MMDC_P0_MDOR, 0x00431023);
	init_write_reg(MX6_MMDC_P0_MDASP, 0x00000017);

	debug("spl: tqma6s ddr mdctl - leave reset\n");
	init_write_reg(MX6_MMDC_P0_MDCTL, 0x83190000);
	
/* TODO: wait to CKE ???? */
	init_write_reg(MX6_MMDC_P0_MDSCR, 0x00408032);
	init_write_reg(MX6_MMDC_P0_MDSCR, 0x00008033);
	init_write_reg(MX6_MMDC_P0_MDSCR, 0x00048031);
	init_write_reg(MX6_MMDC_P0_MDSCR, 0x15208030);
	init_write_reg(MX6_MMDC_P0_MDSCR, 0x04008040);
	init_write_reg(MX6_MMDC_P0_MDREF, 0x00007800);
	init_write_reg(MX6_MMDC_P0_MPODTCTRL, 0x00022222);
	init_write_reg(MX6_MMDC_P1_MPODTCTRL, 0x00000000);
	init_write_reg(MX6_MMDC_P0_MDPDC, 0x0002552D);
	init_write_reg(MX6_MMDC_P0_MAPSR, 0x00001006);
	init_write_reg(MX6_MMDC_P0_MDSCR, 0x00000000);

	debug("spl: tqma6s ddr init done ...\n");
}

void tqma6s_init(void)
{
	tqma6s_init_ddr_controller();

}
