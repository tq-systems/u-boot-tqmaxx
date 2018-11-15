/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 TQ Systems GmbH
 * Max Merchel <Max.Merchel@tq-group.com>
 */
#ifndef __TQMLS1012AL_BB_H__
#define __TQMLS1012AL_BB_H__

#include <common.h>
#include <i2c.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/fsl_serdes.h>
#ifdef CONFIG_FSL_LS_PPA
#include <asm/arch/ppa.h>
#endif
#include <asm/arch/mmu.h>
#include <asm/arch/soc.h>
#include <hwconfig.h>
#include <ahci.h>
#include <mmc.h>
#include <scsi.h>
#include <fsl_esdhc.h>
#include <environment.h>
#include <fsl_mmdc.h>
#include <netdev.h>
#include <fsl_sec.h>
#include <spl.h>
#include "../common/sleep.h"
#include "../common/tqmaxx_eeprom.h"

extern int phy_read_mmd_indirect(struct phy_device *phydev, int prtad,
				 int devad, int addr);
extern void phy_write_mmd_indirect(struct phy_device *phydev, int prtad,
				   int devad, int addr, u32 data);

int board_eth_init(bd_t *bis);
void tqmls1012al_bb_early_init(void);
void tqmls1012al_bb_late_init(void);
#endif /* __TQMLS1012AL_BB_H__ */
