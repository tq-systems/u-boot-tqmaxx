// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017-2018 Texas Instruments Incorporated - http://www.ti.com/
 * Jean-Jacques Hiblot <jjhiblot@ti.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <asm/gpio.h>
#include <dm/lists.h>
#include <dm/device-internal.h>
#include <regmap.h>
#include <reset-uclass.h>

#define WIZ_MAX_LANES		4
#define WIZ_MUX_NUM_CLOCKS	3
#define WIZ_DIV_NUM_CLOCKS	2

#define WIZ_SERDES_CTRL		0x404
#define WIZ_SERDES_TOP_CTRL	0x408
#define WIZ_SERDES_RST		0x40c
#define WIZ_SERDES_TYPEC	0x410
#define WIZ_LANECTL(n)		(0x480 + (0x40 * (n)))

#define WIZ_MAX_LANES		4
#define WIZ_MUX_NUM_CLOCKS	3
#define WIZ_DIV_NUM_CLOCKS	2

#define WIZ_SERDES_TYPEC_LN10_SWAP	BIT(30)

enum wiz_lane_standard_mode {
	LANE_MODE_GEN1,
	LANE_MODE_GEN2,
	LANE_MODE_GEN3,
	LANE_MODE_GEN4,
};

enum wiz_refclk_mux_sel {
	PLL0_REFCLK,
	PLL1_REFCLK,
	REFCLK_DIG,
};

enum wiz_refclk_div_sel {
	CMN_REFCLK,
	CMN_REFCLK1,
};

static const struct reg_field por_en = REG_FIELD(WIZ_SERDES_CTRL, 31, 31);
static const struct reg_field phy_reset_n = REG_FIELD(WIZ_SERDES_RST, 31, 31);
static const struct reg_field pll1_refclk_mux_sel =
					REG_FIELD(WIZ_SERDES_RST, 29, 29);
static const struct reg_field pll0_refclk_mux_sel =
					REG_FIELD(WIZ_SERDES_RST, 28, 28);
static const struct reg_field refclk_dig_sel =
					REG_FIELD(WIZ_SERDES_RST, 24, 25);
static const struct reg_field pma_cmn_refclk_int_mode =
					REG_FIELD(WIZ_SERDES_TOP_CTRL, 28, 29);
static const struct reg_field pma_cmn_refclk_mode =
					REG_FIELD(WIZ_SERDES_TOP_CTRL, 30, 31);
static const struct reg_field pma_cmn_refclk_dig_div =
					REG_FIELD(WIZ_SERDES_TOP_CTRL, 26, 27);
static const struct reg_field pma_cmn_refclk1_dig_div =
					REG_FIELD(WIZ_SERDES_TOP_CTRL, 24, 25);

static const struct reg_field p_enable[WIZ_MAX_LANES] = {
	REG_FIELD(WIZ_LANECTL(0), 30, 31),
	REG_FIELD(WIZ_LANECTL(1), 30, 31),
	REG_FIELD(WIZ_LANECTL(2), 30, 31),
	REG_FIELD(WIZ_LANECTL(3), 30, 31),
};

static const struct reg_field p_align[WIZ_MAX_LANES] = {
	REG_FIELD(WIZ_LANECTL(0), 29, 29),
	REG_FIELD(WIZ_LANECTL(1), 29, 29),
	REG_FIELD(WIZ_LANECTL(2), 29, 29),
	REG_FIELD(WIZ_LANECTL(3), 29, 29),
};

static const struct reg_field p_raw_auto_start[WIZ_MAX_LANES] = {
	REG_FIELD(WIZ_LANECTL(0), 28, 28),
	REG_FIELD(WIZ_LANECTL(1), 28, 28),
	REG_FIELD(WIZ_LANECTL(2), 28, 28),
	REG_FIELD(WIZ_LANECTL(3), 28, 28),
};

