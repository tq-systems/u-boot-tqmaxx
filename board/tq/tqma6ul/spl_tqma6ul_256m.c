// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023-2024 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Max Merchel
 */

#include <config.h>
#include <asm/io.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/mx6ul-ddr.h>
#include <asm/arch/iomux.h>
#include <asm/arch/crm_regs.h>
#include <linux/delay.h>
#include <common.h>

static inline void init_write_reg(u32 address, u32 value)
{
	__raw_writel(value, address);
}

static void tqma6ul_256_init_ddr_controller(void)
{
	/* TQMa6ul DDR config */

	debug("SPL: tqma6ul ddr iomux ....\n");

	/* DDR IO TYPE: */
	init_write_reg(0x020E04B4, 0x000C0000);
	init_write_reg(0x020E04AC, 0x00000000);
	/* CLOCK: */
	init_write_reg(0x020E027C, 0x00000030);
	/* Control: */
	init_write_reg(0x020E0250, 0x00000030);
	init_write_reg(0x020E024C, 0x00000030);
	init_write_reg(0x020E0490, 0x00000030);
	init_write_reg(0x020E0288, 0x00000030);
	init_write_reg(0x020E0270, 0x00000000);
	init_write_reg(0x020E0260, 0x00000030);
	init_write_reg(0x020E0264, 0x00000030);
	init_write_reg(0x020E04A0, 0x00000030);
	/* Data Strobes: */
	init_write_reg(0x020E0494, 0x00020000);
	init_write_reg(0x020E0280, 0x00000030);
	init_write_reg(0x020E0284, 0x00000030);
	/* Data: */
	init_write_reg(0x020E04B0, 0x00020000);
	init_write_reg(0x020E0498, 0x00000030);
	init_write_reg(0x020E04A4, 0x00000030);
	init_write_reg(0x020E0244, 0x00000030);
	init_write_reg(0x020E0248, 0x00000030);

	debug("SPL: tqma6ul ddr controller registers ....\n");

	/* MMDC_MDSCR - MMDC Core Special Command Register */
	init_write_reg(0x021B001C, 0x00008000);

	debug("SPL: tqma6ul ddr calibrations ....\n");

	/* DDR_PHY_P0_MPZQHWCTRL , enable both one-time & periodic HW ZQ calibration. */
	init_write_reg(0x021B0800, 0xA1390003);

#if IS_ENABLED(CONFIG_TQMA6UL_VARIANT_STANDARD)

	debug("SPL: tqma6ul ddr calibration standard variant ....\n");

	init_write_reg(0x021B080C, 0x00000000);
	init_write_reg(0x021B083C, 0x41480144);
	init_write_reg(0x021B0848, 0x40404E54);
	init_write_reg(0x021B0850, 0x40404E48);

#elif IS_ENABLED(CONFIG_TQMA6UL_VARIANT_LGA)

	debug("SPL: tqma6ul ddr calibration lga variant ....\n");

	init_write_reg(0x021B080C, 0x00130003);
	init_write_reg(0x021B083C, 0x41540154);
	init_write_reg(0x021B0848, 0x40405050);
	init_write_reg(0x021B0850, 0x40404E4C);

#else
#error
#endif /* IS_ENABLED(CONFIG_TQMA6UL_VARIANT_STANDARD) */

	init_write_reg(0x021B081C, 0x33333333);
	init_write_reg(0x021B0820, 0x33333333);
	init_write_reg(0x021B082C, 0xf3333333);
	init_write_reg(0x021B0830, 0xf3333333);
	init_write_reg(0x021B08C0, 0x00921012);

	/*
	 * Complete calibration by forced measurement:
	 */
	init_write_reg(0x021B08B8, 0x00000800);
	init_write_reg(0x021B0004, 0x0002002D);

	debug("SPL: tqma6ul ddr mmdc ....\n");

	init_write_reg(0x021B0008, 0x00333030);
	init_write_reg(0x021B000C, 0x676B52F3);
	init_write_reg(0x021B0010, 0xB66D8B63);
	init_write_reg(0x021B0014, 0x01FF00DB);
	init_write_reg(0x021B0018, 0x00201740);
	init_write_reg(0x021B001C, 0x00008000);
	init_write_reg(0x021B002C, 0x000026D2);
	init_write_reg(0x021B0030, 0x006B1023);
	init_write_reg(0x021B0040, 0x00000047);
	init_write_reg(0x021B0000, 0x83180000);

	debug("SPL: tqma6ul ddr cs0 ....\n");

	init_write_reg(0x021B001C, 0x02008032);
	init_write_reg(0x021B001C, 0x00008033);
	init_write_reg(0x021B001C, 0x00048031);
	init_write_reg(0x021B001C, 0x15208030);
	init_write_reg(0x021B001C, 0x04008040);
	init_write_reg(0x021B0020, 0x00000800);
	init_write_reg(0x021B0818, 0x00000227);
	init_write_reg(0x021B0004, 0x0002552D);
	init_write_reg(0x021B0404, 0x00011006);
	init_write_reg(0x021B001C, 0x00000000);
}

void tqma6ul_256_init(void)
{
	tqma6ul_256_init_ddr_controller();
}
