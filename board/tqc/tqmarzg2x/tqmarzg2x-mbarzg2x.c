// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>, D-82229 Seefeld, Germany.
 * Author: Paul Gerber
 */

#include <common.h>
#include <env.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/processor.h>
#include <asm/arch/rmobile.h>
#include <asm/arch/rcar-mstp.h>
#include <i2c.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/libfdt.h>
#include <miiphy.h>
#include <renesas_wdt.h>

#include "../common/tqc_eeprom.h"
#include "../../renesas/rzg-common/common.h"

#define RST_BASE	0xE6160000
#define RST_CA57RESCNT	(RST_BASE + 0x40)
#define RST_CA53RESCNT	(RST_BASE + 0x44)
#define RST_CA57_CODE	0xA5A5000F
#define RST_CA53_CODE	0x5A5A000F

DECLARE_GLOBAL_DATA_PTR;

#define DVFS_MSTP926		BIT(26)
#define GPIO2_MSTP910		BIT(10)
#define GPIO3_MSTP909		BIT(9)
#define GPIO5_MSTP907		BIT(7)
#define HSUSB_MSTP704		BIT(4)	/* HSUSB */

int board_early_init_f(void)
{
	if (IS_ENABLED(CONFIG_SYS_I2C) && IS_ENABLED(CONFIG_SYS_I2C_SH)) {
		/* DVFS for reset */
		mstp_clrbits_le32(SMSTPCR9, SMSTPCR9, DVFS_MSTP926);
	}

	return 0;
}

#define HSUSB_MSTP704		BIT(4)	/* HSUSB */

/* HSUSB block registers */
#define HSUSB_REG_LPSTS			0xE6590102
#define HSUSB_REG_LPSTS_SUSPM_NORMAL	BIT(14)
#define HSUSB_REG_UGCTRL2		0xE6590184
#define HSUSB_REG_UGCTRL2_USB0SEL_EHCI	0x10
#define HSUSB_REG_UGCTRL2_RESERVED_3	0x1 /* bit[3:0] should be B'0001 */

#define	GPIO_USB_HUB_RST	142	/* GP6_22	*/
#define	GPIO_PCIE1_SATA_SEL	25	/* GP1_09	*/
#define	GPIO_M2_PEDET		143	/* GP6_23	*/
#define	GPIO_PCIE0_RST		1	/* GP0_01	*/
#define	GPIO_PCIE0_DISABLE	2	/* GP0_02	*/
#define	GPIO_PCIE1_RST		146	/* GP6_26	*/

#define RTC_IIC_BUS_NUM		7
#define RTC_IIC_ADDR		0x51
#define RTC_CAP_SEL		1	/* 0=7pF 1=12.5pF */

#define CLOCKGEN_I2C_BUS_NUM	4
#define CLOCKGEN_I2C_ADDR	0x6A

static int rtc_cap_sel(void)
{
	struct udevice dev;
	struct udevice *pdev = &dev;
	u8 rtc_reg;
	int ret;

	ret = i2c_get_chip_for_busnum(RTC_IIC_BUS_NUM, RTC_IIC_ADDR, 1, &pdev);

	if (ret)
		return ret;

	ret = dm_i2c_read(pdev, 0, &rtc_reg, 1);

	if (ret)
		printf("Error reading from rtc!\n");

	rtc_reg |= RTC_CAP_SEL;

	ret = dm_i2c_write(pdev, 0, &rtc_reg,  1);

	if (ret)
		printf("Error setting capacitance of RTC!\n");

	return 0;
}

