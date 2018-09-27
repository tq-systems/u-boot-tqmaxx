/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 TQ-Systems GmbH
 */

#ifndef __TQMLS1046A_H__
#define __TQMLS1046A_H__

#include "ls1046a_common.h"

#define CONFIG_SYS_CLK_FREQ		100000000
#define CONFIG_DDR_CLK_FREQ		100000000

#define CONFIG_LAYERSCAPE_NS_ACCESS
#define CONFIG_MISC_INIT_R

#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		0x9fffffff

#define CONFIG_DIMM_SLOTS_PER_CTLR		1
/* Physical Memory Map */
#define CONFIG_CHIP_SELECTS_PER_CTRL	4
#define CONFIG_NR_DRAM_BANKS			1

#define CONFIG_SYS_DDR_RAW_TIMING
#define CONFIG_FSL_DDR_INTERACTIVE

#define CONFIG_DDR_ECC
#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER
#define CONFIG_MEM_INIT_VALUE           0xdeadbeef
#define CONFIG_FSL_DDR_BIST	/* enable built-in memory test */

#ifdef CONFIG_RAMBOOT_PBL
#define CONFIG_SYS_FSL_PBL_PBI board/tqc/tqmls1046a/tqmls1046a_pbi.cfg
#endif

#ifdef CONFIG_SD_BOOT
#define CONFIG_SYS_FSL_PBL_RCW board/tqc/tqmls1046a/tqmls1046a_rcw_sd.cfg
#endif

#ifndef SPL_NO_IFC
/* IFC */
#define CONFIG_FSL_IFC
#endif

/* EEPROM */
#define CONFIG_SYS_EEPROM_BUS_NUM				0
#define CONFIG_SYS_I2C_EEPROM_ADDR				0x50
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN			1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS		3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	5

/* PMIC */
#define CONFIG_POWER
#ifdef CONFIG_POWER
#define CONFIG_POWER_I2C
#endif

/*
 * Environment
 */
#ifndef SPL_NO_ENV
#define CONFIG_ENV_OVERWRITE
#endif

#if defined(CONFIG_SD_BOOT)
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_ENV_OFFSET			(3 * 1024 * 1024)
#define CONFIG_ENV_SIZE				0x2000
#else
#define CONFIG_ENV_SIZE				0x2000		/* 8KB */
#define CONFIG_ENV_OFFSET			0x300000	/* 3MB */
#define CONFIG_ENV_SECT_SIZE		0x40000		/* 256KB */
#endif

/* FMan */
#ifndef SPL_NO_FMAN
#ifdef CONFIG_SYS_DPAA_FMAN
#define CONFIG_FMAN_ENET
#define FDT_SEQ_MACADDR_FROM_ENV
#endif
#endif

/* QSPI device */
#ifndef SPL_NO_QSPI
#ifdef CONFIG_FSL_QSPI
#define CONFIG_SPI_FLASH_MACRONIX
#define FSL_QSPI_FLASH_SIZE			(1 << 26)
#define FSL_QSPI_FLASH_NUM			2
#define CONFIG_SPI_FLASH_BAR
#endif
#endif

#ifndef SPL_NO_MISC
#undef CONFIG_BOOTCOMMAND
#if defined(CONFIG_QSPI_BOOT)
#define CONFIG_BOOTCOMMAND "run distro_bootcmd; run qspi_bootcmd; "	\
			   "env exists secureboot && esbc_halt;;"
#elif defined(CONFIG_SD_BOOT)
#define CONFIG_BOOTCOMMAND "run distro_bootcmd;run sd_bootcmd; "	\
			   "env exists secureboot && esbc_halt;"
#endif
#endif

#include <asm/fsl_secure_boot.h>

#include "tqmls1046a_baseboard.h"

#endif /* __TQMLS1046A_H__ */
