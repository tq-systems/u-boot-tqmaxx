// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 TQ-Systems GmbH
 */

#include <common.h>
#include <i2c.h>
#include <fdt_support.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/ppa.h>
#include <asm/arch/soc.h>
#include <hwconfig.h>
#include <ahci.h>
#include <mmc.h>
#include <scsi.h>
#include <fm_eth.h>
#include <fsl_csu.h>
#include <fsl_esdhc.h>
#include <fsl_sec.h>
#include <fsl-mc/fsl_mc.h>
#include <fsl-mc/ldpaa_wriop.h>
#include "../common/tqc_bb.h"
#include "../common/tqc_eeprom.h"


#define TQMLS1088A_SYSC_BUS_NUM     0
#define TQMLS1088A_SYSC_ADDR        0x11

#define SYSC_REG_SYSC_FW_VERS       0x02
#define SYSC_REG_BOOT_SRC           0x03
#define SYSC_REG_BOOT_SRC_SDSEL_MSK 0x80
#define SYSC_REG_VID                0x10
#define SYSC_REG_CPLD_FW_VERS       0xE1

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	fsl_lsch3_early_init_f();

	return tqc_bb_board_early_init_f();
}

#if defined(CONFIG_VID) && (!defined(CONFIG_SPL) || defined(CONFIG_SPL_BUILD))
int init_func_vid(void)
{
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 fusesr;
	unsigned int oldbus;
	uint8_t gur_vid;
	uint8_t sysc_vid;
	
	/* get the voltage ID from fuse status register */
	fusesr = in_le32(&gur->dcfg_fusesr);
	gur_vid = (fusesr >> FSL_CHASSIS3_DCFG_FUSESR_ALTVID_SHIFT) &
		FSL_CHASSIS3_DCFG_FUSESR_ALTVID_MASK;
	if ((gur_vid == 0) || (gur_vid == FSL_CHASSIS3_DCFG_FUSESR_ALTVID_MASK)) {
		gur_vid = (fusesr >> FSL_CHASSIS3_DCFG_FUSESR_VID_SHIFT) &
			FSL_CHASSIS3_DCFG_FUSESR_VID_MASK;
	}

	/* select I2C bus of SysC */
	oldbus = i2c_get_bus_num();
	i2c_set_bus_num(TQMLS1088A_SYSC_BUS_NUM);

	/* get core voltage setting from SysC */
	sysc_vid = i2c_reg_read(TQMLS1088A_SYSC_ADDR, SYSC_REG_VID);

	/* check if core voltage adjustment has to be done */
	if(gur_vid != sysc_vid) {
		/* core volatage has to be adjusted by SysC */
		printf("Core voltage adjust required (req = 0x%02x, set = 0x%02x)\n",
			gur_vid, sysc_vid);
		printf("New core voltage is applied after reset\n");
		i2c_reg_write(TQMLS1088A_SYSC_ADDR, SYSC_REG_VID, gur_vid);
	}

	/* switch back to previous selected I2C bus */
	i2c_set_bus_num(oldbus);

    return 0;
}
#endif

#ifndef CONFIG_SPL_BUILD
int checkboard(void)
{
	unsigned int oldbus;
	uint8_t bootsrc, syscrev, cpldrev;

	/* get further information from SysC */
	oldbus = i2c_get_bus_num();
	i2c_set_bus_num(TQMLS1088A_SYSC_BUS_NUM);
	bootsrc = i2c_reg_read(TQMLS1088A_SYSC_ADDR, SYSC_REG_BOOT_SRC);
	syscrev = i2c_reg_read(TQMLS1088A_SYSC_ADDR, SYSC_REG_SYSC_FW_VERS);
	cpldrev = i2c_reg_read(TQMLS1088A_SYSC_ADDR, SYSC_REG_CPLD_FW_VERS);
	i2c_set_bus_num(oldbus);

	/* print SoM and baseboard name */
	printf("Board: TQMLS1088A on a %s ", tqc_bb_get_boardname());
	switch(bootsrc & 0x0F) {
		case 0x0:
			printf("(Boot from QSPI)\n");
			break;
		case 0x2:
			printf("(Boot from SD)\n");
			break;
		case 0x3:
			printf("(Boot from eMMC)\n");
			break;
		case 0xe:
			printf("(Boot from Hard Coded RCW)\n");
			break;
		default:
			printf("(Bootsource unknown)\n");
			break;
	}
	printf("         SysC FW Rev: %2d.%02d\n",
		(syscrev >> 4) & 0xF, syscrev & 0xF);
	printf("         CPLD FW Rev: %2d.%02d\n", 
		(cpldrev >> 4) & 0xF, cpldrev & 0xF);
 
	return tqc_bb_checkboard();
}

