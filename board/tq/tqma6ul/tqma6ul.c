// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016-2024 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Marco Felsch, Matthias Schiffer
 *
 */

#include <common.h>
#include <env.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/mach-imx/boot_mode.h>
#include <fdt_support.h>
#include <mmc.h>
#include <mtd_node.h>
#include <spi_flash.h>

#include "../common/tq_bb.h"
#include "../common/tq_eeprom.h"
#include "../common/tq_emmc.h"
#include "tqma6ul.h"

DECLARE_GLOBAL_DATA_PTR;

const u16 tq_emmc_dsr = 0x100;

/* board-specific MMC card detection / modification */
void board_mmc_detect_card_type(struct mmc *mmc)
{
	struct mmc *emmc = find_mmc_device(0);

	if (mmc != emmc)
		return;

	if (tq_emmc_need_dsr(mmc) > 0)
		mmc_set_dsr(mmc, tq_emmc_dsr);
}

int tq_bb_board_early_init_f(void)
{
	if (IS_ENABLED(CONFIG_FSL_QSPI))
		enable_qspi_clk(0);

	return 0;
}

const char *check_cpu_variant(void)
{
	const char *cpu;

	if (is_mx6ul()) {
		cpu = "ul";
		if (!IS_ENABLED(CONFIG_MX6UL))
			printf("*** ERROR: image not compiled for i.MX6UL!\n");
	} else if (is_mx6ull()) {
		cpu = "ull";
		if (!IS_ENABLED(CONFIG_MX6ULL))
			printf("*** ERROR: image not compiled for i.MX6ULL!\n");
	} else {
		printf("unknown CPU\n");
		return NULL;
	}

	return cpu;
}

enum tqma6ul_som_type check_tqma6ul_variant(void)
{
	if (IS_ENABLED(CONFIG_TQMA6UL_VARIANT_STANDARD))
		return tqma6_som_type_ca;
	else if (IS_ENABLED(CONFIG_TQMA6UL_VARIANT_LGA))
		return tqma6_som_type_lga;

	printf("unknown TQMa6UL variant\n");

	return tqma6_som_type_unknown;
}

enum tqma6ul_som_type tqma6ul_dt(char *dt, size_t dtsize, const char *mb)
{
	const char *tqma6ul_cpu, *tqma6ul_variant;
	enum tqma6ul_som_type somtype;
	u8 mx6ul_variant;

	tqma6ul_cpu = check_cpu_variant();
	if (!tqma6ul_cpu)
		return tqma6_som_type_unknown;

	/* MX6UL1 vs MX6UL2 */
	mx6ul_variant = check_module_fused(MODULE_ENET2) ? 1 : 2;

	somtype = check_tqma6ul_variant();
	switch (somtype) {
	case tqma6_som_type_ca:
		tqma6ul_variant = "";
		break;
	case tqma6_som_type_lga:
		tqma6ul_variant = "l";
		break;
	default:
		return tqma6_som_type_unknown;
	}

	snprintf(dt, dtsize, "imx6%s-tqma6%s%u%s-%s",
		 tqma6ul_cpu, tqma6ul_cpu, mx6ul_variant, tqma6ul_variant, mb);

	return somtype;
}

#if !IS_ENABLED(CONFIG_SPL_BUILD)
int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

const char *tq_get_modulename(void)
{
	if (is_mx6ul()) {
		if (IS_ENABLED(CONFIG_TQMA6UL_VARIANT_STANDARD))
			return "TQMa6ULx";
		if (IS_ENABLED(CONFIG_TQMA6UL_VARIANT_LGA))
			return "TQMa6ULxL";
	}

	if (is_mx6ull()) {
		if (IS_ENABLED(CONFIG_TQMA6UL_VARIANT_STANDARD))
			return "TQMa6ULLx";
		if (IS_ENABLED(CONFIG_TQMA6UL_VARIANT_LGA))
			return "TQMa6ULLxL";
	}

	return "Unknown";
}

int checkboard(void)
{
	printf("Board: %s on %s\n", tq_get_modulename(),
	       tq_bb_get_boardname());

	return tq_bb_checkboard();
}

#if IS_ENABLED(CONFIG_CMD_BMODE)
static const struct boot_mode tqma6ul_board_boot_modes[] = {
	/* 4 bit bus width */
	{"sd", MAKE_CFGVAL(0x42, 0x20, 0x00, 0x00)},
	{"emmc", MAKE_CFGVAL(0x40, 0x28, 0x00, 0x00)},
	{"qspi", MAKE_CFGVAL(0x10, 0x00, 0x00, 0x00)},
	{NULL, 0},
};
#endif

int tq_bb_board_late_init(void)
{
	struct tq_eeprom_data eeprom;
	int ret;

	if (IS_ENABLED(CONFIG_CMD_BMODE))
		add_board_boot_modes(tqma6ul_board_boot_modes);

	env_set_runtime("board_name", tq_get_modulename());

	ret = tq_read_module_eeprom(&eeprom);
	if (!ret)
		tq_board_handle_eeprom_data("TQMa6UL", &eeprom);
	else
		printf("EEPROM: err %d\n", ret);

	return 0;
}

int tq_bb_checkboard(void)
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
#if IS_ENABLED(CONFIG_OF_BOARD_SETUP) && IS_ENABLED(CONFIG_OF_LIBFDT)
int tq_bb_ft_board_setup(void *blob, struct bd_info *bd)
{
	struct mmc *mmc = find_mmc_device(0);
	int err;

	fdt_fixup_memory(blob, (u64)PHYS_SDRAM, (u64)gd->ram_size);

	/* bring in eMMC dsr settings if needed */
	if (mmc && (!mmc_init(mmc))) {
		if (tq_emmc_need_dsr(mmc) > 0) {
			err = tq_ft_fixup_emmc_dsr(blob,
						   "/soc/bus@2100000/mmc@2194000",
						   (u32)tq_emmc_dsr);
			if (err)
				printf("ERROR: failed to patch e-MMC DSR in DT\n");
		}
	} else {
		printf("e-MMC: not present?\n");
	}

	if (IS_ENABLED(CONFIG_TQ_SPI_NOR)) {
		const char * const path = "/soc/bus@2100000/spi@21e0000";
		static const struct node_info nodes[] = {
			{ "jedec,spi-nor",	MTD_DEV_TYPE_NOR, },
		};

		/*
		 * Update MTD partition nodes using info
		 * from mtdparts env var
		 * for [Q]SPI this needs the device probed.
		 */
		printf("   Updating SPI NOR status...\n");
		tq_ft_spi_setup(blob, path, nodes, ARRAY_SIZE(nodes));
	}

	return 0;
}
#endif /* IS_ENABLED(CONFIG_OF_BOARD_SETUP) && IS_ENABLED(CONFIG_OF_LIBFDT) */

#endif /* !IS_ENABLED(CONFIG_SPL_BUILD) */
