// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2023 TQ-Systems GmbH
 * Copyright (c) 2023 TQ-Systems GmbH <license@tq-group.com>, D-82229 Seefeld, Germany.
 * Author: Timo Herbrecher, Gregor Herburger
 *
 */

#include <common.h>
#include <env.h>
#include <i2c.h>
#include <net.h>
#include <fdt_support.h>
#include <init.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/soc.h>
#include <hwconfig.h>
#include <mmc.h>
#include <fm_eth.h>
#include <fsl_csu.h>
#include <fsl_esdhc.h>
#include <fsl_sec.h>
#include <fsl-mc/fsl_mc.h>
#include "../common/tq_bb.h"
#include "../common/tq_eeprom.h"
#include "../common/tq_tqmls10xxa_common.h"

#define TQMLS1088A_SYSC_BUS_NUM     0
#define TQMLS1088A_SYSC_ADDR        0x11

#define SYSC_REG_SYSC_FW_VERS       0x02
#define SYSC_REG_BOOT_SRC           0x03
#define SYSC_REG_BOOT_SRC_SDSEL_MSK 0x80
#define SYSC_REG_CPLD_FW_VERS       0xE1

static const char *bname = "TQMLS1088A";

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	fsl_lsch3_early_init_f();

	return tq_bb_board_early_init_f();
}

#if defined(CONFIG_VID)
int init_func_vid(void)
{
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	struct udevice *dev;
	u8 sysc_vid;
	u8 gur_vid;
	u32 fusesr;

	/* get the voltage ID from fuse status register */
	fusesr = in_le32(&gur->dcfg_fusesr);
	gur_vid = (fusesr >> FSL_CHASSIS3_DCFG_FUSESR_ALTVID_SHIFT) &
		FSL_CHASSIS3_DCFG_FUSESR_ALTVID_MASK;
	if (gur_vid == 0 || gur_vid == FSL_CHASSIS3_DCFG_FUSESR_ALTVID_MASK) {
		gur_vid = (fusesr >> FSL_CHASSIS3_DCFG_FUSESR_VID_SHIFT) &
			FSL_CHASSIS3_DCFG_FUSESR_VID_MASK;
	}

	i2c_get_chip_for_busnum(TQMLS1088A_SYSC_BUS_NUM, TQMLS1088A_SYSC_ADDR, 1, &dev);

	/* get core voltage setting from SysC */
	sysc_vid = dm_i2c_reg_read(dev, SYSC_REG_VID);

	/* check if core voltage adjustment has to be done */
	if (gur_vid != sysc_vid) {
		/* core volatage has to be adjusted by SysC */
		printf("Core voltage adjust required (req = 0x%02x, set = 0x%02x)\n",
		       gur_vid, sysc_vid);
		printf("New core voltage is applied after reset\n");
		dm_i2c_reg_write(TQMLS1088A_SYSC_ADDR, SYSC_REG_VID, gur_vid);
	}

	return 0;
}
#endif

int checkboard(void)
{
	printf("Board: %s on a %s ", bname, tq_bb_get_boardname());

	tq_tqmls10xx_checkboard();

	return tq_bb_checkboard();
}

int board_init(void)
{
#ifdef CONFIG_FSL_CAAM
	sec_init();
#endif
	pci_init();

	return tq_bb_board_init();
}

void board_quiesce_devices(void)
{
	fsl_mc_ldpaa_exit(gd->bd);
}

int fsl_board_late_init(void)
{
	struct tq_eeprom_data eeprom;
	int ret;

	ret = tq_read_module_eeprom(&eeprom);

	if (!ret) {
		tq_board_handle_eeprom_data(bname, &eeprom);
		if (is_valid_ethaddr(eeprom.mac))
			tq_tqmls10xxa_set_macaddrs(eeprom.mac, 10);
		else
			puts("EEPROM: error: invalid mac address.\n");
	} else {
		printf("EEPROM: read error: %d\n", ret);
	}

	if (CONFIG_IS_ENABLED(ENV_VARS_UBOOT_RUNTIME_CONFIG))
		env_set("board_name", bname);

	return 0;
}
