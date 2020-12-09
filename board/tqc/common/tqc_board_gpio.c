// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2019 TQ-Systems GmbH
 * Markus Niebel <Markus.Niebel@tq-group.com>
 */

#include <common.h>
#include <asm/gpio.h>
#include "tqc_board_gpio.h"

int tqc_board_gpio_init(struct tqc_gpio_init_data *data, int count)
{
	int i;
	int ret;

	for (i = 0; i < count; ++i) {
		ret = dm_gpio_lookup_name(data[i].name, &data[i].desc);
		if (ret) {
			printf("error: gpio lookup %s", data[i].name);
		} else {
			ret = dm_gpio_request(&data[i].desc, data[i].label);
			if (ret)
				printf("error: gpio REQ %s", data[i].label);
			else
				dm_gpio_set_dir_flags(&data[i].desc,
						      data[i].flags);
		}
	}

	return 0;
}
