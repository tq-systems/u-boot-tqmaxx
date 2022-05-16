/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2021, TQ-Systems GmbH
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 */

#ifndef _TQMA335X_H_
#define _TQMA335X_H_

/*
 * We have three pin mux functions that must exist.  We must be able to enable
 * uart0, for initial output and i2c0 to read the main EEPROM.  We then have a
 * main pinmux function that can be overridden to enable all other pinmux that
 * is required on the board.
 */
void enable_uart0_pin_mux(void);
void enable_uart3_pin_mux(void);
void enable_uart4_pin_mux(void);
void enable_i2c0_pin_mux(void);
void enable_board_pin_mux(void);

/* Samsung K4B2G1646F-BMK0
 * TQ-SAP: 282221.0100
 * 2GBIT F-DIE 16MX16BITX8BANKS 800MHZ 1,35V 1,25NS SDRAM DDR3L -40/+85C
 * Speed: DDR3-800; 6-6-6
 * DDR_CLK = 400MHz
 * OPP100
 *
 * tCK_min = 2,5ns
 * CAS = 6
 * tRCD_min = 15
 * tRP_min = 15
 * tRAS_min = 37,5
 * tRC_min = 52,5
 *
 * Setting-File:
 * ssh://git@tq-git-pr1.tq-net.de/tq-embedded/tqmaxx/TQMa335x.SW.IBPQ.git
 * tree file: DDR3/TQMa335x.DDR3_Timing.gel
 * tag: REV0200
 */
#define DDR3L_256MB_EMIF_READ_LATENCY	0x308		/* READ_ODT on, RD_Latency = (CL + 2) - 1, !!! by CLKINVERT=1 -> (CL+1)!!! see ref.man.! */
#define DDR3L_256MB_EMIF_TIM1		0x0AAAE51B	/* x[3], tRP[4], tRCD[4], tWR[4], tRAS[5], */
#define DDR3L_256MB_EMIF_TIM2		0x24437FDA
#define DDR3L_256MB_EMIF_TIM3		0x501F83FF
#define DDR3L_256MB_EMIF_SDCFG		0x614052B2	/* termination/odt  */
#define DDR3L_256MB_EMIF_SDREF		0x20000618	/* tREFI = 400 * 3.9us = 0x618 */
#define DDR3L_256MB_ZQ_CFG			0x50074BE4
#define DDR3L_256MB_RATIO			0x100
#define DDR3L_256MB_INVERT_CLKOUT		0x1		/* !!! CLK INVERT !!! */
#define DDR3L_256MB_RD_DQS			0x37		/* RD_DQS_SLAVE_RATIO */
#define DDR3L_256MB_PHY_FIFO_WE		0xFC		/* FIFO_WE_SLAVE_RATIO, RD DQS GATE */
#define DDR3L_256MB_WR_DQS			0x82		/* WR_DQS_SLAVE_RATIO */
#define DDR3L_256MB_PHY_WR_DATA		0xC3		/* WR_DATA_SLAVE_RATIO, WRITE DATA*/
#define DDR3L_256MB_IOCTRL_VALUE		0x0000018B

/* Samsung K4B2G1646DBMK00CV */
#define DDR3L_512MB_EMIF_READ_LATENCY		0x308		/* READ_ODT on, RD_Latency = (CL + 2) - 1, !!! by CLKINVERT=1 -> (CL+1)!!! see ref.man.! */
#define DDR3L_512MB_EMIF_TIM1			0x0AAAE51B	/* x[3], tRP[4], tRCD[4], tWR[4], tRAS[5], */
#define DDR3L_512MB_EMIF_TIM2			0x246B7FDA
#define DDR3L_512MB_EMIF_TIM3			0x501F867F
#define DDR3L_512MB_EMIF_SDCFG			0x61405332
#define DDR3L_512MB_EMIF_SDREF			0x20000618	/* tREFI = 400 * 3.9us = 0x618 */
#define DDR3L_512MB_ZQ_CFG			0x50074BE4
#define DDR3L_512MB_RATIO			0x100
#define DDR3L_512MB_INVERT_CLKOUT		0x1		/* !!! CLK INVERT !!! */
#define DDR3L_512MB_RD_DQS			0x37		/* RD_DQS_SLAVE_RATIO */
#define DDR3L_512MB_PHY_FIFO_WE			0xF6		/* FIFO_WE_SLAVE_RATIO, RD DQS GATE */
#define DDR3L_512MB_WR_DQS			0x81		/* WR_DQS_SLAVE_RATIO */
#define DDR3L_512MB_PHY_WR_DATA			0xC1		/* WR_DATA_SLAVE_RATIO, WRITE DATA*/
#define DDR3L_512MB_IOCTRL_VALUE		0x0000018B

#endif
