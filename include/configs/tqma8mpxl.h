/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2022-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#ifndef __TQMA8MPXL_H
#define __TQMA8MPXL_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include <asm/arch/imx-regs.h>

#define CFG_SYS_UBOOT_BASE	\
	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#if !defined(CONFIG_BOOTCOMMAND)
#warn "CONFIG_BOOTCOMMAND missing in configuration"
#endif

/* Link Definitions */
#if defined(CONFIG_BLOBLIST_SIZE)
#define INIT_RAM_OFFSET			(CONFIG_BLOBLIST_SIZE)
#else
#define INIT_RAM_OFFSET			0x0
#endif
#define CFG_SYS_INIT_RAM_ADDR		(0x40000000 + INIT_RAM_OFFSET)
#define CFG_SYS_INIT_RAM_SIZE		0x200000

#define CFG_SYS_SDRAM_BASE		0x40000000
#define PHYS_SDRAM			0x40000000
#define PHYS_SDRAM_SIZE			SZ_1G /* 1GB DDR */
#define PHYS_SDRAM_2_SIZE		0x00000000	/* 0 GB */

/* both variants use the same settings */
#if defined(CONFIG_TQMA8MPXL_BB_MBA8MPXL)
#include "tqma8mpxl-mba8mpxl.h"
#elif defined(CONFIG_TQMA8MPXL_BB_MBA8MP_RAS314)
#include "tqma8mpxl-mba8mp-ras314.h"
#else
#error "no mainboard variant selected"
#endif

#endif /* __TQMA8MPXL_H */