static const struct reg_field p_standard_mode[WIZ_MAX_LANES] = {
	REG_FIELD(WIZ_LANECTL(0), 24, 25),
	REG_FIELD(WIZ_LANECTL(1), 24, 25),
	REG_FIELD(WIZ_LANECTL(2), 24, 25),
	REG_FIELD(WIZ_LANECTL(3), 24, 25),
};

struct clk_div_table {
	unsigned int	val;
	unsigned int	div;
};

struct wiz_clk_mux_sel {
	struct regmap_field	*field;
	u32			table[4];
	const char		*node_name;
};

struct wiz_clk_div_sel {
	struct regmap_field	*field;
	const char		*node_name;
};

static struct wiz_clk_div_sel clk_div_sel[] = {
	{
		.node_name = "cmn_refclk",
	},
	{
		.node_name = "cmn_refclk1",
	},
};

static struct wiz_clk_mux_sel clk_mux_sel[] = {
	{
		/*
		 * Mux value to be configured for each of the input clocks
		 * in the order populated in device tree
		 */
		.table = { 1, 0 },
		.node_name = "pll0_refclk",
	},
	{
		.table = { 1, 0 },
		.node_name = "pll1_refclk",
	},
	{
		.table = { 1, 3, 0, 2 },
		.node_name = "refclk_dig",
	},
};

struct wiz {
	struct regmap		*regmap;
	struct regmap_field	*por_en;
	struct regmap_field	*phy_reset_n;
	struct regmap_field	*p_enable[WIZ_MAX_LANES];
	struct regmap_field	*p_align[WIZ_MAX_LANES];
	struct regmap_field	*p_raw_auto_start[WIZ_MAX_LANES];
	struct regmap_field	*p_standard_mode[WIZ_MAX_LANES];
	struct regmap_field	*pma_cmn_refclk_int_mode;
	struct regmap_field	*pma_cmn_refclk_mode;
	struct regmap_field	*pma_cmn_refclk_dig_div;
	struct regmap_field	*pma_cmn_refclk1_dig_div;
	struct regmap_field	*pll0_refclk_mux_sel;
	struct regmap_field	*pll1_refclk_mux_sel;
	struct regmap_field	*refclk_dig_sel;

	struct udevice		*dev;
	u32			num_lanes;
	struct gpio_desc	*gpio_typec_dir;
};

struct wiz_div_clk {
	struct clk parent_clk;
	struct wiz *wiz;
};

struct wiz_mux_clk {
	struct clk parent_clks[4];
	struct wiz *wiz;
};

struct wiz_reset {
	struct wiz *wiz;
};

static ulong wiz_div_clk_get_rate(struct clk *clk)
{
	struct udevice *dev = clk->dev;
	struct wiz_div_clk *priv = dev_get_priv(dev);
	struct wiz_clk_div_sel *data = dev_get_platdata(dev);
	ulong parent_rate = clk_get_rate(&priv->parent_clk);
	u32 val;

	regmap_field_read(data->field, &val);

	return parent_rate >> val;
}

static ulong wiz_div_clk_set_rate(struct clk *clk, ulong rate)
{
	struct udevice *dev = clk->dev;
	struct wiz_div_clk *priv = dev_get_priv(dev);
	struct wiz_clk_div_sel *data = dev_get_platdata(dev);
	ulong parent_rate = clk_get_rate(&priv->parent_clk);
	u32 div = parent_rate / rate;

	div = __ffs(div);
	regmap_field_write(data->field, div);

	return parent_rate >> div;
}

const struct clk_ops wiz_div_clk_ops = {
	.get_rate = wiz_div_clk_get_rate,
	.set_rate = wiz_div_clk_set_rate,
};

int wiz_div_clk_probe(struct udevice *dev)
{
	struct wiz_div_clk *priv = dev_get_priv(dev);
	struct clk parent_clk;
	int rc;

	rc = clk_get_by_index(dev, 0, &parent_clk);
	if (rc) {
		dev_err(dev, "unable to get parent clock. ret %d\n", rc);
		return rc;
	}
	priv->parent_clk = parent_clk;
	priv->wiz = dev_get_priv(dev->parent);
	return 0;
}

