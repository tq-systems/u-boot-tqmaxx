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

int get_gpio_number(const char *gpio_name, unsigned int *gpio_number)
{
	int ret = 0;

#if defined(CONFIG_DM_GPIO)
	/*
	 * TODO:Once all GPIO drivers are converted to driver model,
	 * we can change the code here to use the GPIO uclass interface.
	 */
	ret = gpio_lookup_name(gpio_name, NULL, NULL, gpio_number);
#else
	/* turn the gpio name into a gpio number */
	gpio_number = name_to_gpio(gpio_name);
#endif

	return ret;
}

#define MASK_ETH_PHY_RST	0x00000100

void reset_phy(void)
{
	/* No PHY reset control from LS1012A */
}

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_FSL_PFE
	return cpu_eth_init(bis);
#endif
	return pci_eth_init(bis);
}

void tqmls1012al_bb_late_init(void)
{
	/* USB */
	unsigned int gpio_number;
	const char *gpio_name_usb = "gpio@20_6";

	if (get_gpio_number(gpio_name_usb, &gpio_number)) {
		printf("USB: GPIO '%s' not found\n", gpio_name_usb);
	} else {
		gpio_request(gpio_number, "USB_rst");
		gpio_direction_output(gpio_number, 1);
	}
}
