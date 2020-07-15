// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2020 TQ-Systems GmbH
 * Markus Niebel <Markus.Niebel@tq-group.com>
 */

#include <common.h>
#include <i2c.h>

#define PCF85063_REG_CTRL1		0x00 /* status */
#define PCF85063_REG_CTRL1_CAP_SEL	BIT(0)

int tqc_pcf85063_adjust_capacity(int bus, int address, int quartz_load)
{
	struct udevice *dev;
	int ret;
	u8 val;

	if ((quartz_load != 7000) && (quartz_load != 12500))
		return -EINVAL;

	ret = i2c_get_chip_for_busnum(bus, address, 1, &dev);
	if (ret)
		return ret;

	val = dm_i2c_reg_read(dev, PCF85063_REG_CTRL1);
	/* Set Bit 0 of Register 0 of RTC to adjust to 12.5 pF */
	switch (quartz_load) {
		case 7000:
			val &= ~PCF85063_REG_CTRL1_CAP_SEL;
			break;
		case 12500:
			val |= PCF85063_REG_CTRL1_CAP_SEL;
			break;
	}

	ret = dm_i2c_reg_write(dev, 0x00, val);

	return ret;
}
