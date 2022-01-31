// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Board specific initialization for TQ-Systems MBa65xx
 *
 * Copyright (C) 2017-2018 Texas Instruments Incorporated - http://www.ti.com/
 * Copyright (C) 2020-2022 TQ-Systems GmbH
 *
 */

#include <common.h>
#include <dm.h>
#include <env.h>
#include <env_internal.h>
#include <spl.h>
#include <mmc.h>

#include "tqma65xx.h"
#include "tqma65xx_mba65xx.h"
#include "../common/board_gpio.h"

DECLARE_GLOBAL_DATA_PTR;

enum {
	PRG0_ETH_EN,
	PRG1_ETH_EN,
};

static struct tq_board_gpio_data board_gpios[] = {
	GPIO_INIT_DATA_ENTRY(PRG0_ETH_EN, "gpio@20_2",
			     GPIOD_IS_IN | GPIOD_ACTIVE_LOW),
	GPIO_INIT_DATA_ENTRY(PRG1_ETH_EN, "gpio@20_5",
			     GPIOD_IS_IN | GPIOD_ACTIVE_LOW),
};

#define DTB_NAME_PRG0_ETH "k3-am65-tqma65xx-mba65xx-eth-prg0.dtbo"
#define DTB_NAME_PRG1_ETH "k3-am65-tqma65xx-mba65xx-eth-prg1.dtbo"
#define DTB_NAME_AUDIO "k3-am65-tqma65xx-mba65xx-audio.dtbo"

static void mba65xx_setup_env_bootdev(void)
{
	switch(tqma65xx_get_boot_device()) {
	case BOOT_DEVICE_MMC1:
		if (!env_get("boot"))
			env_set_runtime("boot", "mmc");
		if (!env_get("mmcdev"))
			env_set_runtime("mmcdev", "0");
		break;
	case BOOT_DEVICE_MMC2:
		if (!env_get("boot"))
			env_set_runtime("boot", "mmc");
		if (!env_get("mmcdev"))
			env_set_runtime("mmcdev", "1");
		break;
	case BOOT_DEVICE_SPI:
		if (!env_get("boot"))
			env_set_runtime("boot", "ubi");
		break;
	default:
		printf("Warning: unknown boot device\n");
	}
}

enum env_location env_get_location(enum env_operation op, int prio)
{
	if (prio > 0)
		return ENVL_UNKNOWN;

	switch (tqma65xx_get_boot_device()) {
	case BOOT_DEVICE_MMC1:
	case BOOT_DEVICE_MMC2:
		return ENVL_MMC;
	case BOOT_DEVICE_SPI:
		return ENVL_SPI_FLASH;
	default:
		return ENVL_NOWHERE;
	}
}

#ifdef CONFIG_ENV_IS_IN_MMC
int mmc_get_env_dev(void)
{
	switch (tqma65xx_get_boot_device()) {
	case BOOT_DEVICE_MMC1:
		return 0;
	case BOOT_DEVICE_MMC2:
		return 1;
	default:
		return CONFIG_SYS_MMC_ENV_DEV;
	}
}
#endif

static void mba65xx_setup_env_overlays(void)
{
	if (dm_gpio_get_value(&board_gpios[PRG0_ETH_EN].desc) == 1) {
		printf("PRG0 Ethernet enabled\n");
		env_set_runtime("name_overlay1", DTB_NAME_PRG0_ETH);
	} else {
		printf("Audio enabled\n");
		env_set_runtime("name_overlay1", DTB_NAME_AUDIO);
	}

	if (dm_gpio_get_value(&board_gpios[PRG1_ETH_EN].desc) == 1) {
		printf("PRG1 Ethernet enabled\n");
		env_set_runtime("name_overlay2", DTB_NAME_PRG1_ETH);
	} else {
		printf("PRG1 Ethernet disabled\n");
		env_set_runtime("name_overlay2", "");
	}
}

int board_late_init(void)
{
	tqma65xx_parse_eeprom();

	mba65xx_setup_env_bootdev();

	tq_board_gpio_init(board_gpios, ARRAY_SIZE(board_gpios));
	mba65xx_setup_env_overlays();

	mba65xx_setup_clock_synthesizer();

	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	return tqma65xx_ft_board_setup(blob, bd);
}
#endif