int board_init(void)
{
#ifdef CONFIG_FSL_CAAM
	sec_init();
#endif

#ifdef CONFIG_FSL_LS_PPA
	ppa_init();
#endif

	return tqc_bb_board_init();
}

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	struct tqc_eeprom_data eedat;
	char safe_string[0x41]; /* must hold largest field of eeprom data */
	int ret;

	ret = tqc_read_eeprom(CONFIG_SYS_EEPROM_BUS_NUM,
			CONFIG_SYS_I2C_EEPROM_ADDR, &eedat);

	if(!ret) {
		/* ID */
		tqc_parse_eeprom_id(&eedat, safe_string, ARRAY_SIZE(safe_string));
		if (0 == strncmp(safe_string, "TQMLS1088A", strlen("TQMLS1088A")))
			env_set("boardtype", safe_string);
		if (0 == tqc_parse_eeprom_serial(&eedat, safe_string,
					ARRAY_SIZE(safe_string)))
			env_set("serial#", safe_string);
		else
			env_set("serial#", "???");
		tqc_show_eeprom(&eedat, "TQMLS1088A");
	} else {
		printf("EEPROM: err %d\n", ret);
	}

	return tqc_bb_misc_init_r();
}
#endif
#endif /* !CONFIG_SPL_BUILD */

int board_mmc_getcd(struct mmc *mmc)
{
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 rcw_iic2_base;
	unsigned int oldbus;
	uint8_t bootsrc;
	int ret;

	/* get sdhc mux information from SysC */
	oldbus = i2c_get_bus_num();
	i2c_set_bus_num(TQMLS1088A_SYSC_BUS_NUM);
	bootsrc = i2c_reg_read(TQMLS1088A_SYSC_ADDR, SYSC_REG_BOOT_SRC);
	i2c_set_bus_num(oldbus);

	/* check if eMMC or sd-card selected */
	if(!(bootsrc & SYSC_REG_BOOT_SRC_SDSEL_MSK)) {
		/* card alway present when eMMC selected */
		ret = 1;
	} else {
		/* read IIC2_BASE configuration from RCW */
		rcw_iic2_base = (in_le32(&gur->rcwsr[12]) & 0x0000000C) >> 2;

		/* check if hardware card detection should be used (RCW) */
		if(rcw_iic2_base == 0x2) {
			/* CD mapped to hardware function */
			ret = -1;
		} else {
			/* sd-card selected without hardware card detection pin,
			 * check baseboard specific function
			 */
			ret = tqc_bb_board_mmc_getcd(mmc);
		}
	}

	return ret;
}

int board_mmc_getwp(struct mmc *mmc)
{
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 rcw_iic2_base;
	unsigned int oldbus;
	uint8_t bootsrc;
	int ret;

	/* get sdhc mux information from SysC */
	oldbus = i2c_get_bus_num();
	i2c_set_bus_num(TQMLS1088A_SYSC_BUS_NUM);
	bootsrc = i2c_reg_read(TQMLS1088A_SYSC_ADDR, SYSC_REG_BOOT_SRC);
	i2c_set_bus_num(oldbus);

	/* check if eMMC or sd-card selected */
	if(!(bootsrc & SYSC_REG_BOOT_SRC_SDSEL_MSK)) {
		/* card always writeable when eMMC selected */
		ret = 0;
	} else {
		/* read IIC2_BASE configuration from RCW */
		rcw_iic2_base = (in_le32(&gur->rcwsr[12]) & 0x0000000C) >> 2;

		/* check if hardware write protect detection should be used (RCW) */
		if(rcw_iic2_base == 0x2) {
			/* WP mapped to hardware function */
			ret = -1;
		} else {
			/* sd-card selected without hardware write protection pin,
			 * check baseboard specific function
			 */
			ret = tqc_bb_board_mmc_getwp(mmc);
		}
	}

	return ret;
}

