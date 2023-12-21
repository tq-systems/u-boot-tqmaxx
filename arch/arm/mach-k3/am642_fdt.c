// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2023 Toradex - https://www.toradex.com/
 * Copyright (c) 2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>, D-82229 Seefeld, Germany.
 */

#include <asm/hardware.h>
#include <fdt_support.h>

static void fdt_fixup_cores_nodes_am642(void *blob, u32 cores)
{
	int core_nr;

	for (core_nr = 1; core_nr < 2; core_nr++) {
		if (cores & BIT(core_nr))
			continue;

		fdt_del_node_by_pathf(blob, "/cpus/cpu@%x", core_nr);
		fdt_del_node_by_pathf(blob, "/cpus/cpu-map/cluster0/core%d",
				      core_nr);
		fdt_status_disabled_by_pathf(blob,
					     "/bus@f4000/watchdog@e0%x0000",
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

static void fdt_fixup_icss_node_am642(void *blob, int has_pru)
{
	if (!has_pru) {
		fdt_status_disabled_by_pathf(blob, "/bus@f4000/icssg@30000000");
		fdt_status_disabled_by_pathf(blob, "/bus@f4000/icssg@30080000");
	}
}

static u32 k3_get_cores(void)
{
	u32 feature0 = readl(CTRLMMR_WKUP_DEVICE_FEATURE0);

	return (feature0 & DEVICE_FEATURE0_MPU_CORE_MASK) >>
		DEVICE_FEATURE0_MPU_CORE_SHIFT;
}

static u32 k3_get_cores_r5fss0(void)
{
	u32 feature0 = readl(CTRLMMR_WKUP_DEVICE_FEATURE0);

	return (feature0 & DEVICE_FEATURE0_R5FSS0_CORE_MASK) >>
		DEVICE_FEATURE0_R5FSS0_CORE_SHIFT;
}

static u32 k3_get_cores_r5fss1(void)
{
	u32 feature0 = readl(CTRLMMR_WKUP_DEVICE_FEATURE0);

	return (feature0 & DEVICE_FEATURE0_R5FSS1_CORE_MASK) >>
		DEVICE_FEATURE0_R5FSS1_CORE_SHIFT;
}

static bool k3_has_adc(void)
{
	u32 full_devid = readl(CTRLMMR_WKUP_JTAG_DEVICE_ID);
	u32 devid = (full_devid & JTAG_DEV_ID_MASK) >> JTAG_DEV_ID_SHIFT;

	switch (devid) {
	case JTAG_DEV_ID_AM6442:
	case JTAG_DEV_ID_AM6441:
	case JTAG_DEV_ID_AM6422:
	case JTAG_DEV_ID_AM6421:
		return true;
	default:
		return false;
	}
}

static int k3_has_icss(void)
{
	u32 full_devid = readl(CTRLMMR_WKUP_JTAG_DEVICE_ID);
	u32 feature_code = (full_devid & JTAG_DEV_FEATURES_MASK) >>
			    JTAG_DEV_FEATURES_SHIFT;

	switch (feature_code) {
	case JTAG_DEV_FEATURES_D:
	case JTAG_DEV_FEATURES_E:
	case JTAG_DEV_FEATURES_F:
		return true;
	default:
		return false;
	}
}

int ft_system_setup(void *blob, struct bd_info *bd)
{
	fdt_fixup_cores_nodes_am642(blob, k3_get_cores());
	fdt_fixup_cores_r5_nodes_am642(blob, 0, k3_get_cores_r5fss0());
	fdt_fixup_cores_r5_nodes_am642(blob, 1, k3_get_cores_r5fss1());
	fdt_fixup_adc_node_am642(blob, k3_has_adc());
	fdt_fixup_icss_node_am642(blob, k3_has_icss());

	return 0;
}
