// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2023 Toradex - https://www.toradex.com/
 */

#include <asm/hardware.h>
#include <fdt_support.h>

static void fdt_fixup_cores_nodes_am625(void *blob, u32 cores)
{
	int core_nr;

	for (core_nr = 1; core_nr < 4; core_nr++) {
		if (cores & BIT(core_nr))
			continue;

		fdt_del_node_by_pathf(blob, "/cpus/cpu@%x", core_nr);
		fdt_del_node_by_pathf(blob, "/cpus/cpu-map/cluster0/core%d",
				      core_nr);
		fdt_status_disabled_by_pathf(blob,
					     "/bus@f0000/watchdog@e0%x0000",
					     core_nr);
	}
}

static void fdt_fixup_gpu_nodes_am625(void *blob, bool has_gpu)
{
	if (!has_gpu) {
		fdt_status_disabled_by_pathf(blob, "/bus@f0000/gpu@fd00000");
		fdt_status_disabled_by_pathf(blob,
					     "/bus@f0000/watchdog@e0f0000");
	}
}

static void fdt_fixup_pru_node_am625(void *blob, bool has_pru)
{
	if (!has_pru)
		fdt_status_disabled_by_pathf(blob, "/bus@f0000/pruss@30040000");
}

static u32 k3_get_cores(void)
{
	u32 feature0 = readl(CTRLMMR_WKUP_DEVICE_FEATURE0);

	return (feature0 & DEVICE_FEATURE0_MPU_CORE_MASK) >>
		DEVICE_FEATURE0_MPU_CORE_SHIFT;
}

static bool k3_has_pru(void)
{
	u32 feature3 = readl(CTRLMMR_WKUP_DEVICE_FEATURE3);

	return feature3 & DEVICE_FEATURE3_ICSSM0;
}

static bool k3_has_gpu(void)
{
	u32 feature1 = readl(CTRLMMR_WKUP_DEVICE_FEATURE1);

	return feature1 & DEVICE_FEATURE1_GPU;
}

int ft_system_setup(void *blob, struct bd_info *bd)
{
	fdt_fixup_cores_nodes_am625(blob, k3_get_cores());
	fdt_fixup_gpu_nodes_am625(blob, k3_has_gpu());
	fdt_fixup_pru_node_am625(blob, k3_has_pru());

	return 0;
}
