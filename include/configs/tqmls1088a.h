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
	"load mmc 0:1 ${dpaa2_firmware_addr_r} ${dpaa2_firmware};"\
	"load mmc 0:1 ${dpaa2_dpc_addr_r} ${dpaa2_dpc}; "	\
	"fsl_mc start mc ${dpaa2_firmware_addr_r} ${dpaa2_dpc_addr_r};\0"
#define QSPI_MC_INIT_CMD					\
	"sf probe 0:0;"						\
	"sf read ${dpaa2_firmware_addr_r} dpaa2-mc 0x200000;"	\
	"sf read ${dpaa2_dpc_addr_r} dpaa2-dpc 0x100000;"	\
	"fsl_mc start mc ${dpaa2_firmware_addr_r} ${dpaa2_dpc_addr_r}\0"

#undef CONFIG_EXTRA_ENV_SETTINGS
/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS				\
	TQMLS10XXA_COMMON_ENV					\
	"dpaa2_dpc=dpc.0x12_0x0D.dtb\0"				\
	"dpaa2_dpc_addr_r=0x80e00000\0"				\
	"dpaa2_dpl=dpl-eth.0x12_0x0D.dtb\0"			\
	"dpaa2_dpl_addr_r=0x80d00000\0"				\
	"dpaa2_firmware=mc.itb\0"				\
	"dpaa2_firmware_addr_r=0x80a00000\0"			\
	"pbl_mmc=bl2_auto.pbl\0"				\
	"ubimtdpart=7\0"					\
	"mcdplapply=load mmc 0:1 ${dpaa2_dpl_addr_r} ${dpaa2_dpl};"\
		"fsl_mc lazyapply DPL ${dpaa2_dpl_addr_r};\0"	\
	"mcdplspiapply=sf probe 0; sf read ${dpaa2_dpl_addr_r} dpaa2-dpl &&"\
		"fsl_mc lazyapply DPL ${dpaa2_dpl_addr_r};\0"	\
	"mmcpart=2\0"						\
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
	"netboot=echo Booting from net ...; "			\
		"run set_getcmd; "				\
		"setenv bootargs; "				\
		"run netargs; "					\
		"if ${getcmd} ${kernel_addr_r} ${kernel}; then "\
			"if ${getcmd} ${fdt_addr_r} ${fdtfile}; then "\
				"if ${getcmd} ${dpaa2_dpl_addr_r} ${dpaa2_dpl}; then "\
					"fsl_mc lazyappply DPL ${dpaa2_dpl_addr_r};"\
					"booti ${kernel_addr_r} - ${fdt_addr_r}; "\
				"fi; "				\
			"fi; "					\
		"fi; "						\
		"echo ... failed\0"				\
	"update_fmucode_mmc=\0"					\
	"update_fmucode_spi=\0"				\
	"update_dpaa2_firmware_mmc=run set_getcmd;"		\
		"if ${getcmd} ${dpaa2_firmware}; then "		\
			"if itest ${filesize} > 0; then "	\
				"save mmc 0:1 ${loadaddr} ${dpaa2_firmware} ${filesize};"\
			"fi;"					\
		"fi;"						\
		"setenv filesize;\0"				\
	"update_dpaa2_dpc_mmc=run set_getcmd;"			\
		"if ${getcmd} ${dpaa2_dpc}; then "		\
			"if itest ${filesize} > 0; then "	\
				"save mmc 0:1 ${loadaddr} ${dpaa2_dpc} ${filesize};"\
			"fi;"					\
		"fi;"						\
		"setenv filesize;\0"				\
	"update_dpaa2_dpl_mmc=run set_getcmd;"			\
		"if ${getcmd} ${dpaa2_dpl}; then "		\
			"if itest ${filesize} > 0; then "	\
				"save mmc 0:1 ${loadaddr} ${dpaa2_dpl} ${filesize};"\
			"fi;"					\
		"fi;"						\
		"setenv filesize;\0"				\
	"update_dpaa2_firmware_spi=run set_getcmd;"		\
		"if ${getcmd} ${dpaa2_firmware}; then "		\
			"if itest ${filesize} > 0; then "	\
				"sf probe 0; "			\
				"sf update ${loadaddr} dpaa2-mc ${filesize}; "	\
			"fi;"					\
		"fi;"						\
		"setenv filesize;\0"				\
	"update_dpaa2_dpc_spi=run set_getcmd;"			\
		"if ${getcmd} ${dpaa2_dpc}; then "		\
			"if itest ${filesize} > 0; then "	\
				"sf probe 0; "			\
				"sf update ${loadaddr} dpaa2-dpc ${filesize}; "	\
			"fi;"					\
		"fi;"						\
		"setenv filesize;\0"				\
	"update_dpaa2_dpl_spi=run set_getcmd;"			\
		"if ${getcmd} ${dpaa2_dpl}; then "		\
			"if itest ${filesize} > 0; then "	\
				"sf probe 0; "			\
				"sf update ${loadaddr} dpaa2-dpl ${filesize}; "	\
			"fi;"					\
		"fi;"						\
		"setenv filesize;\0"

#include <asm/fsl_secure_boot.h>

#include "tqmls1088a_baseboard.h"

#endif /* __TQMLS1088A_H__ */
