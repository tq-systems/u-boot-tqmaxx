/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 TQ Systems GmbH
 * Max Merchel <Max.Merchel@tq-group.com>
 */

#ifndef __TQMLS1012AL_CONFIG_H__
#define __TQMLS1012AL_CONFIG_H__

#include "ls1012a_common.h"

#undef CONFIG_SYS_I2C

/* CONFIG_SYS_MALLOC_LEN >= 512k for UBI! */
#undef CONFIG_SYS_MALLOC_LEN
#define CONFIG_SYS_MALLOC_LEN		(512u * 1024)
/* CONFIG_SYS_TEXT_BASE points to start of U-Boot */
#undef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE		0x40010000

#define CONFIG_MISC_INIT_R

/* FLASH */
#define CONFIG_MTD_PARTITIONS
#define FSL_QSPI_QUAD_MODE
#define CONFIG_SYS_FSL_QSPI_AHB

/* DDR */
#define CONFIG_DIMM_SLOTS_PER_CTLR	1
#define CONFIG_CHIP_SELECTS_PER_CTRL	1
#define CONFIG_NR_DRAM_BANKS		1
/* TQMLS1012AL-PROTO1: 256 MB
 * TQMLS1012AL-PROTO2: 512 MB
 */
#define CONFIG_SYS_SDRAM_SIZE		0x10000000
#define CONFIG_CMD_MEMINFO
#define CONFIG_CMD_MEMTEST
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		0x9fffffff

#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS              \
       "verify=no\0"                           \
       "loadaddr=0x82000000\0"                 \
       "kernel_addr=0x100000\0"                \
       "fdt_high=0xffffffffffffffff\0"         \
       "initrd_high=0xffffffffffffffff\0"      \
       "kernel_start=0xa00000\0"               \
       "kernel_load=0x96000000\0"              \
       "kernel_size=0x2800000\0"

#define CONFIG_SYS_FSL_PBL_RCW	board/tqc/tqmls1012al/ls1012a_rcw_qspi.cfg
#define CONFIG_SYS_FSL_PBL_PBI	board/tqc/tqmls1012al/ls1012a_pbi_qspi.cfg
#define CONFIG_SPL_PAD_TO		0x10000

/*
 * All the defines above are for the TQMLS1012al SoM
 *
 * Now include the baseboard specific configuration
 */
#ifdef CONFIG_MBLS1012AL
#include "tqmls1012al_mbls1012al.h"
#else
#error "No baseboard for the TQMLS1012AL SOM defined!"
#endif

#endif /* __TQMLS1012AL_CONFIG_H__ */
