// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Marco Felsch, Matthias Schiffer
 *
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/mach-imx/boot_mode.h>
#include <fdt_support.h>
#include <mmc.h>
#include <net.h>
#include <spi_flash.h>

#include "../common/tq_eeprom.h"
#include "../common/tq_emmc.h"
#include "../common/tq_spi_nor.h"
#include "tqma6ul_common.h"

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

#if defined(CONFIG_DM_MMC) && defined(CONFIG_BLK)
static const u16 tqma6ul_emmc_dsr = 0x0100;

/* board-specific MMC card detection / modification */
void board_mmc_detect_card_type(struct mmc *mmc)
{
	struct mmc *emmc = find_mmc_device(0);

	if (mmc != emmc)
		return;

	if (tq_emmc_need_dsr(mmc) > 0)
		mmc_set_dsr(mmc, tqma6ul_emmc_dsr);
}
#endif

int tqma6ul_common_early_init_f(void)
{
#ifdef CONFIG_FSL_QSPI
	enable_qspi_clk(0);
#endif

	return 0;
}

const char *tqma6ul_common_get_boardname(void)
{
	if (is_mx6ul()) {
		if (IS_ENABLED(CONFIG_TQMA6UL_VARIANT_STANDARD))
			return "TQMa6ULx REV.030x";
		if (IS_ENABLED(CONFIG_TQMA6UL_VARIANT_LGA))
			return "TQMa6ULxL REV.020x";
	}

	if (is_mx6ull()) {
		if (IS_ENABLED(CONFIG_TQMA6UL_VARIANT_STANDARD))
			return "TQMa6ULLx REV.030x";
		if (IS_ENABLED(CONFIG_TQMA6UL_VARIANT_LGA))
			return "TQMa6ULLxL REV.020x";
	}

	return "Unknown";
}

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode tqma6ul_board_boot_modes[] = {
	/* 4 bit bus width */
	{"sd", MAKE_CFGVAL(0x42, 0x20, 0x00, 0x00)},
	{"emmc", MAKE_CFGVAL(0x40, 0x28, 0x00, 0x00)},
	{"qspi", MAKE_CFGVAL(0x10, 0x00, 0x00, 0x00)},
	{NULL, 0},
};
#endif

int tqma6ul_common_late_init(void)
{
	struct tq_eeprom_data eeprom;
	int ret;

#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(tqma6ul_board_boot_modes);
#endif

	env_set_runtime("board_name", tqma6ul_common_get_boardname());

	ret = tq_read_module_eeprom(&eeprom);
	if (!ret)
		tq_board_handle_eeprom_data("TQMa6UL", &eeprom);
	else
		printf("EEPROM: err %d\n", ret);

	return 0;
}

int tqma6ul_common_checkboard(void)
{
	if (is_mx6ul()) {
		if (!IS_ENABLED(CONFIG_MX6UL))
			printf("*** ERROR: image not compiled for i.MX6UL!\n");
	} else if (is_mx6ull()) {
		if (!IS_ENABLED(CONFIG_MX6ULL))
			printf("*** ERROR: image not compiled for i.MX6ULL!\n");
	} else {
		printf("*** ERROR: unknown CPU variant!\n");
	}

	return 0;
}

/*
 * Device Tree Support
 */
#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)

static void tqma6ul_ft_qspi_setup(void *blob)
{
	const char *path = "/soc/bus@2100000/spi@21e0000";
	const struct node_info nodes[] = {
		{ "jedec,spi-nor", MTD_DEV_TYPE_NOR },
	};

	tq_ft_spi_setup(blob, path, nodes, ARRAY_SIZE(nodes));
}

static void tqma6ul_ft_emmc_setup(void *blob)
{
#if defined(CONFIG_DM_MMC) && defined(CONFIG_BLK)
	struct mmc *mmc = find_mmc_device(0);

	if (!mmc || mmc_init(mmc)) {
		puts("eMMC: not present\n");
		return;
	}

	if (tq_emmc_need_dsr(mmc) > 0)
		tq_ft_fixup_emmc_dsr(blob, "mmc0", tqma6ul_emmc_dsr);
#endif
}

int tqma6ul_common_ft_board_setup(void *blob, struct bd_info *bd)
{
	fdt_fixup_memory(blob, PHYS_SDRAM, gd->ram_size);

	tqma6ul_ft_qspi_setup(blob);
	tqma6ul_ft_emmc_setup(blob);

	return 0;
}
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */
