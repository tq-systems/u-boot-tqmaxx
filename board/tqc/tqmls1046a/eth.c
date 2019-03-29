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
#include "../common/tqc_eeprom.h"

#ifndef CONFIG_SPL_BUILD
int board_eth_init(bd_t *bis)
{
	struct tqc_eeprom_data eedat;
	int ret, iface, j;

	/* set MAC addresses based on EEPROM content */
	ret = tqc_read_eeprom(CONFIG_SYS_EEPROM_BUS_NUM,
			CONFIG_SYS_I2C_EEPROM_ADDR, &eedat);

	if(!ret) {
		for(iface = 0; iface <= FM1_DTSEC10; iface++) {
			eth_env_set_enetaddr_by_index("eth", iface, eedat.mac);

			/* increment MAC with overflow */
			j = sizeof(eedat.mac);
			do {
				j--;
				eedat.mac[j]++;
			} while(eedat.mac[j] == 0 && j > 0);

		}
	} else {
		printf("Could not read EEPROM! err: %d\n", ret);
	}

	ret = tqmls1046a_bb_board_eth_init(bis);
	if(ret != 0) {
		return ret;
	}

#ifdef CONFIG_FMAN_ENET
	cpu_eth_init(bis);
#endif

	return pci_eth_init(bis);
}
#endif