U_BOOT_DRIVER(wiz_div_clk) = {
	.name = "wiz_div_clk",
	.id = UCLASS_CLK,
	.priv_auto_alloc_size = sizeof(struct wiz_div_clk),
	.ops = &wiz_div_clk_ops,
	.probe = wiz_div_clk_probe,
};

int clk_mux_val_to_index(u32 table[4], unsigned int val)
{
	int i;

	for (i = 0; i < 4; i++)
		if (table[i] == val)
			return i;
	return -EINVAL;
}

static ulong wiz_clk_mux_get_rate(struct clk *clk)
{
	struct udevice *dev = clk->dev;
	struct wiz_mux_clk *priv = dev_get_priv(dev);
	struct wiz_clk_mux_sel *data = dev_get_platdata(dev);
	unsigned int val, idx;

	regmap_field_read(data->field, &val);
	idx = clk_mux_val_to_index(data->table, val);
	if (priv->parent_clks[idx].dev)
		return clk_get_rate(&priv->parent_clks[idx]);

	return 0;
}

static int wiz_clk_mux_set_parent(struct clk *clk,  struct clk *parent)
{
	struct udevice *dev = clk->dev;
	struct wiz_mux_clk *priv = dev_get_priv(dev);
	struct wiz_clk_mux_sel *data = dev_get_platdata(dev);
	int i;

	for (i = 0; i < ARRAY_SIZE(priv->parent_clks); i++)
		if (parent->dev == priv->parent_clks[i].dev)
			break;

	if (i == ARRAY_SIZE(priv->parent_clks))
		return -EINVAL;

	regmap_field_write(data->field, data->table[i]);
	return clk_get_rate(parent);
}

static const struct clk_ops wiz_clk_mux_ops = {
	.set_parent = wiz_clk_mux_set_parent,
	.get_rate = wiz_clk_mux_get_rate,
};

int wiz_mux_clk_probe(struct udevice *dev)
{
	struct wiz_mux_clk *priv = dev_get_priv(dev);
	int rc;
	int i;

	for (i = 0; i < ARRAY_SIZE(priv->parent_clks); i++) {
		rc = clk_get_by_index(dev, i, &priv->parent_clks[i]);
		if (rc)
			priv->parent_clks[i].dev = NULL;
	}
	priv->wiz = dev_get_priv(dev->parent);
	return 0;
}

U_BOOT_DRIVER(wiz_mux_clk) = {
	.name = "wiz_mux_clk",
	.id = UCLASS_CLK,
	.priv_auto_alloc_size = sizeof(struct wiz_mux_clk),
	.ops = &wiz_clk_mux_ops,
	.probe = wiz_mux_clk_probe,
};

static int wiz_reset_request(struct reset_ctl *reset_ctl)
{
	return 0;
}

static int wiz_reset_free(struct reset_ctl *reset_ctl)
{
	return 0;
}

static int wiz_reset_assert(struct reset_ctl *reset_ctl)
{
	struct wiz_reset *priv = dev_get_priv(reset_ctl->dev);
	struct wiz *wiz = priv->wiz;
	int ret;
	int id = reset_ctl->id;

	if (id == 0) {
		ret = regmap_field_write(wiz->phy_reset_n, false);
		return ret;
	}

	ret = regmap_field_write(wiz->p_enable[id - 1], false);
	return ret;
}

