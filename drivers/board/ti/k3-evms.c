// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Texas Instruments Incorporated - http://www.ti.com/
 *     Jean-Jacques Hiblot <jjhiblot@ti.com>
 */

#include <common.h>
#include <dm.h>
#include <board.h>
#include <linux/ctype.h>

extern char *k3_dtbo_list[];

struct k3_board_priv {
	int nb_daughter_boards;
	char **names;
};

static const struct udevice_id k3_board_ids[] = {
	{ .compatible = "ti,am654-evm",},
	{ .compatible = "ti,j721e-evm",},
	{ /* sentinel */ }
};

static int k3_board_detect(struct udevice *dev)
{
	struct k3_board_priv *priv = dev_get_priv(dev);
	int i, count = 0;

	while (k3_dtbo_list[count])
		count++;

	priv->nb_daughter_boards = count;
	priv->names = devm_kmalloc(dev, sizeof(*priv->names) * count, 0);
	for (i = 0; i < priv->nb_daughter_boards; i++) {
		int len = strlen(k3_dtbo_list[i]);

		len -= 5; // remove the ".dtbo" suffix
		priv->names[i] = devm_kmalloc(dev, len, 0);
		memcpy(priv->names[i], k3_dtbo_list[i], len);
		priv->names[i][len] = '\0';
	}

	return 0;
}

static int k3_board_get_fit_loadable(struct udevice *dev, int index,
				     const char *type, const char **strp)
{
	struct k3_board_priv *priv = dev_get_priv(dev);

	if (strcmp(type, FIT_FDT_PROP))
		return -ENOENT;

	if (index >= priv->nb_daughter_boards)
		return -ENOENT;

	*strp = priv->names[index];

	return 0;
}

static const struct board_ops k3_board_ops = {
	.detect = k3_board_detect,
	.get_fit_loadable = k3_board_get_fit_loadable,
	.get_int = NULL,
	.get_str = NULL,
};

U_BOOT_DRIVER(board_k3_evms) = {
	.name           = "board_k3_evms",
	.id             = UCLASS_BOARD,
	.of_match       = k3_board_ids,
	.ops		= &k3_board_ops,
	.priv_auto_alloc_size = sizeof(struct k3_board_priv),
};
