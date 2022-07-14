// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 * Copyright (C) 2022 Marek Vasut <marex@denx.de>
 * Copyright (c) 2022 TQ-Systems GmbH <u-boot@ew.tq-group.com>, D-82229 Seefeld, Germany.
 * Author: Alexander Stein
 *
 */

#include <asm/arch/clock.h>
#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <linux/err.h>
#include <linux/iopoll.h>
#include <panel.h>
#include <regmap.h>
#include <syscon.h>
#include <video.h>
#include <video_bridge.h>
#include <video_link.h>

#define	PS2KHZ(ps)	(1000000000UL / (ps))
#define HZ2PS(hz)	(1000000000UL / ((hz) / 1000))

#define LDB_CTRL				0x5c
#define LDB_CTRL_CH0_ENABLE			BIT(0)
#define LDB_CTRL_CH0_DI_SELECT			BIT(1)
#define LDB_CTRL_CH1_ENABLE			BIT(2)
#define LDB_CTRL_CH1_DI_SELECT			BIT(3)
#define LDB_CTRL_SPLIT_MODE			BIT(4)
#define LDB_CTRL_CH0_DATA_WIDTH			BIT(5)
#define LDB_CTRL_CH0_BIT_MAPPING		BIT(6)
#define LDB_CTRL_CH1_DATA_WIDTH			BIT(7)
#define LDB_CTRL_CH1_BIT_MAPPING		BIT(8)
#define LDB_CTRL_DI0_VSYNC_POLARITY		BIT(9)
#define LDB_CTRL_DI1_VSYNC_POLARITY		BIT(10)
#define LDB_CTRL_REG_CH0_FIFO_RESET		BIT(11)
#define LDB_CTRL_REG_CH1_FIFO_RESET		BIT(12)
#define LDB_CTRL_ASYNC_FIFO_ENABLE		BIT(24)
#define LDB_CTRL_ASYNC_FIFO_THRESHOLD_MASK	GENMASK(27, 25)

#define LVDS_CTRL				0x128
#define LVDS_CTRL_CH0_EN			BIT(0)
#define LVDS_CTRL_CH1_EN			BIT(1)
#define LVDS_CTRL_VBG_EN			BIT(2)
#define LVDS_CTRL_HS_EN				BIT(3)
#define LVDS_CTRL_PRE_EMPH_EN			BIT(4)
#define LVDS_CTRL_PRE_EMPH_ADJ(n)		(((n) & 0x7) << 5)
#define LVDS_CTRL_PRE_EMPH_ADJ_MASK		GENMASK(7, 5)
#define LVDS_CTRL_CM_ADJ(n)			(((n) & 0x7) << 8)
#define LVDS_CTRL_CM_ADJ_MASK			GENMASK(10, 8)
#define LVDS_CTRL_CC_ADJ(n)			(((n) & 0x7) << 11)
#define LVDS_CTRL_CC_ADJ_MASK			GENMASK(13, 11)
#define LVDS_CTRL_SLEW_ADJ(n)			(((n) & 0x7) << 14)
#define LVDS_CTRL_SLEW_ADJ_MASK			GENMASK(16, 14)
#define LVDS_CTRL_VBG_ADJ(n)			(((n) & 0x7) << 17)
#define LVDS_CTRL_VBG_ADJ_MASK			GENMASK(19, 17)

struct imx8mp_ldb_priv {
	struct regmap* regmap;
	struct udevice *panel;
};

void imx8mp_ldb_configure(struct udevice *dev, struct display_timing *timings)
{
	struct imx8mp_ldb_priv *priv = dev_get_priv(dev);
	u32 reg;

	/* Setup clocks */
	mxs_set_ldbclk(0, timings->pixelclock.typ * 7 / 1000);

	/* Program LDB_CTRL */
	reg = LDB_CTRL_CH0_ENABLE;
	/* enable 24bpp mode */
	reg |= LDB_CTRL_CH0_DATA_WIDTH;

	if (timings->flags & DISPLAY_FLAGS_VSYNC_HIGH) {
		reg |= LDB_CTRL_DI0_VSYNC_POLARITY;
	}

	regmap_write(priv->regmap, LDB_CTRL, reg);

	/* Program LVDS_CTRL */
	reg = LVDS_CTRL_CC_ADJ(2) | LVDS_CTRL_PRE_EMPH_EN |
	      LVDS_CTRL_PRE_EMPH_ADJ(3) | LVDS_CTRL_VBG_EN;
	regmap_write(priv->regmap, LVDS_CTRL, reg);

	/* Wait for VBG to stabilize. */
	udelay(15);

	reg |= LVDS_CTRL_CH0_EN;

	regmap_write(priv->regmap, LVDS_CTRL, reg);
}

static int imx8mp_ldb_attach(struct udevice *dev)
{
	struct imx8mp_ldb_priv *priv = dev_get_priv(dev);
	struct display_timing timings;
	int ret;

	priv->panel = video_link_get_next_device(dev);
	if (!priv->panel ||
		device_get_uclass_id(priv->panel) != UCLASS_PANEL) {
		dev_err(dev, "get panel device error\n");
		return -ENODEV;
	}

	ret = video_link_get_display_timings(&timings);
	if (ret) {
		dev_err(dev, "decode display timing error %d\n", ret);
		return ret;
	}

	imx8mp_ldb_configure(dev, &timings);

	return 0;
}

static int imx8mp_ldb_set_backlight(struct udevice *dev, int percent)
{
	struct imx8mp_ldb_priv *priv = dev_get_priv(dev);
	int ret;

	ret = panel_enable_backlight(priv->panel);
	if (ret) {
		dev_err(dev, "panel %s enable backlight error %d\n",
			priv->panel->name, ret);
		return ret;
	}

	return 0;
}

static int imx8mp_ldb_probe(struct udevice *dev)
{
	struct imx8mp_ldb_priv *priv = dev_get_priv(dev);

	priv->regmap = syscon_node_to_regmap(dev_get_parent(dev)->node);
	if (IS_ERR(priv->regmap)) {
		printf("fail to get regmap\n");
		return PTR_ERR(priv->regmap);
	}

	return 0;
}

static int imx8mp_ldb_remove(struct udevice *dev)
{
	struct imx8mp_ldb_priv *priv = dev_get_priv(dev);

	if (priv->panel)
		device_remove(priv->panel, DM_REMOVE_NORMAL);

	return 0;
}

struct video_bridge_ops imx8mp_ldb_ops = {
	.attach = imx8mp_ldb_attach,
	.set_backlight = imx8mp_ldb_set_backlight,
};

static const struct udevice_id imx8mp_ldb_ids[] = {
	{ .compatible = "fsl,imx8mp-ldb" },
	{ }
};

U_BOOT_DRIVER(imx8mp_ldb) = {
	.name				= "imx8mp_ldb",
	.id				= UCLASS_VIDEO_BRIDGE,
	.of_match			= imx8mp_ldb_ids,
	.bind				= dm_scan_fdt_dev,
	.remove 			= imx8mp_ldb_remove,
	.probe				= imx8mp_ldb_probe,
	.ops				= &imx8mp_ldb_ops,
	.priv_auto_alloc_size		= sizeof(struct imx8mp_ldb_priv),
};
