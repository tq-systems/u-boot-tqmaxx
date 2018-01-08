/*
 * Copyright (C) 2014 Markus Niebel <Markus.Niebel@tq-group.com>
 *
 * SPL code for the TQ Systems TQMa6<Q,S> module.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <environment.h>
#include <i2c.h>
#include <asm/io.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <spl.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * called from C runtime startup code (arch/arm/lib/crt0.S:_main)
 * - we have a stack and a place to store GD, both in SRAM
 * - no variable global data is available
 */
void board_init_f(ulong dummy)
{
	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	/* iomux and setup of uart */
	board_early_init_f();

	/* setup GP timer */
	timer_init();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

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
	return CONFIG_FLASH_SECTOR_SIZE;
}
#endif

#if defined(CONFIG_SPL_MMC_SUPPORT)
u32 spl_mmc_get_uboot_sector(void)
{
	return CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR;
}
#endif
