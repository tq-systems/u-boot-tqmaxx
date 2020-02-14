// SPDX-License-Identifier: GPL-2.0
/*
 * Multiplexer subsystem
 *
 * Based on the linux multiplexer framework
 *
 * Copyright (C) 2017 Axentia Technologies AB
 * Author: Peter Rosin <peda@axentia.se>
 *
 * Copyright (C) 2017-2018 Texas Instruments Incorporated - http://www.ti.com/
 * Jean-Jacques Hiblot <jjhiblot@ti.com>
 */

#include <common.h>
#include <mux-internal.h>
#include <dm.h>
#include <dt-structs.h>
#include <errno.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/read.h>
#include <dt-bindings/mux/mux.h>

/*
 * The idle-as-is "state" is not an actual state that may be selected, it
 * only implies that the state should not be changed. So, use that state
 * as indication that the cached state of the multiplexer is unknown.
 */
#define MUX_CACHE_UNKNOWN MUX_IDLE_AS_IS

static inline const struct mux_control_ops *mux_dev_ops(struct udevice *dev)
{
	return (const struct mux_control_ops *)dev->driver->ops;
}

static int mux_control_set(struct mux_control *mux, int state)
{
	int ret = mux_dev_ops(mux->dev)->set(mux, state);

	mux->cached_state = ret < 0 ? MUX_CACHE_UNKNOWN : state;

	return ret;
}

unsigned int mux_control_states(struct mux_control *mux)
{
	return mux->states;
}
EXPORT_SYMBOL_GPL(mux_control_states);

static int __mux_control_select(struct mux_control *mux, int state)
{
	int ret;

	if (WARN_ON(state < 0 || state >= mux->states))
		return -EINVAL;

	if (mux->cached_state == state)
		return 0;

	ret = mux_control_set(mux, state);
	if (ret >= 0)
		return 0;

	/* The mux update failed, try to revert if appropriate... */
	if (mux->idle_state != MUX_IDLE_AS_IS)
		mux_control_set(mux, mux->idle_state);

	return ret;
}

int mux_control_select(struct mux_control *mux, unsigned int state)
{
	int ret;

	if (mux->in_use)
		return -EBUSY;

	ret = __mux_control_select(mux, state);

	if (ret < 0)
		return ret;

	mux->in_use = true;

	return 0;
}

int mux_control_deselect(struct mux_control *mux)
{
	int ret = 0;

	if (mux->idle_state != MUX_IDLE_AS_IS &&
	    mux->idle_state != mux->cached_state)
		ret = mux_control_set(mux, mux->idle_state);

	mux->in_use = false;

	return ret;
}

static int mux_of_xlate_default(struct mux_control *mux,
				struct ofnode_phandle_args *args)
{
	debug("%s(mux=%p)\n", __func__, mux);

	if (args->args_count > 1) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	if (args->args_count)
		mux->id = args->args[0];
	else
		mux->id = 0;

	return 0;
}

static int mux_get_by_indexed_prop(struct udevice *dev, const char *prop_name,
				   int index, struct mux_control *mux)
{
	int ret;
	struct ofnode_phandle_args args;
	struct udevice *dev_mux;
	const struct mux_control_ops *ops;
	struct mux_chip *mux_chip;

	debug("%s(dev=%p, index=%d, mux=%p)\n", __func__, dev, index, mux);

	assert(mux);
	mux->dev = NULL;

	ret = dev_read_phandle_with_args(dev, prop_name, "#mux-control-cells",
					 0, index, &args);
	if (ret) {
		debug("%s: fdtdec_parse_phandle_with_args failed: err=%d\n",
		      __func__, ret);
		return ret;
	}

	ret = uclass_get_device_by_ofnode(UCLASS_MUX, args.node, &dev_mux);
	if (ret) {
		debug("%s: uclass_get_device_by_of_offset failed: err=%d\n",
		      __func__, ret);
		return ret;
	}

	mux_chip = dev_get_uclass_priv(dev_mux);
	mux->dev = dev_mux;

	ops = mux_dev_ops(dev_mux);

	if (ops->of_xlate)
		ret = ops->of_xlate(mux, &args);
	else
		ret = mux_of_xlate_default(mux, &args);
	if (ret) {
		debug("of_xlate() failed: %d\n", ret);
		return ret;
	}

	if (mux->id >= mux_chip->controllers) {
		dev_err(dev, "bad mux controller %u specified in %s\n",
			mux->id, ofnode_get_name(args.node));
		return -EINVAL;
	}
	return 0;
}

