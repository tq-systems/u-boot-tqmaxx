// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Module initialization for TQ-Systems TQMa64xxL
 *
 * Copyright (c) 2017-2018 Texas Instruments Incorporated - http://www.ti.com/
 * Copyright (c) 2020-2025 TQ-Systems GmbH <u-boot@ew.tq-group.com>, D-82229 Seefeld, Germany.
 * Author: Nora Schiffer
 */

#include <asm/arch/hardware.h>
#include <command.h>
#include <init.h>
#include <linux/sizes.h>
#include <spl.h>
#include <sysinfo/tq_eeprom.h>

enum {
	RAM_TYPE_DEFAULT = 1,
	RAM_TYPE_NANYA = 2,
};

#ifdef CONFIG_XPL_BUILD

#define CTRLMMR_USB0_PHY_CTRL	(WKUP_CTRL_MMR0_BASE + 0x4008)
#define CORE_VOLTAGE		0x80000000

void spl_board_init(void)
{
	u32 val;
	/* Set USB PHY core voltage to 0.85V */
	val = readl(CTRLMMR_USB0_PHY_CTRL);
	val &= ~(CORE_VOLTAGE);
	writel(val, CTRLMMR_USB0_PHY_CTRL);
}

int spl_board_init_f(void)
{
	struct udevice *sysinfo;
	u64 ram_expected_size = 0;
	u64 ram_size = 0;
	int ram_type = 0;
	int ret;

	ret = sysinfo_get_and_detect(&sysinfo);
	if (ret) {
		printf("Failed to get sysinfo data: %d\n", ret);
		return 1;
	}

	ret = sysinfo_get_int(sysinfo, SYSID_TQ_RAM_TYPE, &ram_type);
	if (ret) {
		printf("Failed to get RAM type: %d.\n", ret);
		return 1;
	}

	/**
	 * Temporary workaround for Nanya only:
	 * RAM is sometimes initialized in an unusable state, causing
	 * get_ram_size to return a value of 0 or only a few bytes.
	 * Work around this by soft resetting the module if the accessible RAM
	 * doesnt match the configured RAM size in VARD.
	 */
	if (ram_type != RAM_TYPE_NANYA) 
		return 0;

	ret = sysinfo_get_uint64(sysinfo, SYSID_RAM_SIZE, &ram_expected_size);
	if (ret) {
		printf("Failed to get RAM size: %d. Using fallback check instead.\n", ret);
		ram_expected_size = SZ_128M;
	}

	ram_size = get_ram_size((void *)CFG_SYS_SDRAM_BASE, ram_expected_size);

	if (ram_size < ram_expected_size) {
		printf("\nResetting to workaround Nanya RAM issue\n");
		do_reset(NULL, 0, 0, NULL);
		panic("Reset failed. Please reset the board!\n");

		return 1;
	}

	return 0;
}

#else /* CONFIG_XPL_BUILD */

size_t tq_common_sysinfo_macaddr_num(void) {
	return k3_has_icss() ? 4 : 1;
}

static const char *tqma64xxl_cpu_type(void)
{
	u32 device_id = readl(CTRLMMR_WKUP_JTAG_DEVICE_ID);

	switch ((device_id & JTAG_DEV_ID_MASK) >> JTAG_DEV_ID_SHIFT) {
	case JTAG_DEV_ID_AM6442:
		return "AM6442";
	case JTAG_DEV_ID_AM6441:
		return "AM6441";
	case JTAG_DEV_ID_AM6422:
		return "AM6422";
	case JTAG_DEV_ID_AM6421:
		return "AM6421";
	case JTAG_DEV_ID_AM6412:
		return "AM6412";
	case JTAG_DEV_ID_AM6411:
		return "AM6411";
	default:
		return "unknown";
	}
}

int checkboard(void)
{
	printf("SoC variant: %s\n", tqma64xxl_cpu_type());
	return 0;
}

#endif /* CONFIG_XPL_BUILD */
