// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 TQ-Systems GmbH
 */
#include <common.h>
#include <i2c.h>
#include <netdev.h>
#include <fm_eth.h>
#include <fsl_dtsec.h>
#include <fsl_mdio.h>
#include <asm/io.h>
#include <asm/arch/immap_lsch2.h>
#include "tqc_mbls10xxa.h"

struct tqc_mbls10xxa_i2c_gpio {
	char name[20];
	uint8_t addr;
	uint8_t nr;
	uint8_t dir;
	uint8_t dval;
};

struct tqc_mbls10xxa_i2c_gpio tqc_mbls10xxa_i2c_gpios[] =
{
	{"sd1_3_lane_a_mux",    0x20,  0, PCA953X_DIR_IN, -1},
	{"sd1_2_lane_b_mux",    0x20,  1, PCA953X_DIR_IN, -1},
	{"sd1_0_lane_d_mux",    0x20,  2, PCA953X_DIR_IN, -1},
	{"sd2_1_lane_b_mux",    0x20,  3, PCA953X_DIR_IN, -1},
	{"sd2_3_lane_d_mux1",   0x20,  4, PCA953X_DIR_IN, -1},
	{"sd2_3_lane_d_mux2",   0x20,  5, PCA953X_DIR_IN, -1},
	{"sd_mux_shdn",         0x20,  6, PCA953X_DIR_IN, -1},
	{"sd1_ref_clk2_sel",    0x20,  7, PCA953X_DIR_IN, -1},
	{"mpcie1_disable#",     0x20,  8, PCA953X_DIR_OUT, 0},
	{"mpcie1_wake#",        0x20,  9, PCA953X_DIR_IN, -1},
	{"mpcie2_disable#",     0x20, 10, PCA953X_DIR_OUT, 0},
	{"mpcie2_wake#",        0x20, 11, PCA953X_DIR_IN, -1},
	{"prsnt#",              0x20, 12, PCA953X_DIR_IN, -1},
	{"pcie_pwr_en",         0x20, 13, PCA953X_DIR_OUT, 1},
	{"dcdc_pwr_en",         0x20, 14, PCA953X_DIR_OUT, 1},
	{"dcdc_pgood_1v8",      0x20, 15, PCA953X_DIR_IN, -1},
	{"xfi1_tx_fault",       0x21,  0, PCA953X_DIR_IN, -1},
	{"xfi1_tx_dis",         0x21,  1, PCA953X_DIR_OUT, 1},
	{"xfi1_moddef_det",     0x21,  2, PCA953X_DIR_IN, -1},
	{"xfi1_rx_loss",        0x21,  3, PCA953X_DIR_IN, -1},
	{"retimer1_loss",       0x21,  4, PCA953X_DIR_IN, -1},
	{"xfi1_ensmb" ,         0x21,  5, PCA953X_DIR_OUT, 1},
	{"qsgmii1_clk_sel0",    0x21,  6, PCA953X_DIR_IN, -1},
	{"qsgmii_phy1_config3", 0x21,  7, PCA953X_DIR_IN, -1},
	{"xfi2_tx_fault",       0x21,  8, PCA953X_DIR_IN, -1},
	{"xfi2_tx_dis",         0x21,  9, PCA953X_DIR_OUT, 1},
	{"xfi2_moddef_det",     0x21, 10, PCA953X_DIR_IN, -1},
	{"xfi2_rx_loss",        0x21, 11, PCA953X_DIR_IN, -1},
	{"retimer2_loss",       0x21, 12, PCA953X_DIR_IN, -1},
	{"xfi2_ensmb" ,         0x21, 13, PCA953X_DIR_OUT, 1},
	{"qsgmii2_clk_sel0",    0x21, 14, PCA953X_DIR_IN, -1},
	{"qsgmii_phy2_config3", 0x21, 15, PCA953X_DIR_IN, -1},
	{"ec1_phy_pwdn",        0x22,  0, PCA953X_DIR_IN, -1},
	{"ec2_phy_pwdn",        0x22,  1, PCA953X_DIR_IN, -1},
	{"usb_c_pwron",         0x22,  2, PCA953X_DIR_OUT, 0},
	{"usb_en_oc_3v3#",      0x22,  3, PCA953X_DIR_IN, -1},
	{"usb_h_grst#",         0x22,  4, PCA953X_DIR_OUT, 1},
	{"gpio_button0",        0x22,  5, PCA953X_DIR_IN, -1},
	{"gpio_button1",        0x22,  6, PCA953X_DIR_IN, -1},
	{"sda_pwr_en",          0x22,  7, PCA953X_DIR_OUT, 0},
	{"qsgmii_phy1_int#",    0x22,  8, PCA953X_DIR_IN, -1},
	{"qsgmii_phy2_int#",    0x22,  9, PCA953X_DIR_IN, -1},
	{"spi_clko_sof",        0x22, 10, PCA953X_DIR_IN, -1},
	{"spi_int#",            0x22, 11, PCA953X_DIR_IN, -1},
	{"can_sel",             0x22, 12, PCA953X_DIR_OUT, 0},
	{"led#",                0x22, 13, PCA953X_DIR_OUT, 1},
	{"pcie_rst_3v3#",       0x22, 14, PCA953X_DIR_OUT, 0},
	{"pcie_wake_3v3#",      0x22, 15, PCA953X_DIR_IN, -1},
};

