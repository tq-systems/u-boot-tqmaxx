/*
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com
 *
 * Author: Felipe Balbi <balbi@ti.com>
 *
 * Based on board/ti/dra7xx/evm.c
 *
 * Copyright (C) 2017 TQ Systems (ported AM57xx IDK to TQMa57xx)
 *
 * Author: Stefan Lange <s.lange@gateware.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef _MUX_DATA_TQMA57XX_H_
#define _MUX_DATA_TQMA57XX_H_

#include <asm/arch/mux_dra7xx.h>

/*
 * Only TQMa57xx pad configs defined here
 * For baseboard relevant pad configs see respective baseboard file
 * (e.g. MBa57xx: tqma57xx_mba57xx.c)
 */
const struct pad_conf_entry core_padconf_array_essential_tqma57xx[] = {
	/* QSPI */
	{GPMC_A13, (M1 | PIN_INPUT | MANUAL_MODE)},	/* gpmc_a13.qspi1_rtclk */
	{GPMC_A14, (M1 | PIN_INPUT | MANUAL_MODE)},	/* gpmc_a14.qspi1_d3 */
	{GPMC_A15, (M1 | PIN_INPUT | MANUAL_MODE)},	/* gpmc_a15.qspi1_d2 */
	{GPMC_A16, (M1 | PIN_INPUT | MANUAL_MODE)},	/* gpmc_a16.qspi1_d0 */
	{GPMC_A17, (M1 | PIN_INPUT | MANUAL_MODE)},	/* gpmc_a17.qspi1_d1 */
	{GPMC_A18, (M1 | PIN_OUTPUT | MANUAL_MODE)},	/* gpmc_a18.qspi1_sclk */
	{GPMC_CS2, (M1 | PIN_OUTPUT | MANUAL_MODE)},	/* gpmc_cs2.qspi1_cs0 */
	{GPMC_CS3, (M2 | PIN_INPUT_PULLDOWN | MANUAL_MODE)},    /* gpmc_cs3.vin3a_clk0 */

	/* MMC2: eMMC */
	{GPMC_A19, (M1 | PIN_INPUT_PULLUP)},    /* gpmc_a19.mmc2_dat4 */
	{GPMC_A20, (M1 | PIN_INPUT_PULLUP)},    /* gpmc_a20.mmc2_dat5 */
	{GPMC_A21, (M1 | PIN_INPUT_PULLUP)},    /* gpmc_a21.mmc2_dat6 */
	{GPMC_A22, (M1 | PIN_INPUT_PULLUP)},    /* gpmc_a22.mmc2_dat7 */
	{GPMC_A23, (M1 | PIN_INPUT_PULLUP)},    /* gpmc_a23.mmc2_clk */
	{GPMC_A24, (M1 | PIN_INPUT_PULLUP)},    /* gpmc_a24.mmc2_dat0 */
	{GPMC_A25, (M1 | PIN_INPUT_PULLUP)},    /* gpmc_a25.mmc2_dat1 */
	{GPMC_A26, (M1 | PIN_INPUT_PULLUP)},    /* gpmc_a26.mmc2_dat2 */
	{GPMC_A27, (M1 | PIN_INPUT_PULLUP)},    /* gpmc_a27.mmc2_dat3 */
	{GPMC_CS1, (M1 | PIN_INPUT_PULLUP)},    /* gpmc_cs1.mmc2_cmd */

	/* control */
	{WAKEUP0, (M0 | PIN_INPUT)},    /* Wakeup0.Wakeup0, from RTC */
	{ON_OFF, (M0 | PIN_OUTPUT)},    /* on_off.on_off */
	{RTC_PORZ, (M0 | PIN_INPUT)},   /* rtc_porz.rtc_porz */

	/* JTAG */
	{TDI, (M0 | PIN_INPUT_PULLUP | SLEWCONTROL)},   /* tdi.tdi */
	{TDO, (M0 | PIN_OUTPUT_PULLUP)},        /* tdo.tdo */
	{TCLK, (M0 | PIN_INPUT_PULLUP)},        /* tclk.tclk */
	{TRSTN, (M0 | PIN_INPUT_PULLDOWN)},     /* trstn.trstn */
	{RTCK, (M0 | PIN_OUTPUT_PULLUP)},       /* rtck.rtck */
	{EMU0, (M0 | PIN_INPUT_PULLUP)},        /* emu0.emu0 */
	{EMU1, (M0 | PIN_INPUT_PULLUP)},        /* emu1.emu1 */

	/* reset */
	{RESETN, (M0 | PIN_INPUT)},     /* resetn.resetn */
	{RSTOUTN, (M0 | PIN_OUTPUT)},   /* rstoutn.rstoutn */
};