static int clockgen_init(void)
{
	struct udevice dev;
	struct udevice *pdev = &dev;
	int ret;

	ret = i2c_get_chip_for_busnum(CLOCKGEN_I2C_BUS_NUM, CLOCKGEN_I2C_ADDR, 1, &pdev);

	if (ret)
		return ret;

	u8 regvals[][2] = {
		/* Start configuration preamble */
		/*    Set device in Ready mode */
		{ 0x06, 0x01 },
		/* End configuration preamble */

		/* Start configuration registers */
		{ 0x17, 0x00 },
		{ 0x18, 0x00 },
		{ 0x19, 0x00 },
		{ 0x1A, 0x00 },
		{ 0x1B, 0x00 },
		{ 0x1C, 0x00 },
		{ 0x21, 0x34 },
		{ 0x24, 0x01 },
		{ 0x25, 0x10 },
		{ 0x26, 0x00 },
		{ 0x27, 0x00 },
		{ 0x28, 0x60 },
		{ 0x2A, 0x50 },
		{ 0x2B, 0x19 },
		{ 0x2D, 0x14 },
		{ 0x36, 0x32 },
		{ 0x37, 0xDC },
		{ 0x38, 0x02 },
		{ 0x39, 0x80 },
		{ 0x3A, 0x03 },
		{ 0x3B, 0x00 },
		{ 0x3C, 0x00 },
		{ 0x48, 0x00 },
		{ 0x4E, 0x25 },
		{ 0x4F, 0xE0 },
		{ 0x50, 0x00 },
		{ 0x51, 0x20 },
		{ 0x52, 0x00 },
		{ 0x53, 0x21 },
		{ 0x54, 0x00 },
		{ 0x60, 0x00 },
		{ 0x67, 0x19 },
		{ 0x68, 0x00 },
		{ 0x69, 0x00 },
		{ 0x6A, 0x00 },
		{ 0x6B, 0x00 },
		{ 0x6C, 0x01 },
		{ 0x73, 0x00 },
		{ 0x74, 0x00 },
		{ 0x75, 0x01 },
		{ 0x7A, 0x01 },
		{ 0x7B, 0x01 },
		{ 0x7C, 0x00 },
		{ 0x7D, 0x00 },
		{ 0x7E, 0x00 },
		{ 0x7F, 0x09 },
		{ 0x80, 0x01 },
		{ 0x81, 0x00 },
		{ 0x82, 0x00 },
		{ 0x84, 0x09 },
		{ 0x85, 0x01 },
		{ 0x86, 0x00 },
		{ 0x87, 0x00 },
		{ 0x89, 0x09 },
		{ 0x8A, 0x01 },
		{ 0x8B, 0x00 },
		{ 0x8C, 0x00 },
		{ 0x8E, 0x09 },
		{ 0x8F, 0x01 },
		{ 0x90, 0x00 },
		{ 0x91, 0x00 },
		{ 0x93, 0x09 },
		{ 0x94, 0x01 },
		{ 0x95, 0x00 },
		{ 0x96, 0x00 },
		{ 0x98, 0x01 },
		{ 0x99, 0x01 },
		{ 0x9A, 0x00 },
		{ 0x9B, 0x00 },
		{ 0x9C, 0x00 },
		{ 0x9D, 0x01 },
		{ 0x9E, 0x01 },
		{ 0x9F, 0x00 },
		{ 0xA0, 0x00 },
		{ 0xA1, 0x00 },
		{ 0xA2, 0x01 },
		{ 0xA3, 0x01 },
		{ 0xA4, 0x00 },
		{ 0xA5, 0x00 },
		{ 0xA6, 0x00 },
		{ 0xA7, 0x07 },
		{ 0xA9, 0x00 },
		{ 0xAA, 0x00 },
		{ 0xAC, 0x01 },
		{ 0xAD, 0x02 },
		{ 0xAE, 0x00 },
		{ 0xAF, 0x00 },
		{ 0xB0, 0x00 },
		{ 0xB1, 0x01 },
		{ 0xB2, 0x01 },
		{ 0xB3, 0x00 },
		{ 0xB4, 0x00 },
		{ 0xB5, 0x00 },
		{ 0xB6, 0xDF },	/* this is the place to enable OUT5 */
		{ 0xB7, 0x0D },
		{ 0xB9, 0x06 },
		{ 0xBA, 0x1C },
		{ 0xBB, 0x10 },
		{ 0xBC, 0x00 },
		{ 0xBD, 0x02 },
		{ 0xBE, 0x20 },
		/* End configuration registers */

		/* Start configuration postamble */
		/*    Set device in Active mode */
		{ 0x06, 0x02 }
		/* End configuration postamble */
	};

	/*
	 * Read GPIO_PCIE1_SATA_SEL and GPIO_M2_PEDET.
	 * If both are high, patch the configuration data stream so that OUT5 is enabled.
	 */
	if (gpio_get_value(GPIO_PCIE1_SATA_SEL) && gpio_get_value(GPIO_M2_PEDET))
		regvals[94][1] = 0xFF;

	for (int i = 0; i < ARRAY_SIZE(regvals); i++) {
		ret = dm_i2c_write(pdev, regvals[i][0], &regvals[i][1], 1);
		if (ret) {
			printf("Error writing to clock generator!\n");
			return 0;
		}
	}

	mdelay(10); /* allow some time for clock to stabilize */
	return 0;
}

