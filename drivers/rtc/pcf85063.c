/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Date & Time support for Philips PCF8563 RTC
 */

/* #define	DEBUG	*/

#include <common.h>
#include <command.h>
#include <rtc.h>
#include <i2c.h>

#if defined(CONFIG_CMD_DATE)

#define PCF85063_CTRL1_REG 0x00
#define PCF85063_CTRL2_REG 0x01

#define PCF85063_SC_REG 0x04
#define PCF85063_MN_REG 0x05
#define PCF85063_HR_REG 0x06
#define PCF85063_DM_REG 0x07
#define PCF85063_DW_REG 0x08
#define PCF85063_MO_REG 0x09
#define PCF85063_YR_REG 0x0a

#define PCF85063_SC_ALARM_REG  0x0b
#define PCF85063_MN_ALARM_REG  0x0c
#define PCF85063_HR_ALARM_REG  0x0d
#define PCF85063_DM_ALARM_REG  0x0e
#define PCF85063_DW_ALARM_REG  0x0f

static uchar rtc_read(uchar reg);
static void  rtc_write(uchar reg, uchar val);

/* ------------------------------------------------------------------------- */

int rtc_get(struct rtc_time *tmp)
{
	int rel = 0;
	uchar sec, min, hour, mday, wday, mon_cent, year;

	sec	 = rtc_read(PCF85063_SC_REG);
	min	 = rtc_read(PCF85063_MN_REG);
	hour	 = rtc_read(PCF85063_HR_REG);
	mday	 = rtc_read(PCF85063_DM_REG);
	wday	 = rtc_read(PCF85063_DW_REG);
	mon_cent = rtc_read(PCF85063_MO_REG);
	year	 = rtc_read(PCF85063_YR_REG);

	debug("Get RTC year: %02x mon/cent: %02x mday: %02x wday: %02x hr: %02x min: %02x sec: %02x\n"
	      , year, mon_cent, mday, wday, hour, min, sec);
	debug("Alarms: wday: %02x day: %02x hour: %02x min: %02x\n",
	      rtc_read(PCF85063_DW_ALARM_REG),
	      rtc_read(PCF85063_DM_ALARM_REG),
	      rtc_read(PCF85063_HR_ALARM_REG),
	      rtc_read(PCF85063_MN_ALARM_REG));

	if (sec & 0x80) {
		puts("### Warning: RTC Low Voltage -date/time not reliable\n");
		rel = -1;
	}

	tmp->tm_sec   = bcd2bin(sec  & 0x7F);
	tmp->tm_min   = bcd2bin(min  & 0x7F);
	tmp->tm_hour  = bcd2bin(hour & 0x3F);
	tmp->tm_mday  = bcd2bin(mday & 0x3F);
	tmp->tm_mon   = bcd2bin(mon_cent & 0x0F);
	tmp->tm_year  = bcd2bin(year) + 2000;
	tmp->tm_wday  = bcd2bin(wday & 0x07);
	tmp->tm_yday  = 0;
	tmp->tm_isdst = 0;

	debug("Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	      tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
	      tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	return rel;
}

int rtc_set(struct rtc_time *tmp)
{
	uchar century;

	debug("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	      tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
	      tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	rtc_write(PCF85063_YR_REG, bin2bcd(tmp->tm_year % 100));

	rtc_write(PCF85063_MO_REG, bin2bcd(tmp->tm_mon));

	rtc_write(PCF85063_DW_REG, bin2bcd(tmp->tm_wday));
	rtc_write(PCF85063_DM_REG, bin2bcd(tmp->tm_mday));
	rtc_write(PCF85063_HR_REG, bin2bcd(tmp->tm_hour));
	rtc_write(PCF85063_MN_REG, bin2bcd(tmp->tm_min));
	rtc_write(PCF85063_SC_REG, bin2bcd(tmp->tm_sec));

	return 0;
}

void rtc_reset(void)
{
	/* clear all control & status registers */
	rtc_write(PCF85063_CTRL1_REG, 0x01);
	rtc_write(PCF85063_CTRL2_REG, 0x00);

	/* clear Voltage Low bit */
	rtc_write(PCF85063_SC_REG, rtc_read(PCF85063_SC_REG) & 0x7F);

	/* reset all alarms */
	rtc_write(PCF85063_SC_ALARM_REG, 0x00);
	rtc_write(PCF85063_MN_ALARM_REG, 0x00);
	rtc_write(PCF85063_HR_ALARM_REG, 0x00);
	rtc_write(PCF85063_DM_ALARM_REG, 0x00);
	rtc_write(PCF85063_DW_ALARM_REG, 0x00);
}

/* ------------------------------------------------------------------------- */

static uchar rtc_read(uchar reg)
{
	return i2c_reg_read(CONFIG_SYS_I2C_RTC_ADDR, reg);
}

static void rtc_write(uchar reg, uchar val)
{
	i2c_reg_write(CONFIG_SYS_I2C_RTC_ADDR, reg, val);
}

#endif
