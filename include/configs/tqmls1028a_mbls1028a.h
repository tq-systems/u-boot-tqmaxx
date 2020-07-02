/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019-2020 TQ-Systems GmbH
 *
 * Configuration settings for the TQ Systems MBLS1028A Carrier board for
 * TQMLS1028A module.
 */

#ifndef __CONFIG_TQMLS1028A_MBLS1028A_H__
#define __CONFIG_TQMLS1028A_MBLS1028A_H__

#define CLOCKGEN_I2C_BUS_NUM		1
#define CLOCKGEN_I2C_ADDR		0x70

#define GPIO_EC1_RESET			"gpio@70_1"
#define GPIO_SGMII_RESET		"gpio@70_3"
#define GPIO_QSGMII_RESET		"gpio@70_5"

#define GPIO_USB_RST			"gpio@25_1"

#define BOOT_ENV_BOARD                                                         \
	"console=ttyS0,115200\0"                                               \
	""

#endif /* __CONFIG_TQMLS1028A_MBLS1028A_H__ */
