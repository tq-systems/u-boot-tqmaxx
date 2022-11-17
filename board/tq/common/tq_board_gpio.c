// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 - 2022 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <common.h>
#include <asm/gpio.h>
#include "tq_board_gpio.h"

int tq_board_gpio_init(struct tq_gpio_init_data *data, int count)
{
	int ret;
	int i;

	for (i = 0; i < count; ++i) {
		ret = dm_gpio_lookup_name(data[i].name, &data[i].desc);
		if (ret) {
			printf("error: gpio lookup %s\n", data[i].name);
		} else {
			ret = dm_gpio_request(&data[i].desc, data[i].label);
			if (ret)
				printf("error: gpio REQ %s\n", data[i].label);
			else
				dm_gpio_set_dir_flags(&data[i].desc,
						      data[i].flags);
		}
	}

	return 0;
}
