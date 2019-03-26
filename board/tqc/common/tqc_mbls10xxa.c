// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 TQ-Systems GmbH
 */
#include <common.h>
#include <i2c.h>
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
	{"retimer1_loss#",      0x21,  4, PCA953X_DIR_IN, -1},
	{"xfi1_ensmb#",         0x21,  5, PCA953X_DIR_OUT, 1},
	{"qsgmii1_clk_sel0",    0x21,  6, PCA953X_DIR_IN, -1},
	{"qsgmii_phy1_config3", 0x21,  7, PCA953X_DIR_IN, -1},
	{"xfi2_tx_fault",       0x21,  8, PCA953X_DIR_IN, -1},
	{"xfi2_tx_dis",         0x21,  9, PCA953X_DIR_OUT, 1},
	{"xfi2_moddef_det",     0x21, 10, PCA953X_DIR_IN, -1},
	{"xfi2_rx_loss",        0x21, 11, PCA953X_DIR_IN, -1},
	{"retimer2_loss#",      0x21, 12, PCA953X_DIR_IN, -1},
	{"xfi2_ensmb#",         0x21, 13, PCA953X_DIR_OUT, 1},
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

