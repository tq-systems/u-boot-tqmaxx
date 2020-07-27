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
#define MAIN_DEVSTAT_BKUP_MMC_PORT_MASK			BIT(7)
#define MAIN_DEVSTAT_BKUP_MMC_PORT_SHIFT		7

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

#define QOS_ATYPE_MASK			0x30000000
#define QOS_VIRTID_MASK			0x0fff0000
#define QOS_PVU_CTX(virtid)		((0x1 << 28) | (virtid << 16))
#define QOS_SMMU_CTX(virtid)		((0x2 << 28) | (virtid << 16))

/* ROM HANDOFF Structure location */
#define ROM_ENTENDED_BOOT_DATA_INFO			0x41cffb00

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

#define QOS_MMC0_RD_CBASS_MAP(i)		(0x45d9a100 + (i) * 4)
#define QOS_MMC0_WR_CBASS_MAP(i)		(0x45d9a500 + (i) * 4)
#define QOS_MMC1_RD_CBASS_MAP(i)		(0x45d82100 + (i) * 4)
#define QOS_MMC1_WR_CBASS_MAP(i)		(0x45d82500 + (i) * 4)

#define QOS_GPU_M0_RD_CBASS_MAP(i)		(0x45dc5100 + (i) * 4)
#define QOS_GPU_M0_WR_CBASS_MAP(i)		(0x45dc5900 + (i) * 4)
#define QOS_GPU_M1_RD_CBASS_MAP(i)		(0x45dc6100 + (i) * 4)
#define QOS_GPU_M1_WR_CBASS_MAP(i)		(0x45dc6900 + (i) * 4)

#define QOS_D5520_RD_CBASS_MAP(i)		(0x45dc0500 + (i) * 4)
#define QOS_D5520_WR_CBASS_MAP(i)		(0x45dc0900 + (i) * 4)

/* NAVSS North Bridge (NB) */
#define NAVSS0_NBSS_NB0_CFG_MMRS		0x3802000
#define NAVSS0_NBSS_NB1_CFG_MMRS		0x3803000
#define NAVSS0_NBSS_NB0_CFG_NB_THREADMAP	(NAVSS0_NBSS_NB0_CFG_MMRS + 0x10)
#define NAVSS0_NBSS_NB1_CFG_NB_THREADMAP	(NAVSS0_NBSS_NB1_CFG_MMRS + 0x10)

#endif /* __ASM_ARCH_J721E_HARDWARE_H */