static int wiz_reset_deassert(struct reset_ctl *reset_ctl)
{
	struct wiz_reset *priv = dev_get_priv(reset_ctl->dev);
	struct wiz *wiz = priv->wiz;
	int ret;
	int id = reset_ctl->id;

	/* if typec-dir gpio was specified, set LN10 SWAP bit based on that */
	if (id == 0 && wiz->gpio_typec_dir) {
		if (dm_gpio_get_value(wiz->gpio_typec_dir)) {
			regmap_update_bits(wiz->regmap, WIZ_SERDES_TYPEC,
					   WIZ_SERDES_TYPEC_LN10_SWAP,
					   WIZ_SERDES_TYPEC_LN10_SWAP);
		} else {
			regmap_update_bits(wiz->regmap, WIZ_SERDES_TYPEC,
					   WIZ_SERDES_TYPEC_LN10_SWAP, 0);
		}
	}

	if (id == 0) {
		ret = regmap_field_write(wiz->phy_reset_n, true);
		return ret;
	}

	ret = regmap_field_write(wiz->p_enable[id - 1], true);
	return ret;
}

static struct reset_ops wiz_reset_ops = {
	.request = wiz_reset_request,
	.free = wiz_reset_free,
	.rst_assert = wiz_reset_assert,
	.rst_deassert = wiz_reset_deassert,
};

int wiz_reset_probe(struct udevice *dev)
{
	struct wiz_reset *priv = dev_get_priv(dev);

	priv->wiz = dev_get_priv(dev->parent);

	return 0;
}

U_BOOT_DRIVER(wiz_reset) = {
	.name = "wiz-reset",
	.id = UCLASS_RESET,
	.probe = wiz_reset_probe,
	.ops = &wiz_reset_ops,
};

static int wiz_reset(struct wiz *wiz)
{
	int ret;

	ret = regmap_field_write(wiz->por_en, 0x1);
	if (ret)
		return ret;

	mdelay(1);

	ret = regmap_field_write(wiz->por_en, 0x0);
	if (ret)
		return ret;

	return 0;
}

static int wiz_mode_select(struct wiz *wiz)
{
	u32 num_lanes = wiz->num_lanes;
	int ret;
	int i;

	for (i = 0; i < num_lanes; i++) {
		ret = regmap_field_write(wiz->p_standard_mode[i],
					 LANE_MODE_GEN4);
		if (ret)
			return ret;
	}

	return 0;
}

static int wiz_init_raw_interface(struct wiz *wiz, bool enable)
{
	u32 num_lanes = wiz->num_lanes;
	int i;
	int ret;

	for (i = 0; i < num_lanes; i++) {
		ret = regmap_field_write(wiz->p_align[i], enable);
		if (ret)
			return ret;

		ret = regmap_field_write(wiz->p_raw_auto_start[i], enable);
		if (ret)
			return ret;
	}

	return 0;
}

static int wiz_init(struct wiz *wiz)
{
	int ret;

	ret = wiz_reset(wiz);
	if (ret) {
		dev_err(dev, "WIZ reset failed\n");
		return ret;
	}

	ret = wiz_mode_select(wiz);
	if (ret) {
		dev_err(dev, "WIZ mode select failed\n");
		return ret;
	}

	ret = wiz_init_raw_interface(wiz, true);
	if (ret) {
		dev_err(dev, "WIZ interface initialization failed\n");
		return ret;
	}

	return 0;
}

