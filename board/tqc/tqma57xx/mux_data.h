/*
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com
 *
 * Author: Felipe Balbi <balbi@ti.com>
 *
 * Based on board/ti/dra7xx/evm.c
 *
 * Copyright (C) 2017 TQ-Systems GmbH (ported AM57xx IDK to TQMa57xx)
 *
 * Author: Stefan Lange <s.lange@gateware.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef _MUX_DATA_TQMA57XX_H_
#define _MUX_DATA_TQMA57XX_H_

#include <asm/arch/mux_dra7xx.h>

#define WAKEUP_ENA                (1 << 24)

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
	{GPMC_CS3, (M1 | PIN_OUTPUT | MANUAL_MODE)},    /* gpmc_cs3.qspi1_cs1 */

	/* MMC2: eMMC */
	{GPMC_A19, (M1 | PIN_INPUT | MANUAL_MODE)},    /* gpmc_a19.mmc2_dat4 */
	{GPMC_A20, (M1 | PIN_INPUT | MANUAL_MODE)},    /* gpmc_a20.mmc2_dat5 */
	{GPMC_A21, (M1 | PIN_INPUT | MANUAL_MODE)},    /* gpmc_a21.mmc2_dat6 */
	{GPMC_A22, (M1 | PIN_INPUT | MANUAL_MODE)},    /* gpmc_a22.mmc2_dat7 */
	{GPMC_A23, (M1 | PIN_INPUT | MANUAL_MODE)},    /* gpmc_a23.mmc2_clk */
	{GPMC_A24, (M1 | PIN_INPUT | MANUAL_MODE)},    /* gpmc_a24.mmc2_dat0 */
	{GPMC_A25, (M1 | PIN_INPUT | MANUAL_MODE)},    /* gpmc_a25.mmc2_dat1 */
	{GPMC_A26, (M1 | PIN_INPUT | MANUAL_MODE)},    /* gpmc_a26.mmc2_dat2 */
	{GPMC_A27, (M1 | PIN_INPUT | MANUAL_MODE)},    /* gpmc_a27.mmc2_dat3 */
	{GPMC_CS1, (M1 | PIN_INPUT | MANUAL_MODE)},    /* gpmc_cs1.mmc2_cmd */

	/* control */
	{WAKEUP0, (M14 | PIN_INPUT | WAKEUP_ENA)},    /* Wakeup0.gpio1_0, from RTC */
	{ON_OFF, (M0 | PIN_OUTPUT)},    /* on_off.on_off */
	{RTC_PORZ, (M0 | PIN_INPUT)},   /* rtc_porz.rtc_porz */

	/* JTAG */
	{TMS, (M0 | PIN_INPUT)},	/* tms.tms */
	{TDI, (M0 | PIN_INPUT_SLEW)},	/* tdi.tdi */
	{TDO, (M0 | PIN_OUTPUT)},	/* tdo.tdo */
	{TCLK, (M0 | PIN_INPUT)},	/* tclk.tclk */
	{TRSTN, (M0 | PIN_INPUT)},	/* trstn.trstn */
	{RTCK, (M0 | PIN_OUTPUT)},	/* rtck.rtck */
	{EMU0, (M0 | PIN_INPUT)},	/* emu0.emu0 */
	{EMU1, (M0 | PIN_INPUT)},	/* emu1.emu1 */

	/* reset */
	{RESETN, (M0 | PIN_INPUT)},     /* resetn.resetn */
	{RSTOUTN, (M0 | PIN_OUTPUT)},   /* rstoutn.rstoutn */
};

const struct pad_conf_entry early_padconf[] = {
	{I2C1_SDA, (M0 | PIN_INPUT)},	/* i2c1_sda.i2c1_sda */
	{I2C1_SCL, (M0 | PIN_INPUT)},	/* i2c1_scl.i2c1_scl */
	{SPI2_SCLK, (M1 | PIN_INPUT)},	/* spi2_sclk.uart3_rxd */
	{SPI2_D1, (M1 | PIN_OUTPUT | SLEWCONTROL)},	/* spi2_d1.uart3_txd */
};

/*
 * Only TQMa57xx iodelay values defined here
 * For baseboard relevant iodelay values see respective baseboard file
 * (e.g. MBa57xx: tqma57xx_mba57xx.c)
 */
#ifdef CONFIG_IODELAY_RECALIBRATION
const struct iodelay_cfg_entry iodelay_cfg_array_tqma57xx[] = {
	/* QSPI */
	{0x0188, 590,	0	}, /* CFG_GPMC_A18_OUT	R2 */
	{0x0144, 0,	0	}, /* CFG_GPMC_A13_IN	R3 */
	{0x0374, 0,	0	}, /* CFG_GPMC_CS2_OUT	P2 */
	{0x0380, 70,	0	}, /* CFG_GPMC_CS3_OUT	P1 */
	{0x0168, 2528,	1007	}, /* CFG_GPMC_A16_IN	U1 */
	{0x0170, 0,	0	}, /* CFG_GPMC_A16_OUT	U1 */
	{0x0174, 2533,	980	}, /* CFG_GPMC_A17_IN	P3 */
	{0x015C, 2503,	889	}, /* CFG_GPMC_A15_IN	U2 */
	{0x0150, 2575,	966	}, /* CFG_GPMC_A14_IN	T2 */

	/* MMC2: eMMC */
	{0x01D0, 935,	280   }, /* CFG_GPMC_A23_OUT  J7 */
	{0x0364, 684,	0     }, /* CFG_GPMC_CS1_OEN  H6 */
	{0x0368, 76,	0     }, /* CFG_GPMC_CS1_OUT  H6 */
	{0x01D8, 621,	0     }, /* CFG_GPMC_A24_OEN  J4 */
	{0x01DC, 0,	0     }, /* CFG_GPMC_A24_OUT  J4 */
	{0x01E4, 183,	0     }, /* CFG_GPMC_A25_OEN  J6 */
	{0x01E8, 0,	0     }, /* CFG_GPMC_A25_OUT  J6 */
	{0x01F0, 467,	0     }, /* CFG_GPMC_A26_OEN  H4 */
	{0x01F4, 0,	0     }, /* CFG_GPMC_A26_OUT  H4 */
	{0x01FC, 262,	0     }, /* CFG_GPMC_A27_OEN  H5 */
	{0x0200, 46,	0     }, /* CFG_GPMC_A27_OUT  H5 */
	{0x0190, 274,	0     }, /* CFG_GPMC_A19_OEN  K7 */
	{0x0194, 162,	0     }, /* CFG_GPMC_A19_OUT  K7 */
	{0x01A8, 401,	0     }, /* CFG_GPMC_A20_OEN  M7 */
	{0x01AC, 73,	0     }, /* CFG_GPMC_A20_OUT  M7 */
	{0x01B4, 465,	0     }, /* CFG_GPMC_A21_OEN  J5 */
	{0x01B8, 115,	0     }, /* CFG_GPMC_A21_OUT  J5 */
	{0x01C0, 633,	0     }, /* CFG_GPMC_A22_OEN  K6 */
	{0x01C4, 47,	0     }, /* CFG_GPMC_A22_OUT  K6 */
};

#endif
#endif /* _MUX_DATA_TQMA57XX_H_ */
