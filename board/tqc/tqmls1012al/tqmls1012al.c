// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2022 TQ-Systems GmbH
 * Max Merchel <Max.Merchel@tq-group.com>
 */

#include <common.h>

#include "tqmls1012al_bb.h"
#include "../common/tq_bb.h"
#include "../common/tq_eeprom.h"
#include "../common/tq_env.h"

#include <command.h>
#include <env_internal.h>
#include <fdt_support.h>
#include <fsl_esdhc.h>
#include <fsl_mmdc.h>
#include <hang.h>
#include <hwconfig.h>
#include <i2c.h>
#include <init.h>
#include <jffs2/load_kernel.h>
#include <mmc.h>
#include <net.h>
#include <netdev.h>
#include <scsi.h>

#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/mmu.h>
#include <asm/arch/soc.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	gd->ram_size = tfa_get_dram_size();

	return 0;
}

int board_early_init_f(void)
{
	fsl_lsch2_early_init_f();

	return 0;
}

int board_init(void)
{
	struct ccsr_cci400 *cci = (struct ccsr_cci400 *)(CONFIG_SYS_IMMR +
					CONFIG_SYS_CCI400_OFFSET);
	/*
	 * Set CCI-400 control override register to enable barrier
	 * transaction
	 */
	if (current_el() == 3)
		out_le32(&cci->ctrl_ord, CCI400_CTRLORD_EN_BARRIER);

	if (IS_ENABLED(CONFIG_SYS_FSL_ERRATUM_A010315))
		erratum_a010315();

	return 0;
}

#if (IS_ENABLED(CONFIG_MISC_INIT_R))

int misc_init_r(void)
{
	struct tq_eeprom_data eedat_modul;
	/* must hold largest field of eeprom data */
	char safe_string[0x41];
	int ret = -1;

	ret = tq_read_eeprom(CONFIG_SYS_I2C_EEPROM_BUS,
			     TQMLS1012AL_I2C_EEPROM1_ADDR, &eedat_modul);

	if (ret) {
		printf("EEPROM: (0x%x) err %d\n", TQMLS1012AL_I2C_EEPROM1_ADDR,
		       ret);
	} else {
		/* Modul ID */
		tq_parse_eeprom_id(&eedat_modul, safe_string,
				   ARRAY_SIZE(safe_string));
		if (strncmp(safe_string, "TQMLS1012AL", 11) == 0)
			tq_env_set("modultype", safe_string);

		/* Modul Serial# */
		if (tq_parse_eeprom_serial(&eedat_modul, safe_string,
					   ARRAY_SIZE(safe_string)) == 0)
			tq_env_set("serial#", safe_string);
		else
			tq_env_set("serial#", "???");

		/* Modul MAC */
		tq_set_ethaddr(&eedat_modul, "ethaddr", 0);
		tq_set_ethaddr(&eedat_modul, "eth1addr", 1);

		tq_show_eeprom(&eedat_modul, "TQMLS1012AL");
	}

	tqmls1012al_bb_misc_init_r();

	return 0;
}
#endif

int ft_board_setup(void *blob, struct bd_info *bd)
{
	const char * const path = "/quadspi@1550000";
	static const struct node_info nodes[] = {
		{ "jedec,spi-nor",	MTD_DEV_TYPE_NOR, },
	};

	arch_fixup_fdt(blob);

	ft_cpu_setup(blob, bd);

	tq_ft_spi_setup(blob, path, nodes, ARRAY_SIZE(nodes));

	return 0;
}
