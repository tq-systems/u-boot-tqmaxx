// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016-2024 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Marco Felsch, Matthias Schiffer
 */

#include <env.h>
#include <asm/arch/clock.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/global_data.h>
#include <asm/mach-imx/sys_proto.h>
#include <malloc.h>

#include "../common/tq_bb.h"
#include "tqma6ul.h"

DECLARE_GLOBAL_DATA_PTR;

const char *tq_bb_get_boardname(void)
{
	return "MBa6ULxL";
}

int board_early_init_f(void)
{
	return tq_bb_board_early_init_f();
}

static void mba6ulxl_setup_eth(void)
{
	struct iomuxc *const iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;

	if (check_module_fused(MODULE_ENET1)) {
		printf("FEC1: fused\n");
	} else {
		/*
		 * Use 50M anatop loopback REF_CLK1 for ENET1,
		 * clear gpr1[13], set gpr1[17]
		 */
		clrsetbits_le32(&iomuxc_regs->gpr[1], IOMUX_GPR1_FEC1_MASK,
				IOMUX_GPR1_FEC1_CLOCK_MUX1_SEL_MASK);

		enable_fec_anatop_clock(0, ENET_50MHZ);
	}

	if (check_module_fused(MODULE_ENET2)) {
		printf("FEC2: fused\n");
	} else {
		/*
		 * Use 50M anatop loopback REF_CLK1 for ENET2,
		 * clear gpr1[14], set gpr1[18]
		 */
		clrsetbits_le32(&iomuxc_regs->gpr[1], IOMUX_GPR1_FEC2_MASK,
				IOMUX_GPR1_FEC2_CLOCK_MUX1_SEL_MASK);

		enable_fec_anatop_clock(1, ENET_50MHZ);
	}

	enable_enet_clk(1);
}

int board_init(void)
{
	return 0;
}

static void mba6ulxl_set_fdt_file(void)
{
	/* Longest FDT name */
	char dt[] = "imx6ull-tqma6ull2l-mba6ulxl.dtb";
	enum tqma6ul_som_type somtype;

	somtype = tqma6ul_dt(dt, sizeof(dt), "mba6ulxl.dtb");
	if (somtype == tqma6_som_type_unknown)
		return;

	if (somtype != tqma6_som_type_lga) {
		debug("wrong TQMa6UL variant\n");
		return;
	}

	if (!env_get("fdtfile"))
		env_set_runtime("fdtfile", dt);
}

int board_late_init(void)
{
	unsigned int bmode =
		(imx6_src_get_boot_mode() & IMX6_BMODE_MASK) >> IMX6_BMODE_SHIFT;

	tq_bb_board_late_init();

	printf("Boot: ");

	switch (bmode) {
	case IMX6_BMODE_MMC:
	case IMX6_BMODE_EMMC:
		printf("eMMC\n");
		env_set_runtime("boot_dev", "mmc");
		board_late_mmc_env_init();
		break;
	case IMX6_BMODE_SD:
	case IMX6_BMODE_ESD:
		printf("SD\n");
		env_set_runtime("boot_dev", "mmc");
		board_late_mmc_env_init();
		break;
	case IMX6_BMODE_QSPI:
	case IMX6_BMODE_NOR:
		printf("QSPI\n");
		env_set_runtime("boot_dev", "qspi");
		break;
	default:
		printf("unhandled boot device %u\n", bmode);
	}

	/* Provide default setting for fdtfile if nothing in env is set */
	if (!env_get("fdtfile"))
		mba6ulxl_set_fdt_file();

	mba6ulxl_setup_eth();

	return 0;
}

int board_mmc_get_env_dev(int devno)
{
	unsigned int port = (imx6_src_get_boot_mode() >> 11) & 0x3;

	switch (port) {
	case 0:
		/* SDHC1 - SD card on MBa6ULxL */
		return 1;

	default:
		/* Return eMMC device otherwise */
		return 0;
	}
}

#if IS_ENABLED(CONFIG_OF_BOARD_SETUP) && IS_ENABLED(CONFIG_OF_LIBFDT)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	return tq_bb_ft_board_setup(blob, bd);
}
#endif /* IS_ENABLED(CONFIG_OF_BOARD_SETUP) && IS_ENABLED(CONFIG_OF_LIBFDT) */
