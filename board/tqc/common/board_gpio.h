// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2019-2022 TQ-Systems GmbH
 * Authors: Markus Niebel <Markus.Niebel@tq-group.com>
 *          Matthias Schiffer <matthias.schiffer@tq.tq-group.com>
 */

#ifndef __TQ_COMMON_BOARD_GPIO_H__
#define __TQ_COMMON_BOARD_GPIO_H__

#include <asm/gpio.h>

struct tq_board_gpio_data {
	const char *name;
	const char *label;
	const unsigned long flags;
	struct gpio_desc desc;
};

#define GPIO_INIT_DATA_ENTRY(IDX, NAME, FLAGS) \
	[IDX] = { \
		.name = (NAME), \
		.label = __stringify(IDX), \
		.flags = (FLAGS), \
	}

void tq_board_gpio_init(struct tq_board_gpio_data *data, int count);

#endif
