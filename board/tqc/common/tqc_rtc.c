// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2020 TQ-Systems GmbH
 * Markus Niebel <Markus.Niebel@tq-group.com>
 */

#include <common.h>
#include <i2c.h>

#define PCF85063_REG_CTRL1		0x00 /* status */
#define PCF85063_REG_CTRL1_CAP_SEL	BIT(0)

#define PCF85063_REG_CTRL2		0x01
#define PCF85063_REG_CTRL2_CLKOUT_MASK	0x07

#define PCF85063_REG_OFFSET		0x02
#define PCF85063_REG_OFFSET_OFFSET_MASK	0x7f
#define PCF85063_REG_OFFSET_MODE	BIT(7)

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

	ret = dm_i2c_reg_write(dev, PCF85063_REG_CTRL1, val);

	return ret;
}

int tqc_pcf85063_set_clkout(int bus, int address, uint8_t clkout)
{
	struct udevice *dev;
	int ret;
	u8 val;

	if (clkout > 0x07)
		return -EINVAL;

	ret = i2c_get_chip_for_busnum(bus, address, 1, &dev);
	if (ret)
		return ret;

	val = dm_i2c_reg_read(dev, PCF85063_REG_CTRL2);
	val &= ~PCF85063_REG_CTRL2_CLKOUT_MASK;
	val |= clkout;
	ret = dm_i2c_reg_write(dev, PCF85063_REG_CTRL2, val);

	return ret;
}

int tqc_pcf85063_set_offset(int bus, int address, bool mode, int offset)
{
	struct udevice *dev;
	int ret;
	u8 val;

	if (offset < -64 || offset > 63)
		return -EINVAL;

	ret = i2c_get_chip_for_busnum(bus, address, 1, &dev);
	if (ret)
		return ret;

	val = ((uint8_t)offset) & PCF85063_REG_OFFSET_OFFSET_MASK;
	if (mode)
		val |= PCF85063_REG_OFFSET_MODE;
	ret = dm_i2c_reg_write(dev, PCF85063_REG_OFFSET, val);

	return ret;
}