static int __gpio_idx_by_name(const char *name)
{
	int i;

	if((name == NULL) || (strlen(name) <= 0)) {
		return -1;
	}

	for(i=0; i<sizeof(tqc_mbls10xxa_i2c_gpios)/sizeof(tqc_mbls10xxa_i2c_gpios[0]); i++) {
		if((strlen(tqc_mbls10xxa_i2c_gpios[i].name) == strlen(name)) &&
			(strncmp(tqc_mbls10xxa_i2c_gpios[i].name, name, strlen(name)) == 0)) {
			return i;
		}
	}

	return -1;
}

int tqc_mbls10xxa_i2c_gpios_init(void)
{
	int ret = 0;
	unsigned int oldbus;
	int i;

	oldbus = i2c_get_bus_num();
	i2c_set_bus_num(TQC_MBLS10XXA_I2C_GPIO_BUS_NUM);

	for(i=0; i<sizeof(tqc_mbls10xxa_i2c_gpios)/sizeof(tqc_mbls10xxa_i2c_gpios[0]); i++) {
		if(tqc_mbls10xxa_i2c_gpios[i].dir == PCA953X_DIR_OUT) {
			pca953x_set_val(tqc_mbls10xxa_i2c_gpios[i].addr,
				1 << tqc_mbls10xxa_i2c_gpios[i].nr,
				tqc_mbls10xxa_i2c_gpios[i].dval << tqc_mbls10xxa_i2c_gpios[i].nr);
		}
		pca953x_set_dir(tqc_mbls10xxa_i2c_gpios[i].addr,
			1 << tqc_mbls10xxa_i2c_gpios[i].nr,
			tqc_mbls10xxa_i2c_gpios[i].dir << tqc_mbls10xxa_i2c_gpios[i].nr);
	}

	i2c_set_bus_num(oldbus);
	return ret;
}

int tqc_mbls10xxa_i2c_gpio_get(const char *name)
{
	int ret = -1;
	unsigned int oldbus;
	int idx, val;

	idx = __gpio_idx_by_name(name);
	if(idx < 0)
		return -1;

	oldbus = i2c_get_bus_num();
	i2c_set_bus_num(TQC_MBLS10XXA_I2C_GPIO_BUS_NUM);

	if(tqc_mbls10xxa_i2c_gpios[idx].dir == PCA953X_DIR_IN) {
		val = pca953x_get_val(tqc_mbls10xxa_i2c_gpios[idx].addr);
		if(val >= 0) {
			ret = (val & (1 << tqc_mbls10xxa_i2c_gpios[idx].nr))?(1):(0);
		}
	}

	i2c_set_bus_num(oldbus);
	return ret;
}

