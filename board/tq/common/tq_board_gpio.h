/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2019 - 2022 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#ifndef __TQ_BOARD_GPIO_H__
#define __TQ_BOARD_GPIO_H__

#include <linux/stringify.h>

struct tq_gpio_init_data {
	const char *name;
	const char *label;
	const unsigned long flags;
	struct gpio_desc desc;
};

#if defined(CONFIG_DM_GPIO)

#define GPIO_INIT_DATA_ENTRY(IDX, NAME, FLAGS) \
	[IDX] = { \
		.name = (NAME), \
		.label = __stringify(IDX), \
		.flags = (FLAGS), \
	}

int tq_board_gpio_init(struct tq_gpio_init_data *data, int count);

#else
static inline int tq_board_gpio_init(struct tq_gpio_init_data *data,
				     int count)
{
	return 0;
}
#endif

#endif
