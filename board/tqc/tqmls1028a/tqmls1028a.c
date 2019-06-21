/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <fsl_ddr.h>
#include <asm/io.h>
#include <hwconfig.h>
#include <fdt_support.h>
#include <linux/libfdt.h>
#include <environment.h>
#include <asm/arch-fsl-layerscape/soc.h>
#include <i2c.h>
#include <asm/arch/soc.h>
#ifdef CONFIG_FSL_LS_PPA
#include <asm/arch/ppa.h>
#endif
#include <fsl_immap.h>
#include <netdev.h>
#include <video_fb.h>

#include <fdtdec.h>
#include <miiphy.h>
#include "../common/tqmaxx_eeprom.h"
#include "tqmls1028a_bb.h"

DECLARE_GLOBAL_DATA_PTR;

void serdes_pll_reset(void)
{
	#define RSTCTL_RSTREQ		0x80000000
	#define RSTCTL_RST_DONE		0x40000000
	#define RSTCTL_RSTERR		0x20000000

	u32 reg;

	/* if error in PLL initiate reset */
	reg = in_le32(0x1EA0000);
	if (reg & RSTCTL_RSTERR)
		setbits_le32(0x1EA0000, RSTCTL_RSTREQ);

	reg = in_le32(0x1EA0020);
	if (reg & RSTCTL_RSTERR)
		setbits_le32(0x1EA0020, RSTCTL_RSTREQ);
}
int board_init(void)
{
#ifdef CONFIG_ENV_IS_NOWHERE
	gd->env_addr = (ulong)&default_environment[0];
#endif

#ifdef CONFIG_FSL_LS_PPA
	ppa_init();
#endif

	board_bb_init();

	serdes_pll_reset();

#ifndef CONFIG_SYS_EARLY_PCI_INIT
	/* run PCI init to kick off ENETC */
	pci_init();
#endif

	return 0;
}

int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}

int board_early_init_f(void)
{
#ifdef CONFIG_SYS_I2C_EARLY_INIT
	i2c_early_init_f();
#endif
	fsl_lsch3_early_init_f();
	return 0;
}

void detail_board_ddr_info(void)
{
	puts("\nDDR    ");
	print_size(gd->bd->bi_dram[0].size + gd->bd->bi_dram[1].size, "");
	print_ddr_info(0);
}

#ifdef CONFIG_OF_BOARD_SETUP
#ifdef CONFIG_FSL_ENETC
extern void enetc_setup(void *blob);
#endif
int ft_board_setup(void *blob, bd_t *bd)
{
	u64 base[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];

	ft_cpu_setup(blob, bd);

	/* fixup DT for the two GPP DDR banks */
	base[0] = gd->bd->bi_dram[0].start;
	size[0] = gd->bd->bi_dram[0].size;

#ifdef CONFIG_RESV_RAM
	/* reduce size if reserved memory is within this bank */
	if (gd->arch.resv_ram >= base[0] &&
	    gd->arch.resv_ram < base[0] + size[0])
		size[0] = gd->arch.resv_ram - base[0];
	else if (gd->arch.resv_ram >= base[1] &&
		 gd->arch.resv_ram < base[1] + size[1])
		size[1] = gd->arch.resv_ram - base[1];
#endif
	fdt_fixup_memory_banks(blob, base, size, 2);

#ifdef CONFIG_FSL_ENETC
	enetc_setup(blob);
#endif
	return 0;
}
#endif

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	int ret = -1;
	struct tqmaxx_eeprom_data eepromdata;
	char safe_string[0x41];
	char ethaddrstring[9];

	ret = tqmaxx_read_eeprom(0, I2C_EEPROM_ADDR, &eepromdata);

	if (ret) {
		printf("Error reading eeprom.\n");
		return ret;
	}

	ret = tqmaxx_parse_eeprom_mac(&eepromdata, safe_string,
				      ARRAY_SIZE(safe_string));
	if (!ret) {
		env_set("ethaddr", safe_string);
		eth_env_set_enetaddr("ethaddr", (uchar *)safe_string);

		for (size_t i = 1; i <= 4; i++) {
			ret = tqmaxx_parse_eeprom_mac_additional(&eepromdata,
				safe_string, ARRAY_SIZE(safe_string), i);
			if (!ret) {
				snprintf(ethaddrstring, 9, "eth%luaddr", i);
				env_set(ethaddrstring, safe_string);
				eth_env_set_enetaddr(ethaddrstring,
						(uchar *)safe_string);
			}
		}
		tqmaxx_show_eeprom(&eepromdata, "TQMLS1028A");
	}
	return 0;
}
#endif

#ifdef CONFIG_LAST_STAGE_INIT
int last_stage_init(void)
{
	u8 val;

	tqmls1028a_bb_late_init();

	/* Set Bit 0 of Register 0 of RTC to adjust to 12.5 pF */
	val = i2c_reg_read(CONFIG_SYS_I2C_RTC_ADDR, 0x00);

	if (!(val & 0x01))
		i2c_reg_write(CONFIG_SYS_I2C_RTC_ADDR, 0x00, val | 0x01);


	return 0;
}
#endif

void *video_hw_init(void)
{
	return NULL;
}

#ifdef CONFIG_EMMC_BOOT
void *esdhc_get_base_addr(void)
{
	return (void *)CONFIG_SYS_FSL_ESDHC1_ADDR;
}

#endif
