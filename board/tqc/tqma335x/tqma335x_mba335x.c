// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021-2025 TQ-Systems GmbH <u-boot@ew.tq-group.com>, D-82229 Seefeld, Germany.
 * Authors: Gregor Herburger, Matthias Schiffer
 *
 * Based on:
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 */

#include <common.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <dm.h>
#include <environment.h>
#include <spl.h>

#include "tqma335x.h"
#include "../common/tqc_sdmmc.h"

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_DM_SERIAL
struct serial_device *default_serial_console(void)
{
	return &eserial4_device;
}
#endif

static struct module_pin_mux uart4_pin_mux[] = {
	{OFFSET(gpmc_wait0), (MODE(6) | PULLUP_EN | RXACTIVE)},	/* UART4_RXD */
	{OFFSET(gpmc_wpn), (MODE(6) | PULLUDEN)},		/* UART4_TXD */
	{-1},
};

static struct module_pin_mux mmc0_pin_mux[] = {
	{OFFSET(mmc0_dat3), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT3 */
	{OFFSET(mmc0_dat2), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT2 */
	{OFFSET(mmc0_dat1), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT1 */
	{OFFSET(mmc0_dat0), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT0 */
	{OFFSET(mmc0_clk), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_CLK */
	{OFFSET(mmc0_cmd), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_CMD */
	{-1},
};

void set_uart_mux_conf(void)
{
	configure_module_pin_mux(uart4_pin_mux);
}

void enable_board_pin_mux(void)
{
	configure_module_pin_mux(mmc0_pin_mux);
}

#if defined(CONFIG_ENV_IS_IN_MMC)
int mmc_get_env_dev(void)
{
	switch (gd->arch.omap_boot_device) {
	case BOOT_DEVICE_MMC1:
		return 0;
	case BOOT_DEVICE_MMC2:
		return 1;
	default:
		return -ENODEV;
	}
}
#endif

#if !defined(CONFIG_SPL_BUILD)
enum env_location env_get_location(enum env_operation op, int prio)
{
	if (prio != 0)
		return ENVL_UNKNOWN;

	if (!gd->arch.omap_boot_device)
		save_omap_boot_params();

	switch (gd->arch.omap_boot_device) {
#if IS_ENABLED(CONFIG_ENV_IS_IN_MMC)
	case BOOT_DEVICE_MMC1:
	case BOOT_DEVICE_MMC2:
		debug("env + boot SD/e-MMC\n");
		return ENVL_MMC;
#endif
#if IS_ENABLED(CONFIG_ENV_IS_IN_SPI_FLASH)
	case BOOT_DEVICE_SPI:
		debug("env + boot SPI NOR\n");
		return ENVL_SPI_FLASH;
#endif
	default:
		if (IS_ENABLED(CONFIG_ENV_IS_NOWHERE))
			return ENVL_NOWHERE;
		return ENVL_UNKNOWN;
	}
}
#endif

#if defined(CONFIG_BOARD_LATE_INIT) && !defined(CONFIG_SPL_BUILD)
int board_late_init(void)
{
	puts("BOOT:\t");
	switch (gd->arch.omap_boot_device) {
	case BOOT_DEVICE_MMC1:
		puts("MMC1 (SD)\n");
		break;
	case BOOT_DEVICE_MMC2:
		puts("MMC2 (e-MMC)\n");
		break;
	case BOOT_DEVICE_SPI:
		puts("SPI (SPI-NOR)\n");
		break;
	default:
		printf("unknown (%u)\n", gd->arch.omap_boot_device);
		break;
	}

	tqma335x_read_eeprom();

	board_late_mmc_env_init();

	return 0;
}
#endif

#if defined(CONFIG_OF_BOARD_SETUP)
int tqc_bb_ft_board_setup(void *fdt, bd_t *bd)
{
	return 0;
}
#endif

#if !CONFIG_IS_ENABLED(OF_CONTROL)
static const struct omap_hsmmc_plat am335x_mmc0_platdata = {
	.base_addr = (struct hsmmc *)OMAP_HSMMC1_BASE,
	.cfg.host_caps = MMC_MODE_HS_52MHz | MMC_MODE_HS | MMC_MODE_4BIT,
	.cfg.f_min = 400000,
	.cfg.f_max = 52000000,
	.cfg.voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195,
	.cfg.b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT,
};

U_BOOT_DEVICE(am335x_mmc0) = {
	.name = "omap_hsmmc",
	.platdata = &am335x_mmc0_platdata,
};
#endif