static int wiz_regfield_init(struct wiz *wiz)
{
	struct regmap *regmap = wiz->regmap;
	int num_lanes = wiz->num_lanes;
	struct udevice *dev = wiz->dev;
	int i;

	wiz->por_en = devm_regmap_field_alloc(dev, regmap, por_en);
	if (IS_ERR(wiz->por_en)) {
		dev_err(dev, "POR_EN reg field init failed\n");
		return PTR_ERR(wiz->por_en);
	}

	wiz->phy_reset_n = devm_regmap_field_alloc(dev, regmap,
						   phy_reset_n);
	if (IS_ERR(wiz->phy_reset_n)) {
		dev_err(dev, "PHY_RESET_N reg field init failed\n");
		return PTR_ERR(wiz->phy_reset_n);
	}

	wiz->pma_cmn_refclk_int_mode =
		devm_regmap_field_alloc(dev, regmap, pma_cmn_refclk_int_mode);
	if (IS_ERR(wiz->pma_cmn_refclk_int_mode)) {
		dev_err(dev, "PMA_CMN_REFCLK_INT_MODE reg field init failed\n");
		return PTR_ERR(wiz->pma_cmn_refclk_int_mode);
	}

	wiz->pma_cmn_refclk_mode =
		devm_regmap_field_alloc(dev, regmap, pma_cmn_refclk_mode);
	if (IS_ERR(wiz->pma_cmn_refclk_mode)) {
		dev_err(dev, "PMA_CMN_REFCLK_MODE reg field init failed\n");
		return PTR_ERR(wiz->pma_cmn_refclk_mode);
	}

	wiz->pma_cmn_refclk_dig_div = devm_regmap_field_alloc(dev, regmap,
						     pma_cmn_refclk_dig_div);
	if (IS_ERR(wiz->pma_cmn_refclk_dig_div)) {
		dev_err(dev, "PMA_CMN_REFCLK_DIG_DIV reg field init failed\n");
		return PTR_ERR(wiz->pma_cmn_refclk_dig_div);
	}
	clk_div_sel[CMN_REFCLK].field = wiz->pma_cmn_refclk_dig_div;

	wiz->pma_cmn_refclk1_dig_div = devm_regmap_field_alloc(dev, regmap,
						     pma_cmn_refclk1_dig_div);
	if (IS_ERR(wiz->pma_cmn_refclk1_dig_div)) {
		dev_err(dev, "PMA_CMN_REFCLK1_DIG_DIV reg field init failed\n");
		return PTR_ERR(wiz->pma_cmn_refclk1_dig_div);
	}
	clk_div_sel[CMN_REFCLK1].field = wiz->pma_cmn_refclk1_dig_div;

	wiz->pll0_refclk_mux_sel = devm_regmap_field_alloc(dev, regmap,
						     pll0_refclk_mux_sel);
	if (IS_ERR(wiz->pll0_refclk_mux_sel)) {
		dev_err(dev, "PLL0_REFCLK_SEL reg field init failed\n");
		return PTR_ERR(wiz->pll0_refclk_mux_sel);
	}
	clk_mux_sel[PLL0_REFCLK].field = wiz->pll0_refclk_mux_sel;

	wiz->pll1_refclk_mux_sel = devm_regmap_field_alloc(dev, regmap,
						     pll1_refclk_mux_sel);
	if (IS_ERR(wiz->pll1_refclk_mux_sel)) {
		dev_err(dev, "PLL1_REFCLK_SEL reg field init failed\n");
		return PTR_ERR(wiz->pll1_refclk_mux_sel);
	}
	clk_mux_sel[PLL1_REFCLK].field = wiz->pll1_refclk_mux_sel;

	wiz->refclk_dig_sel = devm_regmap_field_alloc(dev, regmap,
						     refclk_dig_sel);
	if (IS_ERR(wiz->refclk_dig_sel)) {
		dev_err(dev, "REFCLK_DIG_SEL reg field init failed\n");
		return PTR_ERR(wiz->refclk_dig_sel);
	}
	clk_mux_sel[REFCLK_DIG].field = wiz->refclk_dig_sel;

	for (i = 0; i < num_lanes; i++) {
		wiz->p_enable[i] = devm_regmap_field_alloc(dev, regmap,
							   p_enable[i]);
		if (IS_ERR(wiz->p_enable[i])) {
			dev_err(dev, "P%d_ENABLE reg field init failed\n", i);
			return PTR_ERR(wiz->p_enable[i]);
		}

		wiz->p_align[i] = devm_regmap_field_alloc(dev, regmap,
							  p_align[i]);
		if (IS_ERR(wiz->p_align[i])) {
			dev_err(dev, "P%d_ALIGN reg field init failed\n", i);
			return PTR_ERR(wiz->p_align[i]);
		}

		wiz->p_raw_auto_start[i] =
		  devm_regmap_field_alloc(dev, regmap, p_raw_auto_start[i]);
		if (IS_ERR(wiz->p_raw_auto_start[i])) {
			dev_err(dev, "P%d_RAW_AUTO_START reg field init fail\n",
				i);
			return PTR_ERR(wiz->p_raw_auto_start[i]);
		}

		wiz->p_standard_mode[i] =
		  devm_regmap_field_alloc(dev, regmap, p_standard_mode[i]);
		if (IS_ERR(wiz->p_standard_mode[i])) {
			dev_err(dev, "P%d_STANDARD_MODE reg field init fail\n",
				i);
			return PTR_ERR(wiz->p_standard_mode[i]);
		}
	}

	return 0;
}

