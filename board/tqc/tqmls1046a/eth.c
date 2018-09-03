// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 TQ-Systems GmbH
 */
#include <common.h>
#include <asm/io.h>
#include <netdev.h>
#include <fm_eth.h>
#include <fsl_dtsec.h>
#include <fsl_mdio.h>
#include <malloc.h>
#include "tqmls1046a_bb.h"

int board_eth_init(bd_t *bis)
{
	int ret;

	ret = tqmls1046a_bb_board_eth_init(bis);
	if(ret != 0) {
		return ret;
	}

#ifdef CONFIG_FMAN_ENET
	cpu_eth_init(bis);
#endif

	return pci_eth_init(bis);
}

#ifdef CONFIG_FMAN_ENET
int fdt_update_ethernet_dt(void *blob)
{
	/* TODO: add device-tree fixup for ethernet devices */ 

	return 0;
}
#endif
