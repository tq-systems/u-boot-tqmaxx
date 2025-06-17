/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Common board code for TQ-Systems SOMs based on the Texas Instruments K3 SoCs families
 *
 * Copyright (c) 2020-2025 TQ-Systems GmbH <u-boot@ew.tq-group.com>, D-82229 Seefeld, Germany.
 */

#include <env_internal.h>
#include <init.h>
#include <mmc.h>
#include <spl.h>
#include <sysinfo.h>
#include <asm/arch/k3-ddr.h>
#include <asm/hardware.h>
#include <linux/sizes.h>
#include "common.h"

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	u64 bank0_size, bank1_size = 0;
	struct udevice *sysinfo;
	int ret;

	if (!IS_ENABLED(CONFIG_CPU_V7R))
		return fdtdec_setup_memory_banksize();

	ret = sysinfo_get_and_detect(&sysinfo);
	if (ret) {
		printf("Failed to get sysinfo data: %d\n", ret);
		return ret;
	}

	ret = sysinfo_get_uint64(sysinfo, SYSID_RAM_SIZE, &bank0_size);
	if (ret) {
		printf("Failed to get RAM size: %d\n", ret);
		return ret;
	}

	if (bank0_size > SZ_2G) {
		bank1_size = bank0_size - SZ_2G;
		bank0_size = SZ_2G;
	}

	gd->bd->bi_dram[0].start = CFG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = bank0_size;

	if (CONFIG_NR_DRAM_BANKS > 1) {
		gd->bd->bi_dram[1].start = CFG_SYS_SDRAM_BASE1;
		gd->bd->bi_dram[1].size = bank1_size;
	}

	return 0;
}

#ifdef CONFIG_XPL_BUILD

void spl_perform_fixups(struct spl_image_info *spl_image)
{
	if (IS_ENABLED(CONFIG_K3_DDRSS) && IS_ENABLED(CONFIG_K3_INLINE_ECC))
		fixup_ddr_driver_for_ecc(spl_image);
	else
		fixup_memory_node(spl_image);

	/* Apply USB dr_mode fixup */
	fdtdec_board_setup(spl_image->fdt_addr);
}

#else /* CONFIG_XPL_BUILD */

void tq_common_setup_boot_targets(void)
{
	const char *boot_target = NULL;

	if (env_get("boot_targets"))
		return;

	switch (get_boot_device()) {
	case BOOT_DEVICE_MMC1:
		boot_target = "mmc0";
		break;
	case BOOT_DEVICE_MMC2:
		boot_target = "mmc1";
		break;
	case BOOT_DEVICE_SPI:
		boot_target = "spi_flash0";
		break;
	case BOOT_DEVICE_USB:
		boot_target = "usb0";
		break;
	default:
		printf("Warning: unknown boot device\n");
	}

	if (boot_target)
		env_set_runtime("boot_targets", boot_target);
}

#if IS_ENABLED(CONFIG_TQ_COMMON_AUTO_ENV_LOCATION)

enum env_location env_get_location(enum env_operation op, int prio)
{
	if (prio > 0)
		return ENVL_UNKNOWN;

	switch (get_boot_device()) {
#if CONFIG_IS_ENABLED(ENV_IS_IN_MMC)
	case BOOT_DEVICE_MMC1:
	case BOOT_DEVICE_MMC2:
		return ENVL_MMC;
#endif
#if CONFIG_IS_ENABLED(ENV_IS_IN_SPI_FLASH)
	case BOOT_DEVICE_SPI:
		return ENVL_SPI_FLASH;
#endif
	default:
		return ENVL_NOWHERE;
	}
}

#if CONFIG_IS_ENABLED(ENV_IS_IN_MMC)
int mmc_get_env_dev(void)
{
	switch (get_boot_device()) {
	case BOOT_DEVICE_MMC1:
		return 0;
	case BOOT_DEVICE_MMC2:
		return 1;
	default:
		return CONFIG_ENV_MMC_DEVICE_INDEX;
	}
}

uint mmc_get_env_part(struct mmc *mmc)
{
	if (get_boot_device() == BOOT_DEVICE_MMC1 &&
	    get_mmc_boot_mode() == MMCSD_MODE_EMMCBOOT) {
		int bootpart = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);

		if (bootpart == 1 || bootpart == 2)
			return bootpart;
	}

	return 0;
}

#endif

#endif

#endif /* CONFIG_XPL_BUILD */
