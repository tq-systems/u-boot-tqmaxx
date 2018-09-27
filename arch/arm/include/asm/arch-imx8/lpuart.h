/*
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 */
#ifndef __ASM_ARCH_IMX8_LPUART_H__
#define __ASM_ARCH_IMX8_LPUART_H__

#include <asm/mach-imx/sci/sci.h>

struct imx_lpuart_map {
	void *base;
	sc_rsrc_t rsrc;
};

static struct imx_lpuart_map imx_lpuart_desc[] = {
	{(void*)LPUART1_BASE, SC_R_UART_0},
	{(void*)LPUART2_BASE, SC_R_UART_1},
	{(void*)LPUART3_BASE, SC_R_UART_2},
	{(void*)LPUART4_BASE, SC_R_UART_3},
#ifdef CONFIG_IMX8QM
	{(void*)LPUART5_BASE, SC_R_UART_4},
#endif
};
#endif /* __ASM_ARCH_IMX8_LPUART_H__ */
