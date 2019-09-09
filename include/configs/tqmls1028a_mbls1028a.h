/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2019
 *
 * Configuration settings for the TQ Systems MBLS1028A Carrier board for
 * TQMLS1028A.
 */

#ifndef __CONFIG_TQMLS1028A_MBLS1028AL_H__
#define __CONFIG_TQMLS1028A_MBLS1028AL_H__

#define RGMII_PHY_DEV_ADDR 0x00
#define SGMII_PHY_DEV_ADDR 0x03
#define QSGMII_PHY_DEV_ADDR 0x1C
#define PHY_MDIO_NAME "netc_mdio"

#define CLOCKGEN_I2C_BUS_NUM	0x4
#define CLOCKGEN_I2C_ADDR		0x70

#define GPIO_EXPANDER_I2C_BUS_NUM	5
#define GPIO_EXPANDER1_I2C_ADDR		0x25
#define GPIO_EXPANDER2_I2C_ADDR		0x70

#define BOOT_ENV_BOARD                                                         \
	"gpio_expander_i2c_bus=" __stringify(GPIO_EXPANDER_I2C_BUS_NUM) "\0"   \
	"gpio_expander_1_addr=" __stringify(GPIO_EXPANDER1_I2C_ADDR) "\0"      \
	"gpio_expander_2_addr=" __stringify(GPIO_EXPANDER2_I2C_ADDR) "\0"      \
	"resetusb=i2c dev ${gpio_expander_i2c_bus}; "                          \
		"i2c mw ${gpio_expander_1_addr} 0x6.1 0xfd; "                  \
		"i2c mw ${gpio_expander_1_addr} 0x2.1 0xfd; "                  \
		"sleep 0.1; "                                                  \
		"i2c mw ${gpio_expander_1_addr} 0x2.1 0xff;\0 "                \
	"resetphy=i2c dev ${gpio_expander_i2c_bus}; "                          \
		"i2c mw ${gpio_expander_2_addr} 0x3 0xd5; "                    \
		"i2c mw ${gpio_expander_2_addr} 0x1 0xd5; "                    \
		"sleep 0.1; "                                                  \
		"i2c mw ${gpio_expander_2_addr} 0x1 0xff;\0"                   \
	"boardinit=run resetusb; run resetphy;\0"
#endif /* __CONFIG_TQMLS1028A_MBLS1028AL_H__ */