int tqc_mbls10xxa_i2c_gpio_set(const char *name, int val)
{
	int ret = -1;
	unsigned int oldbus;
	int idx;

	idx = __gpio_idx_by_name(name);
	if(idx < 0)
		return -1;

	oldbus = i2c_get_bus_num();
	i2c_set_bus_num(TQC_MBLS10XXA_I2C_GPIO_BUS_NUM);

	if(tqc_mbls10xxa_i2c_gpios[idx].dir != PCA953X_DIR_OUT)
		return -1;

	if(!val)
		ret = pca953x_set_val(
				tqc_mbls10xxa_i2c_gpios[idx].addr,
				1 << tqc_mbls10xxa_i2c_gpios[idx].nr,
				0);
	else
		ret = pca953x_set_val(
				tqc_mbls10xxa_i2c_gpios[idx].addr,
				1 << tqc_mbls10xxa_i2c_gpios[idx].nr,
				1 <<  tqc_mbls10xxa_i2c_gpios[idx].nr);

	i2c_set_bus_num(oldbus);
	return ret;
}

int tqc_mbls10xxa_clk_cfg_init(void)
{
	int ret = 0;
	unsigned int oldbus;
	uint8_t buf[4];
	int i;

	oldbus = i2c_get_bus_num();
	i2c_set_bus_num(TQC_MBLS10XXA_I2C_CLKBUF_BUS_NUM);

	/* set LVDS output on channels 4 and 5 in all configurations */
	for(i=0; i<4; i++)
		buf[i] = 0xC0;
	ret = i2c_write(TQC_MBLS10xxA_I2C_CLKBUF_ADDR, 12, 1, buf, 4);
	if(ret != 0)
		printf("unable to set clock buffer\n");

	i2c_set_bus_num(oldbus);
	return ret;
}

