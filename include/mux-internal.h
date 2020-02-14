/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#ifndef _MUX_UCLASS_H
#define _MUX_UCLASS_H

/* See mux.h for background documentation. */

#include <mux.h>

struct ofnode_phandle_args;

/**
 * struct mux_control_ops -	Mux controller operations for a mux chip.
 * @set:			Set the state of the given mux controller.
 */
struct mux_control_ops {
	int (*set)(struct mux_control *mux, int state);
	int (*of_xlate)(struct mux_control *mux,
			struct ofnode_phandle_args *args);
};

/**
 * struct mux_control -	Represents a mux controller.
 * @chip:		The mux chip that is handling this mux controller.
 * @cached_state:	The current mux controller state, or -1 if none.
 * @states:		The number of mux controller states.
 * @idle_state:		The mux controller state to use when inactive, or one
 *			of MUX_IDLE_AS_IS and MUX_IDLE_DISCONNECT.
 *
 * Mux drivers may only change @states and @idle_state, and may only do so
 * between allocation and registration of the mux controller. Specifically,
 * @cached_state is internal to the mux core and should never be written by
 * mux drivers.
 */
struct mux_control {
	bool	in_use;
	struct udevice *dev;
	int cached_state;
	unsigned int states;
	int idle_state;
	int id;
};

/**
 * struct mux_chip -	Represents a chip holding mux controllers.
 * @controllers:	Number of mux controllers handled by the chip.
 * @mux:		Array of mux controllers that are handled.
 * @dev:		Device structure.
 * @ops:		Mux controller operations.
 */
struct mux_chip {
	unsigned int controllers;
	struct mux_control *mux;
};

/**
 * mux_control_get_index() - Get the index of the given mux controller
 * @mux: The mux-control to get the index for.
 *
 * Return: The index of the mux controller within the mux chip the mux
 * controller is a part of.
 */
static inline unsigned int mux_control_get_index(struct mux_control *mux)
{
	return mux->id;
}

int mux_alloc_controllers(struct udevice *dev, unsigned int controllers);

#endif