static const char *tqmarzg2x_get_boardname(void)
{
	if (IS_ENABLED(CONFIG_TQMARZG2N))
		return "TQMaRZG2N";
	if (IS_ENABLED(CONFIG_TQMARZG2M))
		return "TQMaRZG2M";
	if (IS_ENABLED(CONFIG_TQMARZG2H))
		return "TQMaRZG2H";

	return "UNKNOWN";
}

int board_init(void)
{
	u32 i;

	/* address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_TEXT_BASE + 0x50000;

	/* Configure the HSUSB block */
	mstp_clrbits_le32(SMSTPCR7, SMSTPCR7, HSUSB_MSTP704);
	/*
	 * We need to add a barrier instruction after HSUSB module stop release.
	 * This barrier instruction can be either reading back the same MSTP
	 * register or any other register in the same IP block. So like linux
	 * adding check for MSTPSR register, which indicates the clock has been
	 * started.
	 */
	for (i = 1000; i > 0; --i) {
		if (!(readl(MSTPSR7) & HSUSB_MSTP704))
			break;
		cpu_relax();
	}

	/* Select EHCI/OHCI host module for USB2.0 ch0 */
	writel(HSUSB_REG_UGCTRL2_USB0SEL_EHCI | HSUSB_REG_UGCTRL2_RESERVED_3,
	       HSUSB_REG_UGCTRL2);
	/* low power status */
	setbits_le16(HSUSB_REG_LPSTS, HSUSB_REG_LPSTS_SUSPM_NORMAL);

	/* USB1 pull-up */
	setbits_le32(PFC_PUEN6, PUEN_USB1_OVC | PUEN_USB1_PWEN);

	/* MBaRZG2x: set GP1_09 as input to allow DIP switch S10.1 control PCIE1_SATA_SEL */
	gpio_request(GPIO_PCIE1_SATA_SEL, "pcie1_sata_sel");
	gpio_direction_input(GPIO_PCIE1_SATA_SEL);

	/* MBaRZG2x: set GP6_23 as input needed in combination with GP1_09 for clock setup */
	gpio_request(GPIO_M2_PEDET, "m2_pedet");
	gpio_direction_input(GPIO_M2_PEDET);

	/* MBaRZG2x: force PCIex reset */
	gpio_request(GPIO_PCIE0_RST, "pcie0_rst");
	gpio_request(GPIO_PCIE1_RST, "pcie1_rst");
	gpio_direction_output(GPIO_PCIE0_RST, 0);
	gpio_direction_output(GPIO_PCIE1_RST, 0);

	clockgen_init();

	/* select RTC oscillator capacitance */
	rtc_cap_sel();

	/* MBaRZG2x: release USB hub reset */
	gpio_request(GPIO_USB_HUB_RST, "usb_hub_rst");
	gpio_direction_output(GPIO_USB_HUB_RST, 1);

	/* MBaRZG2x: release PCIex reset */
	gpio_set_value(GPIO_PCIE0_RST, 1);
	gpio_set_value(GPIO_PCIE1_RST, 1);

	/* MBaRZG2x: deactivate PCIe0 disable */
	gpio_request(GPIO_PCIE0_DISABLE, "pcie0_disable");
	gpio_direction_output(GPIO_PCIE0_DISABLE, 1);

	return 0;
}

void reset_cpu(void)
{
	unsigned long midr, cputype;

	asm volatile("mrs %0, midr_el1" : "=r" (midr));
	cputype = (midr >> 4) & 0xfff;

	if (cputype == 0xd03)
		writel(RST_CA53_CODE, RST_CA53RESCNT);
	else
		writel(RST_CA57_CODE, RST_CA57RESCNT);
}

int board_late_init(void)
{
	if (IS_ENABLED(CONFIG_WDT_RENESAS))
		reinitr_wdt();

	if (!IS_ENABLED(CONFIG_SPL_BUILD)) {
		struct tqc_eeprom_data eeprom;
		const char *bname = tqmarzg2x_get_boardname();

		if (!tqc_read_eeprom_at(7, 0x57, 0x100, 0x0, &eeprom))
			tqc_show_eeprom(&eeprom, bname);
		else
			puts("EEPROM: read error\n");
	}

	return 0;
}
