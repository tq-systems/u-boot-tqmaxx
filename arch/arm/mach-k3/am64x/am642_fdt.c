// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2023 Toradex - https://www.toradex.com/
 * Copyright (c) 2023-2025 TQ-Systems GmbH <u-boot@ew.tq-group.com>, D-82229 Seefeld, Germany.
 */

#include <asm/hardware.h>
#include <fdt_support.h>

static void fdt_fixup_cores_nodes_am642(void *blob, int core_nr)
{
	if (core_nr < 1)
		return;

	for (; core_nr < 4; core_nr++) {
		fdt_del_node_by_pathf(blob, "/cpus/cpu@%x", core_nr);
		fdt_del_node_by_pathf(blob, "/cpus/cpu-map/cluster0/core%d",
				      core_nr);
		fdt_status_disabled_by_pathf(blob,
					     "/bus@f0000/watchdog@e0%x0000",
					     core_nr);
	}
}

static void fdt_fixup_cores_r5_nodes_am642(void *blob, int cluster, u32 cores)
{
	int off;

	if (!(cores & BIT(0))) {
		fdt_status_disabled_by_pathf(blob,
					     "/bus@f4000/r5fss@78%x00000",
					     4 * cluster);
	} else if (!(cores & BIT(1))) {
		off = fdt_node_offset_by_pathf(blob,
					       "/bus@f4000/r5fss@78%x00000/r5f@78%x00000",
					       4 * cluster, 4 * cluster + 2);
		fdt_delprop(blob, off, "mboxes");
		fdt_delprop(blob, off, "memory-region");

		off = fdt_node_offset_by_pathf(blob,
					       "/bus@f4000/r5fss@78%x00000",
					       4 * cluster);
		/* Set to single-CPU mode */
		fdt_setprop_u32(blob, off, "ti,cluster-mode", 2);
	}
}

static void fdt_fixup_adc_node_am642(void *blob, bool has_adc)
{
	if (!has_adc)
		fdt_status_disabled_by_pathf(blob,
					     "/bus@f4000/tscadc@28001000");
}

static void fdt_fixup_icss_node_am642(void *blob, int has_icss)
{
	if (!has_icss) {
		fdt_status_disabled_by_pathf(blob, "/bus@f4000/icssg@30000000");
		fdt_status_disabled_by_pathf(blob, "/bus@f4000/icssg@30080000");
	}
}

int ft_system_setup(void *blob, struct bd_info *bd)
{
	fdt_fixup_cores_nodes_am642(blob, k3_get_core_nr());
	fdt_fixup_cores_r5_nodes_am642(blob, 0, k3_get_cores_r5fss0());
	fdt_fixup_cores_r5_nodes_am642(blob, 1, k3_get_cores_r5fss1());
	fdt_fixup_adc_node_am642(blob, k3_has_adc());
	fdt_fixup_icss_node_am642(blob, k3_has_icss());

	return 0;
}
