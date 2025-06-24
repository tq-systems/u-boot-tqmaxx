/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2022-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Paul Gerber
 */

#ifndef __TQMA8MPXS_H
#define __TQMA8MPXS_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include <asm/arch/imx-regs.h>

#define CFG_SYS_UBOOT_BASE	\
	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#if defined(CONFIG_IMX_BOOTAUX)
#define CFG_CORTEXM_ENV_SETTINGS                                       \
	"cortexm_image=demo.bin\0"                                     \
	"cortexm_load_addr=0x7e0000\0"                                 \
	"cortexm_start_addr=0x7e0000\0"                                \
	"cortexm_boot_mmc=mmc dev ${mmcdev}; mmc rescan; "             \
		"load mmc ${mmcdev}:${mmcfwpart} ${loadaddr} "         \
			"${mmcfwpath}${cortexm_image} && "             \
		"cp.b ${loadaddr} ${cortexm_load_addr} ${filesize} && "\
		"dcache flush && "                                     \
		"bootaux ${cortexm_start_addr} 0;\0"

#else
#define CFG_CORTEXM_ENV_SETTINGS
#endif

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
#if defined(CONFIG_TQMA8MPXS_BB_MB_SMARC_2)
#include "tqma8mpxs-mb-smarc-2.h"
#else
#error "no mainboard variant selected"
#endif

#endif /* __TQMA8MPXS_H */
