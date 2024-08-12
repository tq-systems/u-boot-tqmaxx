// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016-2024 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel, Steffen Doster
 */

#include <common.h>
#include <env.h>
#include <init.h>
#include <log.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx7-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>

#include "../common/tq_bb.h"

const char *tq_bb_get_boardname(void)
{
	return "MBa7x";
}

int board_early_init_f(void)
{
	return 0;
}

static int mba7_setup_fec(int fec_id)
{
	struct iomuxc_gpr_base_regs *const iomuxc_gpr_regs =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;
	int ret;

	switch (fec_id) {
	case 0:
		/* Use 125M anatop REF_CLK1 for ENET1, clear gpr1[13], gpr1[17]*/
		clrsetbits_le32(&iomuxc_gpr_regs->gpr[1],
				IOMUXC_GPR_GPR1_GPR_ENET1_TX_CLK_SEL_MASK |
				IOMUXC_GPR_GPR1_GPR_ENET1_CLK_DIR_MASK, 0);
		break;
	case 1:
		/* Use 125M anatop REF_CLK2 for ENET2, clear gpr1[14], gpr1[18]*/
		clrsetbits_le32(&iomuxc_gpr_regs->gpr[1],
				IOMUXC_GPR_GPR1_GPR_ENET2_TX_CLK_SEL_MASK |
				IOMUXC_GPR_GPR1_GPR_ENET2_CLK_DIR_MASK, 0);
		break;
	default:
		printf("FEC%d: unsupported\n", fec_id);
		return -EINVAL;
	}

	ret = set_clk_enet(ENET_125MHZ);
	if (ret)
		return ret;

	return 0;
}

/*!
 * @brief HW_PMU_REG_HSIC_1P2 - Anadig 1.2V HSIC Regulator Control Register (RW)
 *
 * Reset value: 0x00001878U
 *
 * This register defines the control and status bits for the 1.1V regulator.
 * This regulator is designed to power the digital portions of the analog cells.
 */
static void hsic_1p2_regulator_out(void)
{
	struct mxc_ccm_anatop_reg *ccm_anatop = (struct mxc_ccm_anatop_reg *)
					 ANATOP_BASE_ADDR;
	/*
	 * allow the GPC to override register settings.
	 * use ANATOP_BASE_ADDR which is 0x30360000 instead of PMU_BASE (0x30360200)
	 */
	writel(PMU_REG_HSIC_1P2_SET_OVERRIDE_MASK, &ccm_anatop->reg_hsic_1p2_set);
}

#define GPC_PGC_HSIC				0xd00
#define GPC_PGC_CPU_MAPPING			0xec
#define GPC_PU_PGC_SW_PUP_REQ			0xf8
#define BM_CPU_PGC_SW_PDN_PUP_REQ_USB_HSIC_PHY	0x10
#define USB_HSIC_PHY_A7_DOMAIN			0x40

static void imx_set_usb_hsic_power(void)
{
	u32 reg;
	u32 val;

	writel(1, GPC_IPS_BASE_ADDR + GPC_PGC_HSIC);

	reg = GPC_IPS_BASE_ADDR + GPC_PGC_CPU_MAPPING;
	val = readl(reg);
	val |= USB_HSIC_PHY_A7_DOMAIN;
	writel(val, reg);

	hsic_1p2_regulator_out();

	reg = GPC_IPS_BASE_ADDR + GPC_PU_PGC_SW_PUP_REQ;
	val = readl(reg);
	val |= BM_CPU_PGC_SW_PDN_PUP_REQ_USB_HSIC_PHY;
	writel(val, reg);

	while ((readl(reg) &
		BM_CPU_PGC_SW_PDN_PUP_REQ_USB_HSIC_PHY) != 0)
		;

	writel(0, GPC_IPS_BASE_ADDR + GPC_PGC_HSIC);
}

int tq_bb_board_init(void)
{
	mba7_setup_fec(0);

	if (!is_cpu_type(MXC_CPU_MX7S))
		mba7_setup_fec(1);

	imx_set_usb_hsic_power();

	return 0;
}

int tq_bb_board_late_init(void)
{
	puts("Boot:\t");

	if (is_boot_from_usb()) {
		puts("USB\n");
		env_set_runtime("boot_dev", "mmc");
		env_set_runtime("mmcdev", "0");
		env_set_runtime("mmcblkdev", "0");
	} else {
		/*
		 * try to get sd card slots in order:
		 * eMMC: on Module
		 * -> therefore index 0 for bootloader
		 * index n in kernel (controller instance 3) -> patches needed for
		 * alias indexing
		 * SD1: on Mainboard
		 * index n in kernel (controller instance 1) -> patches needed for
		 * alias indexing
		 * we assume to have a kernel patch that will present mmcblk dev
		 * indexed like controller devs
		 */
		enum boot_device bd = get_boot_device();

		switch (bd) {
		case MMC3_BOOT:
			puts("USDHC3(eMMC)\n");
			env_set_runtime("boot_dev", "mmc");
			env_set_runtime("mmcdev", "0");
			env_set_runtime("mmcblkdev", "0");
			break;
		case SD1_BOOT:
			puts("USDHC1(SD)\n");
			env_set_runtime("boot_dev", "mmc");
			env_set_runtime("mmcdev", "1");
			env_set_runtime("mmcblkdev", "1");
			break;
		case QSPI_BOOT:
			puts("QSPI\n");
			env_set_runtime("boot_dev", "qspi");
			env_set_runtime("mmcdev", "0");
			env_set_runtime("mmcblkdev", "0");
			break;
		default:
			printf("unhandled boot device %d\n", (int)bd);
			env_set_runtime("mmcdev", "0");
			env_set_runtime("mmcblkdev", "0");
		}
	}

	if (!env_get("fdtfile")) {
		/* provide default setting for fdtfile if nothing in env is set */
		u32 cpurev = get_cpu_rev();

		switch ((cpurev & 0xFF000) >> 12) {
		case MXC_CPU_MX7S:
			env_set_runtime("fdtfile", "imx7s-mba7.dtb");
			break;
		case MXC_CPU_MX7D:
			env_set_runtime("fdtfile", "imx7d-mba7.dtb");
			break;
		default:
			debug("unknown CPU");
		}
	}

	return 0;
}

int board_mmc_get_env_dev(int devno)
{
	switch (devno) {
	case 2:
		/* eMMC */
		return 0;
	case 0:
		/* SD card */
		return 1;
	default:
		/* Unknown */
		return 0;
	}
}
