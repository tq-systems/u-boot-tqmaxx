// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 TQ-Systems GmbH
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
#include "tqmls1046a_bb.h"
#include "../common/tqc_eeprom.h"

#define SCFG_QSPI_CLKSEL_DIV_24	0x30100000

#define TQMLS1046A_SYSC_BUS_NUM 0
#define TQMLS1046A_SYSC_ADDR    0x11

#define SYSC_REG_SYSC_FW_VERS   0x02
#define SYSC_REG_BOOT_SRC       0x03
#define SYSC_REG_CPLD_FW_VERS   0xE1

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	struct ccsr_scfg *scfg = (struct ccsr_scfg *)CONFIG_SYS_FSL_SCFG_ADDR;

	fsl_lsch2_early_init_f();
#ifdef CONFIG_FSL_QSPI
	/* divide CGA1/CGA2 PLL by 24 to get QSPI interface clock */
	out_be32(&scfg->qspi_cfg, SCFG_QSPI_CLKSEL_DIV_24);
#endif

	return tqmls1046a_bb_board_early_init_f();
}

#ifndef CONFIG_SPL_BUILD
int checkboard(void)
{
	unsigned int oldbus;
	uint8_t bootsrc, syscrev, cpldrev;

	/* get further information from SysC */
	oldbus = i2c_get_bus_num();
	i2c_set_bus_num(TQMLS1046A_SYSC_BUS_NUM);
	bootsrc = i2c_reg_read(TQMLS1046A_SYSC_ADDR, SYSC_REG_BOOT_SRC);
	syscrev = i2c_reg_read(TQMLS1046A_SYSC_ADDR, SYSC_REG_SYSC_FW_VERS);
	cpldrev = i2c_reg_read(TQMLS1046A_SYSC_ADDR, SYSC_REG_CPLD_FW_VERS);
	i2c_set_bus_num(oldbus);

	/* print SoM and baseboard name */
	printf("Board: TQMLS1046A on a %s ", tqmls1046a_bb_get_boardname());
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
 
	return tqmls1046a_bb_checkboard();
}

int board_init(void)
{
#ifdef CONFIG_SECURE_BOOT
	/*
	 * In case of Secure Boot, the IBR configures the SMMU
	 * to allow only Secure transactions.
	 * SMMU must be reset in bypass mode.
	 * Set the ClientPD bit and Clear the USFCFG Bit
	 */
	u32 val;
	val = (in_le32(SMMU_SCR0) | SCR0_CLIENTPD_MASK) & ~(SCR0_USFCFG_MASK);
	out_le32(SMMU_SCR0, val);
	val = (in_le32(SMMU_NSCR0) | SCR0_CLIENTPD_MASK) & ~(SCR0_USFCFG_MASK);
	out_le32(SMMU_NSCR0, val);
#endif

#ifdef CONFIG_FSL_CAAM
	sec_init();
#endif

#ifdef CONFIG_FSL_LS_PPA
	ppa_init();
#endif

	return tqmls1046a_bb_board_init();
}

int board_setup_core_volt(u32 vdd)
{
	/* TODO: core voltage could be changed from SysC on TQMLS10xxA */
	return 0;
}

int get_serdes_volt(void)
{
	/* TODO: serdes voltage is connected to core voltage on TQMLS10xxA */
	return 1000;
}

int set_serdes_volt(int svdd)
{
	/* TODO: serdes voltage is connected to core voltage on TQMLS10xxA */
	return 0;
}

int power_init_board(void)
{
	/* TODO: anything to do here? */
	setup_chip_volt();

	return 0;
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
		if (0 == strncmp(safe_string, "TQMLS1046A", strlen("TQMLS1046A")))
			env_set("boardtype", safe_string);
		if (0 == tqc_parse_eeprom_serial(&eedat, safe_string,
					ARRAY_SIZE(safe_string)))
			env_set("serial#", safe_string);
		else
			env_set("serial#", "???");
		tqc_show_eeprom(&eedat, "TQMLS1046A");
	} else {
		printf("EEPROM: err %d\n", ret);
	}

	return tqmls1046a_bb_misc_init_r();
}
#endif

int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);

#ifdef CONFIG_SYS_DPAA_FMAN
	fdt_fixup_fman_ethernet(blob);
#endif

	return tqmls1046a_bb_ft_board_setup(blob, bd);
}
#endif
