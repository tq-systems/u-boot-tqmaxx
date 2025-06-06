// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 * Based on driver from https://git.pengutronix.de/cgit/barebox (drivers/rtc/rtc-pcf85063.c)
 * without NVMEM support
 */

/*
 * Date & Time support for Philips/NXP pcf85063 RTC
 */

#include <command.h>
#include <dm.h>
#include <i2c.h>
#include <log.h>
#include <rtc.h>
#include <dm/device_compat.h>

#define PCF85063_REG_CTRL1		0x00 /* status */
#define PCF85063_REG_CTRL1_CAP_SEL	BIT(0)
#define PCF85063_REG_CTRL1_STOP		BIT(5)
#define PCF85063_REG_CTRL1_EXT_TEST	BIT(7)
#define PCF85063_REG_CTRL1_SWR		0x58 /* Software reset command */

#define PCF85063_REG_CTRL2		0x01
#define PCF85063_CTRL2_AF		BIT(6)
#define PCF85063_CTRL2_AIE		BIT(7)

#define PCF85063_REG_OFFSET		0x02
#define PCF85063_OFFSET_SIGN_BIT	6	/* 2's complement sign bit */
#define PCF85063_OFFSET_MODE		BIT(7)
#define PCF85063_OFFSET_STEP0		4340
#define PCF85063_OFFSET_STEP1		4069

#define PCF85063_REG_CLKO_F_MASK	0x07 /* frequency mask */
#define PCF85063_REG_CLKO_F_32768HZ	0x00
#define PCF85063_REG_CLKO_F_OFF		0x07

#define PCF85063_REG_RAM		0x03

#define PCF85063_REG_SC			0x04 /* datetime */
#define PCF85063_REG_SC_OS		0x80

#define PCF85063_REG_ALM_S		0x0b
#define PCF85063_AEN			BIT(7)

static int pcf85063_rtc_get(struct udevice *dev, struct rtc_time *tmp)
{
	u8 regs[7];
	int ret;

	ret = dm_i2c_read(dev, PCF85063_REG_SC, regs, sizeof(regs));
	if (ret)
		return ret;

	/* if the clock has lost its power it makes no sense to use its time */
	if (regs[0] & PCF85063_REG_SC_OS) {
		dev_warn(dev, "Power loss detected, invalid time\n");
		return -EINVAL;
	}

	tmp->tm_sec = bcd2bin(regs[0] & 0x7F);
	tmp->tm_min = bcd2bin(regs[1] & 0x7F);
	/* rtc register in 24 hour mode and rtc_time spec uses 0 .. 23 */
	tmp->tm_hour = bcd2bin(regs[2] & 0x3F);
	tmp->tm_mday = bcd2bin(regs[3] & 0x3F);
	tmp->tm_wday = regs[4] & 0x07;
	/* rtc register and rtc_time spec uses 1 .. 12 */
	tmp->tm_mon = bcd2bin(regs[5] & 0x1F);
	tmp->tm_year = bcd2bin(regs[6]);
	/* adjust register (years after 2000) to match rtc_time spec */
	tmp->tm_year += 2000;
	tmp->tm_yday = 0;
	tmp->tm_isdst = 0;

	return ret;
}

