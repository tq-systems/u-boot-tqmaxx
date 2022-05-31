/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2021 - 2022, TQ-Systems GmbH
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 */

#ifndef _TQMA335X_H_
#define _TQMA335X_H_

/*
 * We have some pin mux functions that must exist:
 * - We must be able to enable uart<n> for initial output
 * - and i2c0 to access the module EEPROM and PMIC.
 * We then have a main pinmux function that can be overridden to enable all
 * other pinmux that is required on the baseboard.
 * For convinience we have some standard muxing functions for common interfaces.
 */
void enable_uart0_pin_mux(void);
void enable_uart3_pin_mux(void);
void enable_uart4_pin_mux(void);
void enable_i2c0_pin_mux(void);
void enable_mmc0_pin_mux(void);
/* can be overwritten */
void enable_board_pin_mux(void);

/* DDR3L 2GB / 256MB */
/* Setting-File: TQMa335x.DDR3.0207.xlsx */
#define DDR3L_256MB_EMIF_READ_LATENCY	0x00100208
#define DDR3L_256MB_EMIF_TIM1		0x0AAAE51B
#define DDR3L_256MB_EMIF_TIM2		0x24437FDA
#define DDR3L_256MB_EMIF_TIM3		0x50FFE3FF
#define DDR3L_256MB_EMIF_SDCFG		0x61A052B2
#define DDR3L_256MB_EMIF_SDREF		0x100005A0
#define DDR3L_256MB_ZQ_CFG		0x5007A468
#define DDR3L_256MB_RATIO		0x00000100
#define DDR3L_256MB_INVERT_CLKOUT	0x00000001
#define DDR3L_256MB_RD_DQS		0x00000037
#define DDR3L_256MB_WR_DQS		0x00000081
#define DDR3L_256MB_PHY_FIFO_WE		0x000000F6
#define DDR3L_256MB_PHY_WR_DATA		0x000000C1
#define DDR3L_256MB_IOCTRL_VALUE	0x0000018B

/* DDR3L 4GB / 512MB */
/* Setting-File: TQMa335x.DDR3.0207.xlsx */
#define DDR3L_512MB_EMIF_READ_LATENCY	0x00100208
#define DDR3L_512MB_EMIF_TIM1		0x0AAAE51B
#define DDR3L_512MB_EMIF_TIM2		0x246B7FDA
#define DDR3L_512MB_EMIF_TIM3		0x50FFE67F
#define DDR3L_512MB_EMIF_SDCFG		0x61A05332
#define DDR3L_512MB_EMIF_SDREF		0x100005A0
#define DDR3L_512MB_ZQ_CFG		0x5007A468
#define DDR3L_512MB_RATIO		0x00000100
#define DDR3L_512MB_INVERT_CLKOUT	0x00000001
#define DDR3L_512MB_RD_DQS		0x00000037
#define DDR3L_512MB_WR_DQS		0x00000081
#define DDR3L_512MB_PHY_FIFO_WE		0x000000F6
#define DDR3L_512MB_PHY_WR_DATA		0x000000C1
#define DDR3L_512MB_IOCTRL_VALUE	0x0000018B

#endif
