/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * K3: J721E SoC definitions, structures etc.
 *
 * (C) Copyright (C) 2018-2019 Texas Instruments Incorporated - http://www.ti.com/
 */
#ifndef __ASM_ARCH_J721E_HARDWARE_H
#define __ASM_ARCH_J721E_HARDWARE_H

#include <config.h>

#define CTRL_MMR0_BASE					0x00100000
#define CTRLMMR_MAIN_DEVSTAT				(CTRL_MMR0_BASE + 0x30)

#define MAIN_DEVSTAT_BOOT_MODE_B_MASK		BIT(0)
#define MAIN_DEVSTAT_BOOT_MODE_B_SHIFT		0
#define MAIN_DEVSTAT_BKUP_BOOTMODE_MASK		GENMASK(3, 1)
#define MAIN_DEVSTAT_BKUP_BOOTMODE_SHIFT	1
#define MAIN_DEVSTAT_PRIM_BOOTMODE_MMC_PORT_MASK	BIT(6)
#define MAIN_DEVSTAT_PRIM_BOOTMODE_PORT_SHIFT		6

#define WKUP_CTRL_MMR0_BASE				0x43000000
#define MCU_CTRL_MMR0_BASE				0x40f00000

#define CTRLMMR_WKUP_DEVSTAT			(WKUP_CTRL_MMR0_BASE + 0x30)
#define WKUP_DEVSTAT_PRIMARY_BOOTMODE_MASK	GENMASK(5, 3)
#define WKUP_DEVSTAT_PRIMARY_BOOTMODE_SHIFT	3
#define WKUP_DEVSTAT_MCU_OMLY_MASK		BIT(6)
#define WKUP_DEVSTAT_MCU_ONLY_SHIFT		6

/*
 * The CTRL_MMR0 memory space is divided into several equally-spaced
 * partitions, so defining the partition size allows us to determine
 * register addresses common to those partitions.
 */
#define CTRL_MMR0_PARTITION_SIZE			0x4000

/*
 * CTRL_MMR0, WKUP_CTRL_MMR0, and MCU_CTR_MMR0 lock/kick-mechanism
 * shared register definitions.
 */
#define CTRLMMR_LOCK_KICK0				0x01008
#define CTRLMMR_LOCK_KICK0_UNLOCK_VAL			0x68ef3490
#define CTRLMMR_LOCK_KICK0_UNLOCKED_MASK		BIT(0)
#define CTRLMMR_LOCK_KICK0_UNLOCKED_SHIFT		0
#define CTRLMMR_LOCK_KICK1				0x0100c
#define CTRLMMR_LOCK_KICK1_UNLOCK_VAL			0xd172bc5a

/* MCU SCRATCHPAD usage */
#define TI_SRAM_SCRATCH_BOARD_EEPROM_START	CONFIG_SYS_K3_MCU_SCRATCHPAD_BASE

/* CBASS */
#define QOS_DSS0_DMA				0x45dc2000
#define QOS_DSS0_DMA_CBASS_GRP_MAP1(j)		(QOS_DSS0_DMA + 0x0 + (j) * 8)
#define QOS_DSS0_DMA_CBASS_GRP_MAP2(j)		(QOS_DSS0_DMA + 0x4 + (j) * 8)
#define QOS_DSS0_DMA_CBASS_MAP(i)		(QOS_DSS0_DMA + 0x100 + (i) * 4)

#define QOS_DSS0_FBDC				0x45dc2400
#define QOS_DSS0_FBDC_CBASS_GRP_MAP1(j)		(QOS_DSS0_FBDC + 0x0 + (j) * 8)
#define QOS_DSS0_FBDC_CBASS_GRP_MAP2(j)		(QOS_DSS0_FBDC + 0x4 + (j) * 8)
#define QOS_DSS0_FBDC_CBASS_MAP(i)		(QOS_DSS0_FBDC + 0x100 + (i) * 4)

/* NAVSS North Bridge (NB) */
#define NAVSS0_NBSS_NB0_CFG_MMRS		0x3802000
#define NAVSS0_NBSS_NB1_CFG_MMRS		0x3803000
#define NAVSS0_NBSS_NB0_CFG_NB_THREADMAP	(NAVSS0_NBSS_NB0_CFG_MMRS + 0x10)
#define NAVSS0_NBSS_NB1_CFG_NB_THREADMAP	(NAVSS0_NBSS_NB1_CFG_MMRS + 0x10)

#endif /* __ASM_ARCH_J721E_HARDWARE_H */
