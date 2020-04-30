// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2019-2022 TQ-Systems GmbH
 * Authors: Markus Niebel <Markus.Niebel@tq-group.com>
 *          Matthias Schiffer <matthias.schiffer@tq.tq-group.com>
 */

#include <common.h>
#include "board_gpio.h"

void tq_board_gpio_init(struct tq_board_gpio_data *data, int count)
{
	int ret, i;

	for (i = 0; i < count; i++) {
		ret = dm_gpio_lookup_name(data[i].name, &data[i].desc);
		if (ret) {
			printf("Error: GPIO lookup %s: %d\n",
			       data[i].name, ret);
			continue;
		}

		ret = dm_gpio_request(&data[i].desc, data[i].label);
		if (ret) {
			printf("Error: GPIO request %s: %d",
			       data[i].label, ret);
			continue;
		}

		dm_gpio_set_dir_flags(&data[i].desc, data[i].flags);
	}
}
