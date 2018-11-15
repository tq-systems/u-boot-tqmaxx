/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Dummy header file to enable CONFIG_DM_GPIO.
 * If CONFIG_OF_CONTROL is enabled, lib/fdtdec.c is compiled.
 * It includes <asm/arch/gpio.h> via <asm/gpio.h>, so those SoCs that enable
 * OF_CONTROL must have arch/gpio.h.
 */

#ifndef __ASM_ARCH_LS1012A_GPIO_H_
#define __ASM_ARCH_LS1012A_GPIO_H_

#endif
