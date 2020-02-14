/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#ifndef _MUX_H_
#define _MUX_H_

#include <linux/errno.h>
#include <linux/types.h>

struct udevice;
struct mux_control;

#if CONFIG_IS_ENABLED(MULTIPLEXER)
/**
 * mux_control_states() - Query the number of multiplexer states.
 * @mux: The mux-control to query.
 *
 * Return: The number of multiplexer states.
 */
unsigned int mux_control_states(struct mux_control *mux);

/**
 * mux_control_select() - Select the given multiplexer state.
 * @mux: The mux-control to request a change of state from.
 * @state: The new requested state.
 *
 * On successfully selecting the mux-control state, it will be locked until
 * there is a call to mux_control_deselect(). If the mux-control is already
 * selected when mux_control_select() is called, the function will indicate
 * -EBUSY
 *
 * Therefore, make sure to call mux_control_deselect() when the operation is
 * complete and the mux-control is free for others to use, but do not call
 * mux_control_deselect() if mux_control_select() fails.
 *
 * Return: 0 when the mux-control state has the requested state or a negative
 * errno on error.
 */
int __must_check mux_control_select(struct mux_control *mux,
				    unsigned int state);
#define mux_control_try_select(mux) mux_control_select(mux)

/**
 * mux_control_deselect() - Deselect the previously selected multiplexer state.
 * @mux: The mux-control to deselect.
 *
 * It is required that a single call is made to mux_control_deselect() for
 * each and every successful call made to either of mux_control_select() or
 * mux_control_try_select().
 *
 * Return: 0 on success and a negative errno on error. An error can only
 * occur if the mux has an idle state. Note that even if an error occurs, the
 * mux-control is unlocked and is thus free for the next access.
 */
int mux_control_deselect(struct mux_control *mux);

int mux_get_by_index(struct udevice *dev, int index, struct mux_control *mux);
int mux_control_get(struct udevice *dev, const char *name,
		    struct mux_control *mux);

void mux_control_put(struct mux_control *mux);

struct mux_control *devm_mux_control_get(struct udevice *dev,
					 const char *mux_name);

void dm_mux_init(void);
#else
unsigned int mux_control_states(struct mux_control *mux)
{
	return -ENOSYS;
}

int __must_check mux_control_select(struct mux_control *mux,
				    unsigned int state)
{
	return -ENOSYS;
}

#define mux_control_try_select(mux) mux_control_select(mux)

int mux_control_deselect(struct mux_control *mux)
{
	return -ENOSYS;
}

struct mux_control *mux_control_get(struct udevice *dev, const char *mux_name)
{
	return NULL;
}

void mux_control_put(struct mux_control *mux)
{
}

struct mux_control *devm_mux_control_get(struct udevice *dev,
					 const char *mux_name)
{
	return NULL;
}

void dm_mux_init(void)
{
}
#endif

#endif
