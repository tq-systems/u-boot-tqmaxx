// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 - 2022 TQ-Systems GmbH <license@tq-group.com>, D-82229 Seefeld, Germany.
 * Author: Gregor Herburger
 *
 * Based on:
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 */

#include <common.h>
#include <cpsw.h>
#include <dm.h>
#include <environment.h>
#include <errno.h>
#include <i2c.h>
#include <micrel.h>
#include <miiphy.h>
#include <spl.h>
#include <serial.h>
#include <asm/io.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <asm/arch/clk_synthesizer.h>
#include <asm/arch/clock.h>
#include <asm/arch/cpu.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/hardware.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mem.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/mux.h>
#include <asm/arch/omap.h>
#include <asm/arch/sys_proto.h>
#include <asm/omap_common.h>
#include <asm/omap_sec_common.h>
#include <asm/omap_mmc.h>
#include <power/tps65910.h>
#include <watchdog.h>

#include "tqma335x.h"

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_DM_SERIAL
struct serial_device *default_serial_console(void)
{
	return &eserial4_device;
}
#endif

void set_uart_mux_conf(void)
{
	enable_uart4_pin_mux();
}

void enable_board_pin_mux(void)
{
	enable_mmc0_pin_mux();
	enable_uart4_pin_mux();
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

enum env_location board_get_envl(enum env_location loc, enum env_operation op,
				 int prio)
{
#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_TARGET_TQMA335X)
	switch (gd->arch.omap_boot_device) {
	case BOOT_DEVICE_MMC1:
	case BOOT_DEVICE_MMC2:
		if (IS_ENABLED(CONFIG_ENV_IS_IN_MMC)) {
			debug("env + boot SD/e-MMC\n");
			loc = ENVL_MMC;
		}
		break;
	case BOOT_DEVICE_SPI:
		if (IS_ENABLED(CONFIG_ENV_IS_IN_SPI_FLASH)) {
			debug("env + boot SPI NOR\n");
			loc = ENVL_SPI_FLASH;
		}
		break;
	default:
		if (IS_ENABLED(CONFIG_ENV_IS_NOWHERE))
			loc = ENVL_NOWHERE;
		break;
	}
#endif

	return loc;
}

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