static int wiz_clock_init(struct wiz *wiz)
{
	struct udevice *dev = wiz->dev;
	unsigned long rate;
	struct clk *clk;
	int ret;

	clk = devm_clk_get(dev, "core_ref_clk");
	if (IS_ERR(clk)) {
		dev_err(dev, "core_ref_clk clock not found\n");
		ret = PTR_ERR(clk);
		return ret;
	}

	rate = clk_get_rate(clk);
	if (rate >= 100000000)
		regmap_field_write(wiz->pma_cmn_refclk_int_mode, 0x1);
	else
		regmap_field_write(wiz->pma_cmn_refclk_int_mode, 0x3);

	clk = devm_clk_get(dev, "ext_ref_clk");
	if (IS_ERR(clk)) {
		dev_err(dev, "ext_ref_clk clock not found\n");
		ret = PTR_ERR(clk);
		return ret;
	}

	rate = clk_get_rate(clk);
	if (rate >= 100000000)
		regmap_field_write(wiz->pma_cmn_refclk_mode, 0x0);
	else
		regmap_field_write(wiz->pma_cmn_refclk_mode, 0x2);

	return 0;
}

static ofnode get_child_by_name(struct udevice *dev, const char *name)
{
	int l = strlen(name);
	ofnode node = dev_read_first_subnode(dev);

	while (ofnode_valid(node)) {
		const char *child_name = ofnode_get_name(node);

		if (!strncmp(child_name, name, l)) {
			if (child_name[l] == '\0' || child_name[l] == '@')
				return node;
		}
		node = dev_read_next_subnode(node);
	}
	return node;
}

static int j721e_wiz_bind_clocks(struct udevice *dev)
{
	int i;
	int rc;
	ofnode node;
	struct driver *div_clk_drv;
	struct driver *mux_clk_drv;

	div_clk_drv = lists_driver_lookup_name("wiz_div_clk");
	if (!div_clk_drv) {
		dev_err(dev, "Cannot find driver 'wiz_div_clk'\n");
		return -ENOENT;
	}

	mux_clk_drv = lists_driver_lookup_name("wiz_mux_clk");
	if (!mux_clk_drv) {
		dev_err(dev, "Cannot find driver 'wiz_mux_clk'\n");
		return -ENOENT;
	}

	for (i = 0; i < WIZ_DIV_NUM_CLOCKS; i++) {
		node = get_child_by_name(dev, clk_div_sel[i].node_name);
		if (!ofnode_valid(node)) {
			dev_err(dev, "cannot find node for clock %s\n",
				clk_div_sel[i].node_name);
			continue;
		}
		rc = device_bind_ofnode(dev, div_clk_drv,
					clk_div_sel[i].node_name,
					&clk_div_sel[i], node, NULL);
		if (rc) {
			dev_err(dev, "cannot bind driver for clock %s\n",
				clk_div_sel[i].node_name);
		}
	}

	for (i = 0; i < WIZ_MUX_NUM_CLOCKS; i++) {
		node = get_child_by_name(dev, clk_mux_sel[i].node_name);
		if (!ofnode_valid(node)) {
			dev_err(dev, "cannot find node for clock %s\n",
				clk_mux_sel[i].node_name);
			continue;
		}
		rc = device_bind_ofnode(dev, mux_clk_drv,
					clk_mux_sel[i].node_name,
					&clk_mux_sel[i], node, NULL);
		if (rc) {
			dev_err(dev, "cannot bind driver for clock %s\n",
				clk_mux_sel[i].node_name);
		}
	}

	return 0;
}

