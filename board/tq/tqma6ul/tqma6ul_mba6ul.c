// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Marco Felsch, Matthias Schiffer
 */

#include <common.h>
#include <env.h>
#include <log.h>
#include <asm/arch/clock.h>
#include <asm/arch/iomux.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx6-pins.h>
#include <asm/global_data.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/sys_proto.h>

#include "../common/tq_sdmmc.h"
#include "tqma6ul_common.h"

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE | \
		       PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
		       PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST | PAD_CTL_HYS)

static const iomux_v3_cfg_t mba6ul_uart1_pads[] = {
	NEW_PAD_CTRL(MX6_PAD_UART1_TX_DATA__UART1_DCE_TX, UART_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_UART1_RX_DATA__UART1_DCE_RX, UART_PAD_CTRL),
};

static void mba6ul_setup_iomuxc_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(mba6ul_uart1_pads,
					 ARRAY_SIZE(mba6ul_uart1_pads));
}

int board_early_init_f(void)
{
	mba6ul_setup_iomuxc_uart();

	return tqma6ul_common_early_init_f();
}

static void mba6ul_setup_eth(void)
{
	struct iomuxc *const iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;

	if (check_module_fused(MODULE_ENET1)) {
		puts("FEC1: fused\n");
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
		puts("FEC2: fused\n");
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
	mba6ul_setup_eth();

	return 0;
}

static void mba6ul_set_fdt_file(void)
{
	/* Enough space for the longest FDT name + 1 */
	const char *cpu, *tqma6ul_variant;
	unsigned int mx6ul_variant;
	char fdt_file[30];

	if (is_mx6ul()) {
		cpu = "ul";
	} else if (is_mx6ull()) {
		cpu = "ull";
	} else {
		debug("unknown CPU\n");
		return;
	}

	/* MX6UL1 vs MX6UL2 */
	mx6ul_variant = check_module_fused(MODULE_ENET2) ? 1 : 2;

	if (IS_ENABLED(CONFIG_TQMA6UL_VARIANT_STANDARD)) {
		tqma6ul_variant = "";
	} else if (IS_ENABLED(CONFIG_TQMA6UL_VARIANT_LGA)) {
		tqma6ul_variant = "l";
	} else {
		debug("unknown TQMa6UL variant\n");
		return;
	}

	/* Construct name like imx6ull-tqma6ull2l-mba6ulx.dtb */
	snprintf(fdt_file, sizeof(fdt_file), "imx6%s-tqma6%s%u%s-mba6ulx.dtb",
		 cpu, cpu, mx6ul_variant, tqma6ul_variant);
	env_set_runtime("fdt_file", fdt_file);
}

int board_late_init(void)
{
	unsigned int bmode =
		(imx6_src_get_boot_mode() & IMX6_BMODE_MASK) >> IMX6_BMODE_SHIFT;
	printf("Boot: ");

	switch (bmode) {
	case IMX6_BMODE_MMC:
	case IMX6_BMODE_EMMC:
		printf("eMMC\n");
		env_set_runtime("boot_dev", "mmc");
		tq_sdmmc_env_init();
		break;
	case IMX6_BMODE_SD:
	case IMX6_BMODE_ESD:
		printf("SD\n");
		env_set_runtime("boot_dev", "mmc");
		tq_sdmmc_env_init();
		break;
	case IMX6_BMODE_QSPI:
		printf("QSPI\n");
		env_set_runtime("boot_dev", "qspi");
		break;
	default:
		printf("unhandled boot device %u\n", bmode);
	}

	/* Provide default setting for fdt_file if nothing in env is set */
	if (!env_get("fdt_file"))
		mba6ul_set_fdt_file();

	return 0;
}

int checkboard(void)
{
	printf("Board: %s on MBa6ULx\n", tqma6ul_common_get_boardname());

	return tqma6ul_common_checkboard();
}

int board_mmc_get_env_dev(int devno)
{
	unsigned int port = (imx6_src_get_boot_mode() >> 11) & 0x3;

	switch (port) {
	case 0:
		/* SDHC1 - SD card on MBa6ULx */
		return 1;

	default:
		/* Return eMMC device otherwise */
		return 0;
	}
}

#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	return tqma6ul_common_ft_board_setup(blob, bd);
}
#endif
