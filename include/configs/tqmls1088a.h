/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2019-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Timo Herbrecher, Gregor Herburger
 */

#ifndef __TQMLS1088A_H__
#define __TQMLS1088A_H__

#include "ls1088a_common.h"
#include "tqmls10xxa_common.h"

#define CONFIG_LAYERSCAPE_NS_ACCESS

#define CONFIG_SYS_LS_MC_BOOT_TIMEOUT_MS 5000
#define COUNTER_FREQUENCY_REAL		25000000	/* 25MHz */
#define COUNTER_FREQUENCY		25000000	/* 25MHz */

#define SD_MC_INIT_CMD						\
	"mmc rescan; "						\
	"load mmc 0:1 0x80a00000 mc.itb; "			\
	"load mmc 0:1 0x80e00000 dpc.0x12_0x0D.dtb; "		\
	"fsl_mc start mc 0x80a00000 0x80e00000;\0"
#define QSPI_MC_INIT_CMD					\
	"sf probe 0:0;sf read 0x80a00000 dpaa2-mc 0x200000;"	\
	"sf read 0x80e00000 dpaa2-dpc 0x100000;"		\
	"fsl_mc start mc 0x80a00000 0x80e00000\0"

#undef CONFIG_EXTRA_ENV_SETTINGS
/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS				\
	TQMLS10XXA_COMMON_ENV					\
	"pbl_mmc=bl2_auto.pbl\0"				\
	"ubimtdpart=7\0"					\
	"mcdplapply=load mmc 0:1 0x80a00000 dpl-eth.0x12_0x0D.dtb;"\
	"fsl_mc lazyapply DPL 0x80a00000;\0"			\
	"mcdplspiapply=sf probe 0; sf read 0x80a00000 dpaa2-dpl &&"\
	"fsl_mc lazyapply dpl 0x80a00000;\0"			\
	"mmcpart=2\0"	\
	"mmcboot=echo Booting from mmc ...; "			\
		"setenv bootargs; "				\
		"run mmcargs; "					\
		"run loadimage; "				\
		"run loadfdt; "					\
		"run mcdplapply; "				\
		"booti ${kernel_addr_r} - ${fdt_addr_r};\0"     \
	"spiboot=echo Booting from SPI NOR flash ...;"		\
		"setenv bootargs; "				\
		"run spiargs; "					\
		"run loadspiimage; "				\
		"run loadspifdt; "				\
		"run mcdplspiapply; "				\
		"booti ${kernel_addr_r} - ${fdt_addr_r};\0"	\

#include <asm/fsl_secure_boot.h>

#include "tqmls1088a_baseboard.h"

#endif /* __TQMLS1088A_H__ */
