/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2021-2025 TQ-Systems GmbH <u-boot@ew.tq-group.com>, D-82229 Seefeld, Germany.
 * Authors: Gregor Herburger, Matthias Schiffer
 *
 * Based on:
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 */

#ifndef __CONFIG_AM335X_MBA335X_H
#define __CONFIG_AM335X_MBA335X_H

#include "tqma335x.h"

#ifndef CONFIG_SPL_BUILD

/* USB Device Firmware Update support */
#include <environment/ti/dfu.h>
#define DFUARGS \
	DFU_ALT_INFO_EMMC \
	DFU_ALT_INFO_MMC \
	DFU_ALT_INFO_RAM

#define CONFIG_EXTRA_ENV_SETTINGS \
	TQMA335X_ENV_SETTINGS \
	"console=ttyS4,115200n8\0" \
	DFUARGS \
	"upd_uboot_emmc_net=setenv mmcdev 0 && run update_uboot_mmc\0" \
	"upd_uboot_sd_net=setenv mmcdev 1 && run update_uboot_mmc\0" \
	""

#endif

#endif	/* ! __CONFIG_AM335X_MBA335X_H */