static void tqc_mbls10xxa_retimer_init_one(uint8_t addr)
{
	struct {
		uint8_t reg;
		uint8_t value;
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
	uint8_t v;

	for (i = 0; i < ARRAY_SIZE(cfg); i++) {
		v = cfg[i].value;
		ret = i2c_write(addr, cfg[i].reg, 1,
				&v, sizeof(v));
		if (ret != 0) {
			printf("failed to configure XFI retimer %02x: %d\n",
			       addr, ret);
			break;
		}
	}
}

void tqc_mbls10xxa_retimer_init(void)
{
	const uint8_t addrs[] = TQC_MBLS10XXA_I2C_RETIMER_ADDRS;
	unsigned int oldbus;
	int i;

	oldbus = i2c_get_bus_num();
	i2c_set_bus_num(TQC_MBLS10XXA_I2C_RETIMER_BUS_NUM);

	for (i = 0; i < ARRAY_SIZE(addrs); i++)
		tqc_mbls10xxa_retimer_init_one(addrs[i]);

	i2c_set_bus_num(oldbus);
}

void tqc_mbls10xxa_xfi_init(struct ccsr_serdes *serdes, int lane)
{
	u32 gcr0;

	/* Put lane in reset */
	gcr0 = in_be32(&serdes->lane[lane].gcr0);
	gcr0 &= 0xFF9FFFFF;
	out_be32(&serdes->lane[lane].gcr0, gcr0);

	udelay(1);

	/* Set up Transmit Equalization Control Register 0 for XFI */
	out_be32(&serdes->lane[lane].tecr0, 0x00233827);

	udelay(1);

	/* Take lane out of reset */
	gcr0 |= 0x00600000;
	out_be32(&serdes->lane[lane].gcr0, gcr0);
}

#ifndef CONFIG_SPL_BUILD
static char *_srds_proto_name(int proto)
{
	switch(proto) {
	case TQC_MBLS10xxA_SRDS_PROTO_UNUSED:
		return "Unused    ";
	case TQC_MBLS10xxA_SRDS_PROTO_SGMII:
		return "SGMII     ";
	case TQC_MBLS10xxA_SRDS_PROTO_SGMII2G5:
		return "2.5G SGMII";
	case TQC_MBLS10xxA_SRDS_PROTO_QSGMII:
		return "QSGMII    ";
	case TQC_MBLS10xxA_SRDS_PROTO_XFI:
		return "XFI       ";
	case TQC_MBLS10xxA_SRDS_PROTO_PCIEx1:
		return "PCIe x1   ";
	case TQC_MBLS10xxA_SRDS_PROTO_PCIEx2:
		return "PCIe x2   ";
	case TQC_MBLS10xxA_SRDS_PROTO_PCIEx4:
		return "PCIe x4   ";
	case TQC_MBLS10xxA_SRDS_PROTO_SATA:
		return "SATA      ";
	default:
		return "Unknown   ";
	}
}

static int _srds_mux_check_and_print(int port, int lane, int mux, int proto)
{
	int stat = 0;

	if((proto != TQC_MBLS10xxA_SRDS_PROTO_UNUSED) &&
	   (proto != mux))
		stat++;

	printf("  SD%d-%d: MUX=%s | RCW=%s -> %s\n",
	   port, lane, _srds_proto_name(mux), _srds_proto_name(proto),
	   (stat)?("FAIL"):("OK"));

	return stat;
}

int tqc_mbls10xxa_serdes_init(struct tqc_mbls10xxa_serdes lanes[8])
{
	int idx = 0;
	int mux_val1, mux_val2;
	int mux_proto;
	int err_cnt = 0;

	printf("Checking MBLS10xxA SerDes muxing:\n");

	/* check state of SD_MUX_SHDN */
	mux_val1 = tqc_mbls10xxa_i2c_gpio_get("sd_mux_shdn");
	if(mux_val1) {
		printf("!!! ATTENTION: SerDes MUXes disabled,\n");
		printf("!!!  muxed SerDes interfaces won't work\n");
	}

	/* check config for SD1_0 */
	mux_val1 = tqc_mbls10xxa_i2c_gpio_get("sd1_0_lane_d_mux");
	if((mux_val1 >= 0) && (lanes[idx].port != 0)) {
		mux_proto = (mux_val1)?(TQC_MBLS10xxA_SRDS_PROTO_XFI):(TQC_MBLS10xxA_SRDS_PROTO_SGMII);
		err_cnt += _srds_mux_check_and_print(
			          lanes[idx].port, lanes[idx].lane,
			          mux_proto, lanes[idx].proto);
	}
	idx++;

	/* check config for SD1_1 */
	if(lanes[idx].port != 0) {
		mux_proto = TQC_MBLS10xxA_SRDS_PROTO_XFI;
		err_cnt += _srds_mux_check_and_print(
			          lanes[idx].port, lanes[idx].lane,
			          mux_proto, lanes[idx].proto);
	}
	idx++;

	/* check config for SD1_2 */
	mux_val1 = tqc_mbls10xxa_i2c_gpio_get("sd1_2_lane_b_mux");
	if((mux_val1 >= 0) && (lanes[idx].port != 0)) {
		mux_proto = (mux_val1)?(TQC_MBLS10xxA_SRDS_PROTO_QSGMII):(TQC_MBLS10xxA_SRDS_PROTO_SGMII);
		err_cnt += _srds_mux_check_and_print(
			          lanes[idx].port, lanes[idx].lane,
			          mux_proto, lanes[idx].proto);
	}
	idx++;

	/* check config for SD1_3 */
	mux_val1 = tqc_mbls10xxa_i2c_gpio_get("sd1_3_lane_a_mux");
	if((mux_val1 >= 0) && (lanes[idx].port != 0)) {
		mux_proto = (mux_val1)?(TQC_MBLS10xxA_SRDS_PROTO_QSGMII):(TQC_MBLS10xxA_SRDS_PROTO_SGMII);
		err_cnt += _srds_mux_check_and_print(
			          lanes[idx].port, lanes[idx].lane,
			          mux_proto, lanes[idx].proto);
	}
	idx++;

	/* check config for SD2_0 */
	if(lanes[idx].port != 0) {
		mux_proto = TQC_MBLS10xxA_SRDS_PROTO_PCIEx1;
		err_cnt += _srds_mux_check_and_print(
			          lanes[idx].port, lanes[idx].lane,
			          mux_proto, lanes[idx].proto);
	}
	idx++;

	/* check config for SD2_1 */
	mux_val1 = tqc_mbls10xxa_i2c_gpio_get("sd2_1_lane_b_mux");
	if((mux_val1 >= 0) && (lanes[idx].port != 0)) {
		mux_proto = (mux_val1)?(TQC_MBLS10xxA_SRDS_PROTO_SGMII):(TQC_MBLS10xxA_SRDS_PROTO_PCIEx1);
		err_cnt += _srds_mux_check_and_print(
			          lanes[idx].port, lanes[idx].lane,
			          mux_proto, lanes[idx].proto);

		/* enable miniPCIe-Slot when selected */
		if((lanes[idx].proto == TQC_MBLS10xxA_SRDS_PROTO_PCIEx1) &&
		   (mux_proto == lanes[idx].proto))
			tqc_mbls10xxa_i2c_gpio_set("mpcie2_disable#", 1);
		else
			tqc_mbls10xxa_i2c_gpio_set("mpcie2_disable#", 0);
	}
	idx++;

	/* check config for SD2_2 */
	if(lanes[idx].port != 0) {
		mux_proto = TQC_MBLS10xxA_SRDS_PROTO_PCIEx1;
		if(lanes[idx].proto == TQC_MBLS10xxA_SRDS_PROTO_PCIEx2)
			mux_proto = TQC_MBLS10xxA_SRDS_PROTO_PCIEx2;
		err_cnt += _srds_mux_check_and_print(
			          lanes[idx].port, lanes[idx].lane,
			          mux_proto, lanes[idx].proto);

		/* get M.2 slot out of reset when lane (SD2-2) is configured as PCIe */
		if(((lanes[idx].proto == TQC_MBLS10xxA_SRDS_PROTO_PCIEx1) ||
		    (lanes[idx].proto == TQC_MBLS10xxA_SRDS_PROTO_PCIEx2)) &&
		   (mux_proto == lanes[idx].proto))
			tqc_mbls10xxa_i2c_gpio_set("pcie_rst_3v3#", 1);
		else
			tqc_mbls10xxa_i2c_gpio_set("pcie_rst_3v3#", 0);

	}
	idx++;

	/* check config for SD2 - LANE D */
	mux_val1 = tqc_mbls10xxa_i2c_gpio_get("sd2_3_lane_d_mux1");
	mux_val2 = tqc_mbls10xxa_i2c_gpio_get("sd2_3_lane_d_mux2");
	if((mux_val1 >= 0) && (mux_val2 >= 0) && (lanes[idx].port != 0)) {
		mux_proto = (mux_val1)?
			           ((mux_val2)?(TQC_MBLS10xxA_SRDS_PROTO_PCIEx1):(TQC_MBLS10xxA_SRDS_PROTO_SATA)):
					   (TQC_MBLS10xxA_SRDS_PROTO_PCIEx2);
		err_cnt += _srds_mux_check_and_print(
			          lanes[idx].port, lanes[idx].lane,
			          mux_proto, lanes[idx].proto);

		/* enable miniPCIe-Slot when selected */
		if((lanes[idx].proto == TQC_MBLS10xxA_SRDS_PROTO_PCIEx1) &&
		   (mux_proto == lanes[idx].proto))
			tqc_mbls10xxa_i2c_gpio_set("mpcie1_disable#", 1);
		else
			tqc_mbls10xxa_i2c_gpio_set("mpcie1_disable#", 0);
	}
	idx++;

	/* print error message when muxing is invalid */
	if(err_cnt) {
		printf("!!! ATTENTION: Some SerDes lanes are misconfigured,\n");
		printf("!!!  this may cause some interfaces to be inoperable.\n");
		printf("!!!  Check SerDes muxing DIP switch settings!\n");
	}

	return 0;
}

int tqc_mbls10xxa_serdes_clk_get(int clk)
{
	int freq = -1;
	int mux_val;

	switch(clk) {
		case TQC_MBLS10xxA_SRDS_CLK_1_1:
		case TQC_MBLS10xxA_SRDS_CLK_2_1:
		case TQC_MBLS10xxA_SRDS_CLK_2_2:
			/* fixed serdes clock (125MHz) */
			freq = 125000000;
			break;
		case TQC_MBLS10xxA_SRDS_CLK_1_2:
			/* get clock mux */
			mux_val = tqc_mbls10xxa_i2c_gpio_get("sd1_ref_clk2_sel");
			if(mux_val >= 0) {
				if(mux_val)
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

static uint16_t _rgmii_phy_read_indirect(struct phy_device *phydev,
					uint8_t addr)
{
	phy_write(phydev, MDIO_DEVAD_NONE, 0x0d, 0x001f);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x0e, addr);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x0d, 0x401f);
	return phy_read(phydev, MDIO_DEVAD_NONE, 0x0e);
}

static void _rgmii_phy_write_indirect(struct phy_device *phydev,
					uint8_t addr, uint16_t value)
{
	phy_write(phydev, MDIO_DEVAD_NONE, 0x0d, 0x001f);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x0e, addr);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x0d, 0x401f);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x0e, value);
}

