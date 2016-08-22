/*
 * Copyright 2016 TQ Systems GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __TQMLS102XA_BB_H__
#define __TQMLS102XA_BB_H__
#include <common.h>
#include <i2c.h>
#include <asm/io.h>
#include <asm/arch/immap_ls102xa.h>
#include <asm/arch/clock.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/ls102xa_soc.h>
#include <asm/arch/ls102xa_sata.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <mmc.h>
#include <fsl_csu.h>
#include <fsl_esdhc.h>
#include <fsl_immap.h>
#include <fsl_ifc.h>
#include <netdev.h>
#include <fsl_mdio.h>
#include <tsec.h>
#include <fsl_sec.h>
#include <spl.h>
#include "../common/sleep.h"
#ifdef CONFIG_U_QE
#include <fsl_qe.h>
#endif
#include "tqmls102xa_eeprom.h"

extern int phy_read_mmd_indirect(struct phy_device *phydev, int prtad,
			  int devad, int addr);
extern void phy_write_mmd_indirect(struct phy_device *phydev, int prtad,
			    int devad, int addr, u32 data);
#define DP83867_DEVADDR		0x1f
int board_eth_init(bd_t *bis);
void tqmls102xa_bb_early_init(void);
void tqmls102xa_bb_late_init(void);
#endif /* __TQMLS102XA_BB_H__ */
