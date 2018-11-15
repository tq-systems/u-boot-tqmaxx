/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 TQ Systems GmbH
 * Max Merchel <Max.Merchel@tq-group.com>
 */

#include "tqmls1012al_bb.h"
#include <asm/gpio.h>
#include <dm.h>
#include <fm_eth.h>
#include <fsl_mdio.h>
#include <malloc.h>
#include <asm/types.h>
#include <fsl_dtsec.h>
#include <asm/arch-fsl-layerscape/config.h>
#include <asm/arch-fsl-layerscape/immap_lsch2.h>
#include <pfe_eth/pfe_eth.h>

int checkboard(void)
{
	puts("Board: MBLS1012AL\n");

	return 0;
}

#ifdef CONFIG_TSEC_ENET
int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}
#endif

void tqmls1012al_bb_late_init(void)
{
}