#ifndef CONFIG_SPL_BUILD
#ifdef CONFIG_FSL_MC_ENET
void fdt_fixup_board_enet(void *fdt)
{
    int offset;

	offset = fdt_path_offset(fdt, "/soc/fsl-mc");
	if (offset < 0)
		offset = fdt_path_offset(fdt, "/fsl-mc");
    if (offset < 0)
        offset = fdt_path_offset(fdt, "/fsl,dprc@0");
    if (offset < 0) {
        printf("%s: ERROR: fsl-mc node not found in device tree (error %d)\n",
               __func__, offset);
        return;
    }

    if ((get_mc_boot_status() == 0) && (get_dpl_apply_status() == 0))
        fdt_status_okay(fdt, offset);
    else
        fdt_status_fail(fdt, offset);
}
#endif

int ft_board_setup(void *blob, bd_t *bd)
{
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	int offset;
	unsigned int oldbus;
	uint8_t bootsrc;
	int err;
	int i;
	u64 base[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];
	u32 rcw_iic2_base;

	ft_cpu_setup(blob, bd);

	/* fixup DT for the two GPP DDR banks */
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		base[i] = gd->bd->bi_dram[i].start;
		size[i] = gd->bd->bi_dram[i].size;
	}

#ifdef CONFIG_RESV_RAM
	/* reduce size if reserved memory is within this bank */
	if (gd->arch.resv_ram >= base[0] &&
	    gd->arch.resv_ram < base[0] + size[0])
		size[0] = gd->arch.resv_ram - base[0];
#if (CONFIG_NR_DRAM_BANKS > 1)
	else if (gd->arch.resv_ram >= base[1] &&
		 gd->arch.resv_ram < base[1] + size[1])
		size[1] = gd->arch.resv_ram - base[1];
#endif
#endif

	fdt_fixup_memory_banks(blob, base, size, CONFIG_NR_DRAM_BANKS);

#ifdef CONFIG_FSL_MC_ENET
	fdt_fsl_mc_fixup_iommu_map_entry(blob);
#endif

#ifdef CONFIG_FSL_MC_ENET 
	fdt_fixup_board_enet(blob);
	err = fsl_mc_ldpaa_exit(bd);
	if (err)
		return err;
#endif

	/* get sdhc mux information from SysC */
	oldbus = i2c_get_bus_num();
	i2c_set_bus_num(TQMLS1088A_SYSC_BUS_NUM);
	bootsrc = i2c_reg_read(TQMLS1088A_SYSC_ADDR, SYSC_REG_BOOT_SRC);
	i2c_set_bus_num(oldbus);

	/* get offset of sdhc node */
	offset = fdt_path_offset(blob, "/soc/esdhc@2140000");
	if (offset < 0)
		return offset;

	/* delete eMMC specific properties if sd-card selected */
	if(bootsrc & SYSC_REG_BOOT_SRC_SDSEL_MSK) {
		/* SDHC_EXT_SEL = 1 => sd-card */
		fdt_delprop(blob, offset, "non-removable");
		fdt_delprop(blob, offset, "disable-wp");
		fdt_delprop(blob, offset, "mmc-hs200-1_8v");
		fdt_setprop_empty(blob, offset, "no-1-8-v");
	}

	/* read IIC2_BASE configuration from RCW */
	rcw_iic2_base = (in_le32(&gur->rcwsr[12]) & 0x0000000C) >> 2;

	/* delete cd and wp gpio handle if hardware card detection should be used
	 * (RCW) or eMMC is selected
	 */
	if(!(bootsrc & SYSC_REG_BOOT_SRC_SDSEL_MSK) || (rcw_iic2_base == 0x2)) {
		fdt_delprop(blob, offset, "cd-gpios");
		fdt_delprop(blob, offset, "wp-gpios");
	}

	return tqc_bb_ft_board_setup(blob, bd);
}
#endif