static int j721e_wiz_bind_reset(struct udevice *dev)
{
	int rc;
	struct driver *drv;

	drv = lists_driver_lookup_name("wiz-reset");
	if (!drv) {
		dev_err(dev, "Cannot find driver 'wiz-reset'\n");
		return -ENOENT;
	}
	rc = device_bind_ofnode(dev, drv, "wiz-reset", NULL, dev_ofnode(dev),
				NULL);
	if (rc) {
		dev_err(dev, "cannot bind driver for wiz-reset\n");
		return rc;
	}

	return 0;
}

static int j721e_wiz_bind(struct udevice *dev)
{
	int rc;

	rc = j721e_wiz_bind_clocks(dev);
	if (rc)
		return rc;

	rc = j721e_wiz_bind_reset(dev);
	if (rc)
		return rc;

	dm_scan_fdt_dev(dev);

	return 0;
}

static int j721e_wiz_probe(struct udevice *dev)
{
	struct wiz *wiz = dev_get_priv(dev);
	int rc;
	ofnode node;
	struct regmap *regmap;
	u32 num_lanes;

	node = get_child_by_name(dev, "serdes");

	if (!ofnode_valid(node)) {
		dev_err(dev, "Failed to get SERDES child DT node\n");
		return -ENODEV;
	}

	rc = regmap_init_mem(node, &regmap);
	if (rc)  {
		dev_err(dev, "Failed to get memory resource\n");
		return rc;
	}
	rc = dev_read_u32(dev, "num-lanes", &num_lanes);
	if (rc) {
		dev_err(dev, "Failed to read num-lanes property\n");
		goto err_addr_to_resource;
	}

	if (num_lanes > WIZ_MAX_LANES) {
		dev_err(dev, "Cannot support %d lanes\n", num_lanes);
		goto err_addr_to_resource;
	}

	wiz->gpio_typec_dir = devm_gpiod_get_optional(dev, "typec-dir",
						      GPIOD_IS_IN);
	if (IS_ERR(wiz->gpio_typec_dir)) {
		rc = PTR_ERR(wiz->gpio_typec_dir);
		dev_err(dev, "Failed to request typec-dir gpio: %d\n", rc);
		goto err_addr_to_resource;
	}

	wiz->regmap = regmap;
	wiz->num_lanes = num_lanes;
	wiz->dev = dev;

	rc = wiz_regfield_init(wiz);
	if (rc) {
		dev_err(dev, "Failed to initialize regfields\n");
		goto err_addr_to_resource;
	}

	rc = wiz_clock_init(wiz);
	if (rc) {
		dev_warn(dev, "Failed to initialize clocks\n");
		goto err_addr_to_resource;
	}

	rc = wiz_init(wiz);
	if (rc) {
		dev_err(dev, "WIZ initialization failed\n");
		goto err_addr_to_resource;
	}

	return 0;

err_addr_to_resource:
	free(regmap);

	return rc;
}

static int j721e_wiz_remove(struct udevice *dev)
{
	struct wiz *wiz = dev_get_priv(dev);

	if (wiz->regmap)
		free(wiz->regmap);

	return 0;
}

static const struct udevice_id j721e_wiz_ids[] = {
	{ .compatible = "ti,j721e-wiz"},
	{ }
};

U_BOOT_DRIVER(phy_j721e_wiz) = {
	.name	= "phy-j721e-wiz",
	.id	= UCLASS_NOP,
	.of_match = j721e_wiz_ids,
	.bind = j721e_wiz_bind,
	.probe = j721e_wiz_probe,
	.remove = j721e_wiz_remove,
	.priv_auto_alloc_size = sizeof(struct wiz),
};