static int pcf85063_rtc_set(struct udevice *dev, const struct rtc_time *tmp)
{
	u8 regs[7];
	int ret;

	if (tmp->tm_year < 2000)
		return -EINVAL;

	/*
	 * to accurately set the time, reset the divider chain and keep it in
	 * reset state until all time/date registers are written
	 */
	ret = dm_i2c_reg_clrset(dev, PCF85063_REG_CTRL1,
				PCF85063_REG_CTRL1_EXT_TEST |
				PCF85063_REG_CTRL1_STOP,
				PCF85063_REG_CTRL1_STOP);

	if (ret)
		return ret;

	/* hours, minutes and seconds */
	regs[0] = bin2bcd(tmp->tm_sec) & 0x7F; /* clear OS flag */
	regs[1] = bin2bcd(tmp->tm_min);
	regs[2] = bin2bcd(tmp->tm_hour);
	/* Day of month, 1 - 31 */
	regs[3] = bin2bcd(tmp->tm_mday);
	/* Day, 0 - 6 */
	regs[4] = tmp->tm_wday & 0x07;
	/* rtc register and rtc_time spec uses 1 .. 12 */
	regs[5] = bin2bcd(tmp->tm_mon);
	/* adjust register (years after 2000) to match rtc_time spec */
	regs[6] = bin2bcd(tmp->tm_year - 2000);

	/* write all registers at once */
	ret = dm_i2c_write(dev, PCF85063_REG_SC,
			   regs, sizeof(regs));
	if (ret)
		return ret;

	/*
	 * Write the control register as a separate action since the size of
	 * the register space is different between the PCF85063TP and
	 * PCF85063A devices. The rollover point can not be used.
	 */
	return dm_i2c_reg_clrset(dev, PCF85063_REG_CTRL1,
				 PCF85063_REG_CTRL1_STOP, 0);

	return 0;
}

static int pcf85063_rtc_reset(struct udevice *dev)
{
	/* Send software reset, see 7.2.1.3 Software reset in data sheet */
	return dm_i2c_reg_write(dev, PCF85063_REG_CTRL1,
				PCF85063_REG_CTRL1_SWR);
}

static int pcf85063_rtc_read8(struct udevice *dev, unsigned int reg)
{
	u8 data;
	int ret;

	if (reg >= dev->driver_data)
		return -EINVAL;

	ret = dm_i2c_read(dev, reg, &data, sizeof(data));
	return ret < 0 ? ret : data;
}

static int pcf85063_rtc_write8(struct udevice *dev, unsigned int reg, int val)
{
	u8 data = val;

	if (reg >= dev->driver_data)
		return -EINVAL;

	return dm_i2c_write(dev, reg, &data, sizeof(data));
}

static int pcf85063_load_capacitance(struct udevice *dev,
				     unsigned int force_cap)
{
	u32 load = 7000;
	u8 reg = 0;

	if (force_cap)
		load = force_cap;
	else
		ofnode_read_u32(dev_ofnode(dev), "quartz-load-femtofarads", &load);

	switch (load) {
	default:
		dev_warn(dev, "Unknown quartz-load-femtofarads value: %d. Assuming 7000",
			 load);
		fallthrough;
	case 7000:
		break;
	case 12500:
		reg = PCF85063_REG_CTRL1_CAP_SEL;
		break;
	}

	return dm_i2c_reg_clrset(dev, PCF85063_REG_CTRL1,
				 PCF85063_REG_CTRL1_CAP_SEL, reg);
}

static int pcf85063_probe(struct udevice *dev)
{
	u8 tmp;
	int err;

	err = dm_i2c_read(dev, PCF85063_REG_CTRL1, &tmp, sizeof(tmp));
	if (err) {
		dev_err(dev, "RTC chip is not present\n");
		return err;
	}

	err = pcf85063_load_capacitance(dev, 7000);
	if (err < 0)
		dev_warn(dev, "failed to set xtal load capacitance: %d",
			 err);

	return 0;
}

static const struct rtc_ops pcf85063_rtc_ops = {
	.get = pcf85063_rtc_get,
	.set = pcf85063_rtc_set,
	.reset = pcf85063_rtc_reset,
	.read8 = pcf85063_rtc_read8,
	.write8 = pcf85063_rtc_write8,
};

static const struct udevice_id pcf85063_rtc_ids[] = {
	{ .compatible = "nxp,pcf85063", .data = 0xb },
	{ .compatible = "nxp,pcf85063a", .data = 0x12 },
	{ .compatible = "nxp,pcf85063tp", .data = 0xb },
	{ .compatible = "nxp,pcf85073a", .data = 0x12 },
	{ }
};

U_BOOT_DRIVER(rtc_pcf85063) = {
	.name   = "rtc-pcf85063",
	.id     = UCLASS_RTC,
	.of_match = pcf85063_rtc_ids,
	.probe	= pcf85063_probe,
	.ops    = &pcf85063_rtc_ops,
};