int tqc_mbls10xxa_board_phy_config(struct phy_device *phydev)
{
	uint16_t val;
	int ret = 0;
	static bool qsgmii1_initdone = false;
	static bool qsgmii2_initdone = false;

	if (phydev->drv->config)
		ret = phydev->drv->config(phydev);


	if(!ret) {
		if((phydev->addr == RGMII_PHY1_ADDR) ||
		   (phydev->addr == RGMII_PHY2_ADDR)) {
			/* enable RGMII delay in both directions */
			val = _rgmii_phy_read_indirect(phydev, 0x32);
			val |= 0x0003;
			_rgmii_phy_write_indirect(phydev, 0x32, val);

			/* set RGMII delay in both directions to 1,5ns */
			val = _rgmii_phy_read_indirect(phydev, 0x86);
			val = (val & 0xFF00) | 0x0055;
			_rgmii_phy_write_indirect(phydev, 0x86, val);
		} else if(((phydev->addr & 0x1C) == QSGMII_PHY1_ADDR_BASE) ||
		          ((phydev->addr & 0x1C) == QSGMII_PHY2_ADDR_BASE)) {
			/* check if initialization has already been done */
			if((((phydev->addr & 0x1C) == QSGMII_PHY1_ADDR_BASE) &&
			    qsgmii1_initdone) ||
			   (((phydev->addr & 0x1C) == QSGMII_PHY2_ADDR_BASE) &&
			    qsgmii2_initdone)) {
				/* initialization already done, skip */
				return ret;
			}

			/* reset PHY because clock input may have changed */
			phy_write(phydev, MDIO_DEVAD_NONE, 0x16, 0x0004);
			phy_write(phydev, MDIO_DEVAD_NONE, 0x1B, 0x8000);
			/* wait for 50ms for reset to complete */
			mdelay(50);

			/* execute marvell specified initialization */
			/* see MV-S301615 release note */
			/* PHY initialization #1 */
			phy_write(phydev, MDIO_DEVAD_NONE, 0x16, 0x00ff);
			phy_write(phydev, MDIO_DEVAD_NONE, 0x18, 0x2800);
			phy_write(phydev, MDIO_DEVAD_NONE, 0x17, 0x2001);
			phy_write(phydev, MDIO_DEVAD_NONE, 0x16, 0x0000);
			/* PHY initialization #2 */
			phy_write(phydev, MDIO_DEVAD_NONE, 0x16, 0x0000);
			phy_write(phydev, MDIO_DEVAD_NONE, 0x1D, 0x0003);
			phy_write(phydev, MDIO_DEVAD_NONE, 0x1E, 0x0002);
			phy_write(phydev, MDIO_DEVAD_NONE, 0x16, 0x0000);

			/* check configuration of PHY device */
			printf("QSGMII ethernet PHY 0x%02x configuration: ", phydev->addr);
			phy_write(phydev, MDIO_DEVAD_NONE, 0x16, 0x0006);
			val = phy_read(phydev, MDIO_DEVAD_NONE, 0x14);
			phy_write(phydev, MDIO_DEVAD_NONE, 0x16, 0x0000);
			if((val & 0x7) == 0) {
				printf("QSGMII to Copper\n");
			} else if((val & 0x7) == 1) {
				printf("SGMII to Copper\n");
			} else {
				printf("unsupported\n");
			}

			/* mark PHY as initialized */
			if((phydev->addr & 0x1C) == QSGMII_PHY1_ADDR_BASE)
				qsgmii1_initdone = true;
			if((phydev->addr & 0x1C) == QSGMII_PHY2_ADDR_BASE)
				qsgmii2_initdone = true;
		}
	}

	return ret;
}
#endif

