// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Timo Herbrecher, Gregor Herburger
 */

#include <common.h>
#include <i2c.h>
#include <netdev.h>
#include <fdt_support.h>
#include <fm_eth.h>
#include <fsl_dtsec.h>
#include <fsl_mdio.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <linux/delay.h>
#if defined(CONFIG_FSL_LSCH3)
#include <asm/arch/immap_lsch3.h>
#elif defined(CONFIG_FSL_LSCH2)
#include <asm/arch/immap_lsch2.h>
#endif
#include "tq_mbls10xxa.h"

struct tq_gpio_init_data tq_mbls10xxa_i2c_gpios[] = {
	GPIO_INIT_DATA_ENTRY(SD1_3_LANE_A_MUX, "gpio@20_0", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(SD1_2_LANE_B_MUX, "gpio@20_1", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(SD1_0_LANE_D_MUX, "gpio@20_2", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(SD2_1_LANE_B_MUX, "gpio@20_3", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(SD2_3_LANE_D_MUX1, "gpio@20_4", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(SD2_3_LANE_D_MUX2, "gpio@20_5", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(SD_MUX_SHDN, "gpio@20_6", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(SD1_REF_CLK2_SEL, "gpio@20_7", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(MPCIE1_DISABLE, "gpio@20_8",
			     GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(MPCIE1_WAKE, "gpio@20_9", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(MPCIE2_DISABLE, "gpio@20_10",
			     GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(MPCIE2_WAKE, "gpio@20_11", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(PRSNT, "gpio@20_12", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(PCIE_PWR_EN, "gpio@20_13", GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(DCDC_PGOOD_1V8, "gpio@20_15", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(XFI1_TX_FAULT, "gpio@21_0", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(XFI1_TX_DIS, "gpio@21_1", GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(XFI1_MODDEF_DET, "gpio@21_2", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(XFI1_RX_LOSS, "gpio@21_3", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(RETIMER1_LOSS, "gpio@21_4", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(XFI1_ENSMB, "gpio@21_5", GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(QSGMII1_CLK_SEL0, "gpio@21_6", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(QSGMII_PHY1_CONFIG3, "gpio@21_7", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(XFI2_TX_FAULT, "gpio@21_8", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(XFI2_TX_DIS, "gpio@21_9", GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(XFI2_MODDEF_DET, "gpio@21_10", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(XFI2_RX_LOSS, "gpio@21_11", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(RETIMER2_LOSS, "gpio@21_12", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(XFI2_ENSMB, "gpio@21_13", GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(QSGMII2_CLK_SEL0, "gpio@21_14", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(QSGMII_PHY2_CONFIG3, "gpio@21_15", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(EC1_PHY_PWDN, "gpio@22_0", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(EC2_PHY_PWDN, "gpio@22_1", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(USB_C_PWRON, "gpio@22_2", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(USB_EN_OC_3V3, "gpio@22_3", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(USB_H_GRST, "gpio@22_4", GPIOD_IS_OUT | GPIOD_ACTIVE_LOW),
	GPIO_INIT_DATA_ENTRY(GPIO_BUTTON0, "gpio@22_5", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(GPIO_BUTTON1, "gpio@22_6", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(SDA_PWR_EN, "gpio@22_7", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(QSGMII_PHY1_INT, "gpio@22_8", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(QSGMII_PHY2_INT, "gpio@22_9", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(SPI_CLKO_SOF, "gpio@22_10", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(SPI_INT, "gpio@22_11", GPIOD_IS_IN),
	GPIO_INIT_DATA_ENTRY(CAN_SEL, "gpio@22_12", GPIOD_IS_OUT),
	GPIO_INIT_DATA_ENTRY(LED, "gpio@22_13", GPIOD_IS_OUT | GPIOD_ACTIVE_LOW),
	GPIO_INIT_DATA_ENTRY(PCIE_RST_3V3, "gpio@22_14",
			     GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE),
	GPIO_INIT_DATA_ENTRY(PCIE_WAKE_3V3, "gpio@22_15", GPIOD_IS_IN),
};

const char *tq_bb_get_boardname(void)
{
	return "MBLS10xxA";
}

int tq_mbls10xxa_i2c_gpios_init(void)
{
	return tq_board_gpio_init(tq_mbls10xxa_i2c_gpios, ARRAY_SIZE(tq_mbls10xxa_i2c_gpios));
}

int tq_mbls10xxa_i2c_gpio_get(enum tq_mbls10xxa_gpios gpio)
{
	struct gpio_desc *desc = &tq_mbls10xxa_i2c_gpios[gpio].desc;

	return dm_gpio_get_value(desc);
}

int tq_mbls10xxa_i2c_gpio_set(enum tq_mbls10xxa_gpios gpio, int val)
{
	struct gpio_desc *desc = &tq_mbls10xxa_i2c_gpios[gpio].desc;

	return dm_gpio_set_value(desc, val);
}

int tq_mbls10xxa_clk_cfg_init(void)
{
	struct udevice *dev;
	int ret = 0;
	u8 buf[4];
	int i;

	i2c_get_chip_for_busnum(TQ_MBLS10XXA_I2C_CLKBUF_BUS_NUM, TQ_MBLS10XXA_I2C_CLKBUF_ADDR, 1,
				&dev);

	/* set LVDS output on channels 4 and 5 in all configurations */
	for (i = 0; i < 4; i++)
		buf[i] = 0xC0;
	ret = dm_i2c_write(dev, 12, buf, 4);
	if (ret != 0)
		printf("unable to set clock buffer\n");

	return ret;
}

static int tq_mbls10xxa_retimer_init_one(struct udevice *dev)
{
	struct {
		u8 reg;
		u8 value;
	} const cfg[] = {
		/* Write all channels */
		{ 0xff, 0x0c },
		/* Assert CDR Reset */
		{ 0x0a, 0x0c },
		/* Write channel A */
		{ 0xff, 0x04 },
		/* Output Voltage 900mV */
		{ 0x2d, 0x03 },
		/* De-emphasis -4.5dB */
		{ 0x15, 0x45 },
		/* Write channel B B */
		{ 0xff, 0x05 },
		/* Output Voltage 1200mV */
		{ 0x2d, 0x06 },
		/* De-emphasis -2dB */
		{ 0x15, 0x42 },
		/* Write all channels */
		{ 0xff, 0x0c },
		/* CDR Reset Release */
		{ 0x0a, 0x00 },
	};
	int ret, i;
	u8 v;

	for (i = 0; i < ARRAY_SIZE(cfg); i++) {
		v = cfg[i].value;
		ret = dm_i2c_write(dev, cfg[i].reg, &v, sizeof(v));
		if (ret != 0)
			return ret;
	}

	return 0;
}

void tq_mbls10xxa_retimer_init(void)
{
	const u8 addrs[] = TQ_MBLS10XXA_I2C_RETIMER_ADDRS;
	struct udevice *dev;
	int ret;
	int i;

	for (i = 0; i < ARRAY_SIZE(addrs); i++) {
		ret = i2c_get_chip_for_busnum(TQ_MBLS10XXA_I2C_RETIMER_BUS_NUM, addrs[i], 1, &dev);
		if (ret)
			printf("Error finding dev 0x%x: %d\n", addrs[i], ret);
		ret = tq_mbls10xxa_retimer_init_one(dev);
		if (ret)
			printf("Error configuring retimer dev 0x%x: %d\n", addrs[i], ret);
	}
}

void tq_mbls10xxa_xfi_init(struct ccsr_serdes *serdes, int lane)
{
	u32 gcr0;

	/* Put lane in reset */
	gcr0 = serdes_in32(&serdes->lane[lane].gcr0);
	gcr0 &= 0xFF9FFFFF;
	serdes_out32(&serdes->lane[lane].gcr0, gcr0);

	udelay(1);

	/* Set up Transmit Equalization Control Register 0 for XFI */
#if defined(CONFIG_FSL_LSCH3)
	serdes_out32(&serdes->lane[lane].tec0,  0x00233827);
#elif defined(CONFIG_FSL_LSCH2)
	serdes_out32(&serdes->lane[lane].tecr0, 0x00233827);
#endif

	udelay(1);

	/* Take lane out of reset */
	gcr0 |= 0x00600000;
	serdes_out32(&serdes->lane[lane].gcr0, gcr0);
}

static char *_srds_proto_name(enum tq_mbls10xxa_srds_proto proto)
{
	switch (proto) {
	case TQ_MBLS10XXA_SRDS_PROTO_UNUSED:
		return "Unused    ";
	case TQ_MBLS10XXA_SRDS_PROTO_SGMII:
		return "SGMII     ";
	case TQ_MBLS10XXA_SRDS_PROTO_SGMII2G5:
		return "2.5G SGMII";
	case TQ_MBLS10XXA_SRDS_PROTO_QSGMII:
		return "QSGMII    ";
	case TQ_MBLS10XXA_SRDS_PROTO_XFI:
		return "XFI       ";
	case TQ_MBLS10XXA_SRDS_PROTO_PCIEX1:
		return "PCIe x1   ";
	case TQ_MBLS10XXA_SRDS_PROTO_PCIEX2:
		return "PCIe x2   ";
	case TQ_MBLS10XXA_SRDS_PROTO_PCIEX4:
		return "PCIe x4   ";
	case TQ_MBLS10XXA_SRDS_PROTO_SATA:
		return "SATA      ";
	default:
		return "Unknown   ";
	}
}

static int _srds_mux_check_and_print(int port, int lane, int mux,
				     enum tq_mbls10xxa_srds_proto proto)
{
	int stat = 0;

	if (proto != TQ_MBLS10XXA_SRDS_PROTO_UNUSED &&
	    proto != mux)
		stat++;

	printf("  SD%d-%d: MUX=%s | RCW=%s -> %s\n", port, lane, _srds_proto_name(mux),
	       _srds_proto_name(proto), (stat) ? ("FAIL") : ("OK"));

	return stat;
}

int tq_mbls10xxa_serdes_init(struct tq_mbls10xxa_serdes lanes[8])
{
	int idx = 0;
	int mux_val1, mux_val2;
	int mux_proto;
	int err_cnt = 0;

	printf("Checking MBLS10xxA SerDes muxing:\n");

	/* check state of SD_MUX_SHDN */
	mux_val1 = tq_mbls10xxa_i2c_gpio_get(SD_MUX_SHDN);
	if (mux_val1) {
		printf("!!! ATTENTION: SerDes MUXes disabled,\n");
		printf("!!!  muxed SerDes interfaces won't work\n");
	}

	/* check config for SD1_0 */
	mux_val1 = tq_mbls10xxa_i2c_gpio_get(SD1_0_LANE_D_MUX);
	if (mux_val1 >= 0 && lanes[idx].port != 0) {
		mux_proto = mux_val1 ? TQ_MBLS10XXA_SRDS_PROTO_XFI : TQ_MBLS10XXA_SRDS_PROTO_SGMII;
		err_cnt += _srds_mux_check_and_print(lanes[idx].port, lanes[idx].lane, mux_proto,
						     lanes[idx].proto);
	}
	idx++;

	/* check config for SD1_1 */
	if (lanes[idx].port != 0) {
		mux_proto = TQ_MBLS10XXA_SRDS_PROTO_XFI;
		err_cnt += _srds_mux_check_and_print(lanes[idx].port, lanes[idx].lane, mux_proto,
						     lanes[idx].proto);
	}
	idx++;

	/* check config for SD1_2 */
	mux_val1 = tq_mbls10xxa_i2c_gpio_get(SD1_2_LANE_B_MUX);
	if (mux_val1 >= 0 && lanes[idx].port != 0) {
		mux_proto = mux_val1 ? TQ_MBLS10XXA_SRDS_PROTO_QSGMII :
				       TQ_MBLS10XXA_SRDS_PROTO_SGMII;
		err_cnt += _srds_mux_check_and_print(lanes[idx].port, lanes[idx].lane, mux_proto,
						     lanes[idx].proto);
	}
	idx++;

	/* check config for SD1_3 */
	mux_val1 = tq_mbls10xxa_i2c_gpio_get(SD1_3_LANE_A_MUX);
	if (mux_val1 >= 0 && lanes[idx].port != 0) {
		mux_proto = mux_val1 ? TQ_MBLS10XXA_SRDS_PROTO_QSGMII :
				       TQ_MBLS10XXA_SRDS_PROTO_SGMII;
		err_cnt += _srds_mux_check_and_print(lanes[idx].port, lanes[idx].lane, mux_proto,
						     lanes[idx].proto);
	}
	idx++;

	/* check config for SD2_0 */
	if (lanes[idx].port != 0) {
		mux_proto = TQ_MBLS10XXA_SRDS_PROTO_PCIEX1;
		err_cnt += _srds_mux_check_and_print(lanes[idx].port, lanes[idx].lane, mux_proto,
						     lanes[idx].proto);
	}
	idx++;

	/* check config for SD2_1 */
	mux_val1 = tq_mbls10xxa_i2c_gpio_get(SD2_1_LANE_B_MUX);
	if (mux_val1 >= 0 && lanes[idx].port != 0) {
		mux_proto = mux_val1 ? TQ_MBLS10XXA_SRDS_PROTO_SGMII :
				       TQ_MBLS10XXA_SRDS_PROTO_PCIEX1;
		err_cnt += _srds_mux_check_and_print(lanes[idx].port, lanes[idx].lane, mux_proto,
						     lanes[idx].proto);

		/* enable miniPCIe-Slot when selected */
		if (lanes[idx].proto == TQ_MBLS10XXA_SRDS_PROTO_PCIEX1 &&
		    mux_proto == lanes[idx].proto)
			tq_mbls10xxa_i2c_gpio_set(MPCIE2_DISABLE, 0);
		else
			tq_mbls10xxa_i2c_gpio_set(MPCIE2_DISABLE, 1);
	}
	idx++;

	/* check config for SD2_2 */
	if (lanes[idx].port != 0) {
		mux_proto = TQ_MBLS10XXA_SRDS_PROTO_PCIEX1;
		if (lanes[idx].proto == TQ_MBLS10XXA_SRDS_PROTO_PCIEX2)
			mux_proto = TQ_MBLS10XXA_SRDS_PROTO_PCIEX2;
		err_cnt += _srds_mux_check_and_print(lanes[idx].port, lanes[idx].lane, mux_proto,
						     lanes[idx].proto);

		/* get M.2 slot out of reset when lane (SD2-2) is configured as PCIe */
		if ((lanes[idx].proto == TQ_MBLS10XXA_SRDS_PROTO_PCIEX1 ||
		     lanes[idx].proto == TQ_MBLS10XXA_SRDS_PROTO_PCIEX2) &&
		    mux_proto == lanes[idx].proto)
			tq_mbls10xxa_i2c_gpio_set(PCIE_RST_3V3, 0);
		else
			tq_mbls10xxa_i2c_gpio_set(PCIE_RST_3V3, 1);
	}
	idx++;

	/* check config for SD2 - LANE D */
	mux_val1 = tq_mbls10xxa_i2c_gpio_get(SD2_3_LANE_D_MUX1);
	mux_val2 = tq_mbls10xxa_i2c_gpio_get(SD2_3_LANE_D_MUX2);
	if (mux_val1 >= 0 && mux_val2 >= 0 && lanes[idx].port != 0) {
		mux_proto = mux_val2 ? TQ_MBLS10XXA_SRDS_PROTO_PCIEX1 :
				       TQ_MBLS10XXA_SRDS_PROTO_SATA;
		mux_proto = mux_val1 ? mux_proto : TQ_MBLS10XXA_SRDS_PROTO_PCIEX2;
		err_cnt += _srds_mux_check_and_print(lanes[idx].port, lanes[idx].lane, mux_proto,
						     lanes[idx].proto);

		/* enable miniPCIe-Slot when selected */
		if (lanes[idx].proto == TQ_MBLS10XXA_SRDS_PROTO_PCIEX1 &&
		    mux_proto == lanes[idx].proto)
			tq_mbls10xxa_i2c_gpio_set(MPCIE1_DISABLE, 0);
		else
			tq_mbls10xxa_i2c_gpio_set(MPCIE1_DISABLE, 1);
	}
	idx++;

	/* print error message when muxing is invalid */
	if (err_cnt) {
		printf("!!! ATTENTION: Some SerDes lanes are misconfigured,\n");
		printf("!!!  this may cause some interfaces to be inoperable.\n");
		printf("!!!  Check SerDes muxing DIP switch settings!\n");
	}

	return 0;
}

int tq_mbls10xxa_serdes_clk_get(enum tq_mbls10xxa_srds_clk clk)
{
	int freq = -1;
	int mux_val;

	switch (clk) {
	case TQ_MBLS10XXA_SRDS_CLK_1_1:
	case TQ_MBLS10XXA_SRDS_CLK_2_1:
	case TQ_MBLS10XXA_SRDS_CLK_2_2:
		/* fixed serdes clock (125MHz) */
		freq = 125000000;
		break;
	case TQ_MBLS10XXA_SRDS_CLK_1_2:
		/* get clock mux */
		mux_val = tq_mbls10xxa_i2c_gpio_get(SD1_REF_CLK2_SEL);
		if (mux_val >= 0) {
			if (mux_val)
				freq = 125000000;
			else
				freq = 156250000;
		}
		break;
	default:
		break;
	}

	return freq;
}

static int qsgmii_phy_initialization(struct phy_device *phydev)
{
	int ret = 0;

	/* execute marvell specified initialization */
	/* see MV-S301615 release note */
	/* PHY initialization #1 */
	ret = phy_write(phydev, MDIO_DEVAD_NONE, 0x16, 0x00ff);
	if (ret)
		return ret;
	ret = phy_write(phydev, MDIO_DEVAD_NONE, 0x18, 0x2800);
	if (ret)
		return ret;
	ret = phy_write(phydev, MDIO_DEVAD_NONE, 0x17, 0x2001);
	if (ret)
		return ret;
	ret = phy_write(phydev, MDIO_DEVAD_NONE, 0x16, 0x0000);
	if (ret)
		return ret;

	/* PHY initialization #2 */
	ret = phy_write(phydev, MDIO_DEVAD_NONE, 0x16, 0x0000);
	if (ret)
		return ret;
	ret = phy_write(phydev, MDIO_DEVAD_NONE, 0x1D, 0x0003);
	if (ret)
		return ret;
	ret = phy_write(phydev, MDIO_DEVAD_NONE, 0x1E, 0x0002);
	if (ret)
		return ret;
	ret = phy_write(phydev, MDIO_DEVAD_NONE, 0x16, 0x0000);
	if (ret)
		return ret;

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	u16 val;
	static bool qsgmii1_initdone;
	static bool qsgmii2_initdone;
	int ret = 0;

	if (phydev->drv->config)
		ret = phydev->drv->config(phydev);

	if (ret < 0)
		printf("Failed to configure Phy 0x%02x.\n", phydev->addr);

	/* no board specific work needed, skip */
	if ((phydev->addr & 0x1C) != QSGMII_PHY1_ADDR_BASE &&
	    (phydev->addr & 0x1C) != QSGMII_PHY2_ADDR_BASE)
		return 0;

	/* when initialization already done, skip */
	if ((phydev->addr & 0x1C) == QSGMII_PHY1_ADDR_BASE && qsgmii1_initdone)
		return 0;
	if ((phydev->addr & 0x1C) == QSGMII_PHY2_ADDR_BASE && qsgmii2_initdone)
		return 0;

	/* reset PHY because clock input may have changed */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x16, 0x0004);
	ret = phy_write(phydev, MDIO_DEVAD_NONE, 0x1B, 0x8000);
	if (ret) {
		printf("Failed to reset Phy 0x%02x.\n", phydev->addr);
		goto out;
	}
	/* wait for 50ms for reset to complete */
	mdelay(50);

	ret = qsgmii_phy_initialization(phydev);
	if (ret) {
		printf("Failed to initialize Phy 0x%02x.\n", phydev->addr);
		goto out;
	}

	/* check configuration of PHY device */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x16, 0x0006);
	val = phy_read(phydev, MDIO_DEVAD_NONE, 0x14);

	if (val < 0) {
		printf("Failed to check Phy 0x%02x config.\n", phydev->addr);
		goto out;
	}

	printf("QSGMII ethernet PHY 0x%02x configuration: ", phydev->addr);
	if ((val & 0x7) == 0)
		printf("QSGMII to Copper\n");
	else if ((val & 0x7) == 1)
		printf("SGMII to Copper\n");
	else
		printf("unsupported\n");

	/* mark PHY as initialized */
	if ((phydev->addr & 0x1C) == QSGMII_PHY1_ADDR_BASE)
		qsgmii1_initdone = true;
	if ((phydev->addr & 0x1C) == QSGMII_PHY2_ADDR_BASE)
		qsgmii2_initdone = true;

out:
	/* Set page back to 0x0000 */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x16, 0x0000);
	return ret;
}

int tq_mbls10xxa_fixup_phy_to_enet(void *fdt, char *enet_alias, char *phy_alias, char *connection)
{
	const char *path;
	u32 phy_phandle;
	int offset;
	int ret;

	ret = fdt_increase_size(fdt, 32);
	if (ret)
		return ret;

	path = fdt_get_alias(fdt, phy_alias);
	if (!path)
		return -FDT_ERR_BADPATH;

	offset = fdt_path_offset(fdt, path);
	if (offset < 0)
		return offset;

	phy_phandle = fdt_create_phandle(fdt, offset);

	path = fdt_get_alias(fdt, enet_alias);
	if (!path)
		return -FDT_ERR_BADPATH;

	offset = fdt_path_offset(fdt, path);
	if (offset < 0)
		return offset;

	phy_phandle = cpu_to_fdt32(phy_phandle);
	ret = fdt_setprop(fdt, offset, "phy-handle", &phy_phandle, sizeof(phy_phandle));

	if (ret)
		return ret;

	ret = fdt_setprop_string(fdt, offset, "phy-connection-type", connection);

	return fdt_status_okay_by_alias(fdt, enet_alias);
}

int tq_mbls10xxa_fixup_enet_fixed_link(void *fdt, char *enet_alias, int id, char *connection)
{
	struct fixed_link f_link;
	const char *path;
	int offset;
	int ret;

	ret = fdt_increase_size(fdt, 64);
	if (ret)
		return ret;

	path = fdt_get_alias(fdt, enet_alias);
	if (!path)
		return -FDT_ERR_BADPATH;

	offset = fdt_path_offset(fdt, path);
	if (offset < 0)
		return offset;

	f_link.phy_id = cpu_to_fdt32(id);
	f_link.duplex = cpu_to_fdt32(1);
	f_link.link_speed = cpu_to_fdt32(1000);
	f_link.pause = 0;
	f_link.asym_pause = 0;
	fdt_setprop(fdt, offset, "fixed-link", &f_link, sizeof(f_link));
	fdt_setprop_string(fdt, offset, "phy-connection-type", connection);

	return fdt_status_okay_by_alias(fdt, enet_alias);
}