const struct pad_conf_entry early_padconf[] = {
	{I2C1_SDA, (PIN_INPUT_PULLUP | M0)},	/* i2c1_sda.i2c1_sda */
	{I2C1_SCL, (PIN_INPUT_PULLUP | M0)},	/* i2c1_scl.i2c1_scl */
	{SPI2_SCLK, (M0 | PIN_INPUT)},  /* spi2_sclk.spi2_sclk */
	{SPI2_D1, (M0 | PIN_INPUT | SLEWCONTROL)},      /* spi2_d1.spi2_d1 */
};

/*
 * Only TQMa57xx iodelay values defined here
 * For baseboard relevant iodelay values see respective baseboard file
 * (e.g. MBa57xx: tqma57xx_mba57xx.c)
 */
#ifdef CONFIG_IODELAY_RECALIBRATION
const struct iodelay_cfg_entry iodelay_cfg_array_tqma57xx[] = {
	{0x0114, 1861, 901},	/* CFG_GPMC_A0_IN */
	{0x0120, 0, 0},	/* CFG_GPMC_A10_IN */
	{0x012C, 1783, 1178},	/* CFG_GPMC_A11_IN */
	{0x0138, 1903, 853},	/* CFG_GPMC_A12_IN */
	{0x0144, 0, 0},	/* CFG_GPMC_A13_IN */
	{0x0150, 2575, 966},	/* CFG_GPMC_A14_IN */
	{0x015C, 2503, 889},	/* CFG_GPMC_A15_IN */
	{0x0168, 2528, 1007},	/* CFG_GPMC_A16_IN */
	{0x0170, 0, 0},	/* CFG_GPMC_A16_OUT */
	{0x0174, 2533, 980},	/* CFG_GPMC_A17_IN */
	{0x0188, 590, 0},	/* CFG_GPMC_A18_OUT */
	{0x0198, 1652, 891},	/* CFG_GPMC_A1_IN */
	{0x0204, 1888, 1212},	/* CFG_GPMC_A2_IN */
	{0x0210, 1839, 1274},	/* CFG_GPMC_A3_IN */
	{0x021C, 1868, 1113},	/* CFG_GPMC_A4_IN */
	{0x0228, 1757, 1079},	/* CFG_GPMC_A5_IN */
	{0x0234, 1800, 670},	/* CFG_GPMC_A6_IN */
	{0x0240, 1967, 898},	/* CFG_GPMC_A7_IN */
	{0x024C, 1731, 959},	/* CFG_GPMC_A8_IN */
	{0x0258, 1766, 1150},	/* CFG_GPMC_A9_IN */
	{0x0374, 0, 0},	/* CFG_GPMC_CS2_OUT */
	{0x0590, 1000, 4200},	/* CFG_MCASP5_ACLKX_OUT */
	{0x05AC, 800, 3800},	/* CFG_MCASP5_FSX_IN */
	{0x06F0, 260, 0},	/* CFG_RGMII0_RXC_IN */
	{0x06FC, 0, 1412},	/* CFG_RGMII0_RXCTL_IN */
	{0x0708, 123, 1047},	/* CFG_RGMII0_RXD0_IN */
	{0x0714, 139, 1081},	/* CFG_RGMII0_RXD1_IN */
	{0x0720, 195, 1100},	/* CFG_RGMII0_RXD2_IN */
	{0x072C, 239, 1216},	/* CFG_RGMII0_RXD3_IN */
	{0x0740, 89, 0},	/* CFG_RGMII0_TXC_OUT */
	{0x074C, 15, 125},	/* CFG_RGMII0_TXCTL_OUT */
	{0x0758, 339, 162},	/* CFG_RGMII0_TXD0_OUT */
	{0x0764, 146, 94},	/* CFG_RGMII0_TXD1_OUT */
	{0x0770, 0, 27},	/* CFG_RGMII0_TXD2_OUT */
	{0x077C, 291, 205},	/* CFG_RGMII0_TXD3_OUT */
	{0x0A70, 0, 0},	/* CFG_VIN2A_D12_OUT */
	{0x0A7C, 219, 101},	/* CFG_VIN2A_D13_OUT */
	{0x0A88, 92, 58},	/* CFG_VIN2A_D14_OUT */
	{0x0A94, 135, 100},	/* CFG_VIN2A_D15_OUT */
	{0x0AA0, 154, 101},	/* CFG_VIN2A_D16_OUT */
	{0x0AAC, 78, 27},	/* CFG_VIN2A_D17_OUT */
	{0x0AB0, 411, 0},	/* CFG_VIN2A_D18_IN */
	{0x0ABC, 0, 382},	/* CFG_VIN2A_D19_IN */
	{0x0AD4, 320, 750},	/* CFG_VIN2A_D20_IN */
	{0x0AE0, 192, 836},	/* CFG_VIN2A_D21_IN */
	{0x0AEC, 294, 669},	/* CFG_VIN2A_D22_IN */
	{0x0AF8, 50, 700},	/* CFG_VIN2A_D23_IN */
	{0x0B30, 0, 0},	/* CFG_VIN2A_D5_OUT */
	{0x0B9C, 1126, 751},	/* CFG_VOUT1_CLK_OUT */
	{0x0BA8, 395, 0},	/* CFG_VOUT1_D0_OUT */
	{0x0BB4, 282, 0},	/* CFG_VOUT1_D10_OUT */
	{0x0BC0, 348, 0},	/* CFG_VOUT1_D11_OUT */
	{0x0BCC, 1240, 0},	/* CFG_VOUT1_D12_OUT */
	{0x0BD8, 182, 0},	/* CFG_VOUT1_D13_OUT */
	{0x0BE4, 311, 0},	/* CFG_VOUT1_D14_OUT */
	{0x0BF0, 285, 0},	/* CFG_VOUT1_D15_OUT */
	{0x0BFC, 166, 0},	/* CFG_VOUT1_D16_OUT */
	{0x0C08, 278, 0},	/* CFG_VOUT1_D17_OUT */
	{0x0C14, 425, 0},	/* CFG_VOUT1_D18_OUT */
	{0x0C20, 516, 0},	/* CFG_VOUT1_D19_OUT */
	{0x0C2C, 521, 0},	/* CFG_VOUT1_D1_OUT */
	{0x0C38, 386, 0},	/* CFG_VOUT1_D20_OUT */
	{0x0C44, 111, 0},	/* CFG_VOUT1_D21_OUT */
	{0x0C50, 227, 0},	/* CFG_VOUT1_D22_OUT */
	{0x0C5C, 0, 0},	/* CFG_VOUT1_D23_OUT */
	{0x0C68, 282, 0},	/* CFG_VOUT1_D2_OUT */
	{0x0C74, 438, 0},	/* CFG_VOUT1_D3_OUT */
	{0x0C80, 1298, 0},	/* CFG_VOUT1_D4_OUT */
	{0x0C8C, 397, 0},	/* CFG_VOUT1_D5_OUT */
	{0x0C98, 321, 0},	/* CFG_VOUT1_D6_OUT */
	{0x0CA4, 155, 309},	/* CFG_VOUT1_D7_OUT */
	{0x0CB0, 212, 0},	/* CFG_VOUT1_D8_OUT */
	{0x0CBC, 466, 0},	/* CFG_VOUT1_D9_OUT */
	{0x0CC8, 0, 0},	/* CFG_VOUT1_DE_OUT */
	{0x0CE0, 0, 0},	/* CFG_VOUT1_HSYNC_OUT */
	{0x0CEC, 139, 701},	/* CFG_VOUT1_VSYNC_OUT */
};

#endif
#endif /* _MUX_DATA_TQMA57XX_H_ */