int mux_get_by_index(struct udevice *dev, int index, struct mux_control *mux)
{
	return mux_get_by_indexed_prop(dev, "mux-controls", index, mux);
}

int mux_control_get(struct udevice *dev, const char *name,
		    struct mux_control *mux)
{
	int index;

	debug("%s(dev=%p, name=%s, mux=%p)\n", __func__, dev, name, mux);
	mux->dev = NULL;

	index = dev_read_stringlist_search(dev, "mux-control-names", name);
	if (index < 0) {
		debug("fdt_stringlist_search() failed: %d\n", index);
		return index;
	}

	return mux_get_by_index(dev, index, mux);
}

void mux_control_put(struct mux_control *mux)
{
}

static void devm_mux_control_release(struct udevice *dev, void *res)
{
	mux_control_put(res);
}

struct mux_control *devm_mux_control_get(struct udevice *dev, const char *id)
{
	int rc;
	struct mux_control *mux;

	mux = devres_alloc(devm_mux_control_release, sizeof(struct mux_control),
			   __GFP_ZERO);
	if (unlikely(!mux))
		return ERR_PTR(-ENOMEM);

	rc = mux_control_get(dev, id, mux);
	if (rc)
		return ERR_PTR(rc);

	devres_add(dev, mux);
	return mux;
}

int mux_alloc_controllers(struct udevice *dev, unsigned int controllers)
{
	int i;
	struct mux_chip *mux_chip = dev_get_uclass_priv(dev);

	mux_chip->mux = devm_kmalloc(dev,
				     sizeof(struct mux_control) * controllers,
				     __GFP_ZERO);
	if (!mux_chip->mux)
		return -ENOMEM;

	mux_chip->controllers = controllers;

	for (i = 0; i < mux_chip->controllers; ++i) {
		struct mux_control *mux = &mux_chip->mux[i];

		mux->dev = dev;
		mux->cached_state = MUX_CACHE_UNKNOWN;
		mux->idle_state = MUX_IDLE_AS_IS;
		mux->in_use = false;
		mux->id = i;
	}

	return 0;
}

int mux_uclass_post_probe(struct udevice *dev)
{
	int i, ret;
	struct mux_chip *mux_chip = dev_get_uclass_priv(dev);

	for (i = 0; i < mux_chip->controllers; ++i) {
		struct mux_control *mux = &mux_chip->mux[i];

		if (mux->idle_state == mux->cached_state)
			continue;

		ret = mux_control_set(mux, mux->idle_state);
		if (ret < 0) {
			dev_err(&mux_chip->dev, "unable to set idle state\n");
			return ret;
		}
	}
	return 0;
}

void dm_mux_init(void)
{
	struct uclass *uc;
	struct udevice *dev;
	int ret;

	ret = uclass_get(UCLASS_MUX, &uc);
	if (ret < 0) {
		debug("unable to get MUX uclass\n");
		return;
	}
	uclass_foreach_dev(dev, uc) {
		if (dev_read_bool(dev, "u-boot,mux-autoprobe")) {
			ret = device_probe(dev);
			if (ret)
				debug("unable to probe device %s\n", dev->name);
		} else {
			printf("not found for dev %s\n", dev->name);
		}
	}
}

UCLASS_DRIVER(mux) = {
	.id		= UCLASS_MUX,
	.name		= "mux",
	.post_probe	= mux_uclass_post_probe,
	.per_device_auto_alloc_size = sizeof(struct mux_chip),
};
