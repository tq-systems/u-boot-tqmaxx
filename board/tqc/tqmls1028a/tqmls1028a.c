// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2019 NXP
 * Copyright 2019-2020 TQ-Systems GmbH
 *
 * Author: Matthias Schiffer <matthias.schiffer@tq-group.com>
 */

#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <fsl_ddr.h>
#include <asm/io.h>
#include <hwconfig.h>
#include <fdt_support.h>
#include <linux/libfdt.h>
#include <asm/arch-fsl-layerscape/clock.h>
#include <asm/arch-fsl-layerscape/soc.h>
#include <i2c.h>
#include <asm/arch/soc.h>
#include <fsl_immap.h>
#include <fsl_sec.h>
#include <netdev.h>
#include <fdtdec.h>
#include "../common/tqmaxx_eeprom.h"
#include "tqmls1028a_bb.h"

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
#ifdef CONFIG_FSL_CAAM
	sec_init();
#endif

	board_bb_init();

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

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, bd_t *bd)
{
	u64 base[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];

	ft_cpu_setup(blob, bd);

	/* fixup DT for the two GPP DDR banks */
	base[0] = gd->bd->bi_dram[0].start;
	size[0] = gd->bd->bi_dram[0].size;

	fdt_fixup_memory_banks(blob, base, size, 2);

	return 0;
}
#endif

#ifdef CONFIG_MISC_INIT_R

static const char *tqmls1028a_variant(void)
{
	u32 svr = get_svr();

	switch (SVR_SOC_VER(svr)) {
	case SVR_LS1017A: return "TQMLS1017A";
	case SVR_LS1018A: return "TQMLS1018A";
	case SVR_LS1027A: return "TQMLS1027A";
	case SVR_LS1028A: return "TQMLS1028A";
	}

	return "unknown module variant";
}

int misc_init_r(void)
{
	int ret = -1;
	struct tqmaxx_eeprom_data eepromdata;
	char safe_string[0x41];
	char ethaddrstring[9];

	ret = tqmaxx_read_eeprom(0, CONFIG_SYS_I2C_EEPROM_ADDR, &eepromdata);

	if (ret) {
		printf("Error reading eeprom.\n");
		return ret;
	}

	ret = tqmaxx_parse_eeprom_mac(&eepromdata, safe_string,
				      ARRAY_SIZE(safe_string));
	if (!ret) {
		env_set("ethaddr", safe_string);
		eth_env_set_enetaddr("ethaddr", (uchar *)safe_string);

		int i = 1;
		for (i = 1; i <= 2; i++) {
			ret = tqmaxx_parse_eeprom_mac_additional(&eepromdata,
					safe_string, ARRAY_SIZE(safe_string),
					i, "%02x:%02x:%02x:%02x:%02x:%02x");
			if (!ret) {
				snprintf(ethaddrstring, 9, "eth%daddr", i);
				env_set(ethaddrstring, safe_string);
				eth_env_set_enetaddr(ethaddrstring,
						    (uchar *)safe_string);
			}
		}

		tqmaxx_show_eeprom(&eepromdata, tqmls1028a_variant());
	}
	return 0;
}
#endif

int last_stage_init(void)
{
	struct udevice *dev;
	int ret;
	u8 val;

	tqmls1028a_bb_late_init();

	ret = i2c_get_chip_for_busnum(0, CONFIG_SYS_I2C_RTC_ADDR, 1, &dev);
	if (ret)
		return ret;

	/* Set Bit 0 of Register 0 of RTC to adjust to 12.5 pF */
	val = dm_i2c_reg_read(dev, 0x00);

	if (!(val & 0x01))
		dm_i2c_reg_write(dev, 0x00, val | 0x01);

	return 0;
}
