// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019-2020 TQ-Systems GmbH
 *
 * Author: Matthias Schiffer <matthias.schiffer@tq-group.com>
 */

#include <common.h>
#include "tqmls1028a_bb.h"
#include <asm/arch/soc.h>
#include <asm/gpio.h>
#include <i2c.h>

static int clockgen_init(void)
{
	struct udevice *dev;
	int ret;

        ret = i2c_get_chip_for_busnum(CLOCKGEN_I2C_BUS_NUM,
                                      CLOCKGEN_I2C_ADDR, 1, &dev);
	if (ret)
		return ret;

	u8 regvals[][2] = {
		{0x6, 0x1d},
		{0x2f, 0x14},
		{0x6a, 0x80},
		{0x74, 0x80},
		{0x1e, 0xb0},
		{0x1d, 0x90},
		{0x24, 0x7},
		{0x25, 0x7},
		{0x28, 0xe7},
		{0x26, 0x7},
		{0x29, 0x1c},
		{0x27, 0x6},
		{0x23, 0xaa},
		{0x2a, 0x24},
		{0x62, 0x30},
		{0x67, 0x1},
		{0x35, 0x80},
		{0x36, 0xa},
		{0x3b, 0x1},
		{0x1c, 0x16},
		{0x30, 0x3a},
		{0x33, 0x7},
		{0x32, 0xc4},
		{0x1f, 0xc0},
		{0x40, 0x80},
		{0x41, 0xa},
		{0x42, 0x54},
		{0x46, 0x1},
		{0x20, 0xc0},
		{0x4b, 0x80},
		{0x4c, 0xa},
		{0x51, 0x1},
		{0x21, 0xc0},
		{0x57, 0x8},
		{0x5c, 0x1},
		{0x22, 0xc0},
		{0xf1, 0x65},
		{0x2d, 0xee},
		{0x2e, 0x1},
		{0x31, 0x80},
	};

	for (int i = 0; i < ARRAY_SIZE(regvals); i++) {
		ret = dm_i2c_reg_write(dev, regvals[i][0], regvals[i][1]);
		if (ret)
			return ret;
	}

	return 0;
}

static int gpio_lookup_name_and_request(const char *name,
				        struct gpio_desc *gpio,
				        const char *label, ulong flags)
{
	int ret;

	ret = dm_gpio_lookup_name(name, gpio);
	if (ret)
		return ret;

	ret = dm_gpio_request(gpio, label);
	if (ret)
		return ret;

	dm_gpio_set_dir_flags(gpio, flags);

	return 0;
}

static void mbls1028a_reset_one_gpio(const char *label, const char *name,
				     ulong flags, ulong usecs)
{
	struct gpio_desc gpio;
	int ret;

	ret = gpio_lookup_name_and_request(name, &gpio, label,
					   flags | GPIOD_IS_OUT |
					   GPIOD_IS_OUT_ACTIVE);
	if (ret) {
		printf("Failed to request %s GPIO: %d\n", label, ret);
		return;
	}

	udelay(usecs);
	dm_gpio_set_value(&gpio, 0);
}

int board_bb_init(void)
{
	int ret;

	ret = clockgen_init();
	if (ret) {
		printf("Error: failed to initialize clockgen: %d\n", ret);
		printf("Ethernet, PCIe and SATA may not function properly, please check signal switch S10.1\n");
	} else {
		printf("Initialized clockgen\n");
	}

	mbls1028a_reset_one_gpio("EC1_RESET", GPIO_EC1_RESET,
				 GPIOD_ACTIVE_LOW, 1);
	/* Long reset to work around strap config issue */
	mbls1028a_reset_one_gpio("SGMII_RESET", GPIO_SGMII_RESET,
				 GPIOD_ACTIVE_LOW, 2500);
	mbls1028a_reset_one_gpio("QSGMII_RESET", GPIO_QSGMII_RESET,
				 GPIOD_ACTIVE_LOW, 10000);
	mbls1028a_reset_one_gpio("USB_RST", GPIO_USB_RST,
				 GPIOD_ACTIVE_LOW, 3000);

	/* Ensure that everything has time to come out of reset again */
	mdelay(10);

	return 0;
}

void tqmls1028a_bb_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	printf("Booting from: ");
	switch (get_boot_src()) {
	case BOOT_SOURCE_SD_MMC:
		printf("SDHC1 (SD)\n");

		if (!env_get("mmcdev"))
			env_set("mmcdev", "0");
		break;

	case BOOT_SOURCE_SD_MMC2:
		printf("SDHC2 (eMMC)\n");

		if (!env_get("mmcdev"))
			env_set("mmcdev", "1");
		break;

	case BOOT_SOURCE_XSPI_NOR:
		printf("SPI-NOR\n");
		break;

	default:
		printf("unknown\n");
	}
#endif
}
