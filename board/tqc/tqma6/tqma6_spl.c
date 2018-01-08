/*
 * Copyright (C) 2014 Markus Niebel <Markus.Niebel@tq-group.com>
 *
 * SPL code for the TQ Systems TQMa6<Q,S> module.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <config.h>

#include <asm/io.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <environment.h>
#include <fdt_support.h>
#include <i2c.h>
#include <spl.h>


void tqma6s_init(void);
void tqma6dl_init(void);
void tqma6q_init(void);
void tqma6qp_init(void);


DECLARE_GLOBAL_DATA_PTR;

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/* set the default clock gate to save power */
	writel(0x00C03F3F, &ccm->CCGR0);
	writel(0x0030FC03, &ccm->CCGR1);
	writel(0x0FFFC000, &ccm->CCGR2);
	writel(0x3FF00000, &ccm->CCGR3);
	writel(0x00FFF300, &ccm->CCGR4);
	writel(0x0F0000C3, &ccm->CCGR5);
	writel(0x000003FF, &ccm->CCGR6);
}

/*
 * called from C runtime startup code (arch/arm/lib/crt0.S:_main)
 * - we have a stack and a place to store GD, both in SRAM
 * - no variable global data is available
 */
void board_init_f(ulong dummy)
{
	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	ccgr_init();
	gpr_init();

	/* iomux and setup of uart */
	board_early_init_f();

	/* setup GP timer */
	timer_init();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	if (is_mx6solo())
		tqma6s_init();
	else if (is_mx6dl())
		tqma6dl_init();
	else if (is_mx6dq())
		tqma6q_init();
	else if (is_mx6dqp())
		tqma6qp_init();
	else
		hang();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	/* load/boot image from boot device */
	board_init_r(NULL, 0);
}

void reset_cpu(ulong addr)
{
}

#if defined(CONFIG_SPL_SPI_SUPPORT)
u32 spl_spi_get_uboot_offset(void)
{
	return TQMA6_SPI_FLASH_SECTOR_SIZE;
}
#endif

#if defined(CONFIG_SPL_MMC_SUPPORT)
u32 spl_mmc_get_uboot_sector(void)
{
	return CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR;
}
#endif
