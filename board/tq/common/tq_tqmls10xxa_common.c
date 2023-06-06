// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Gregor Herburger
 */

#include <asm/global_data.h>
#include <fdt_support.h>
#include <fsl_ddr_sdram.h>
#include <i2c.h>
#include <jffs2/load_kernel.h>
#include <linux/printk.h>
#include <mtd_node.h>
#include <net.h>
#include <netdev.h>
#include <spi_flash.h>

#include "../common/tq_bb.h"

#define TQMLS10XXA_SYSC_BUS_NUM     0
#define TQMLS10XXA_SYSC_ADDR        0x11

#define SYSC_REG_SYSC_FW_VERS       0x02
#define SYSC_REG_BOOT_SRC           0x03
#define SYSC_REG_BOOT_SRC_SDSEL_MSK 0x80
#define SYSC_REG_CPLD_FW_VERS       0xE1

#define TQMLS10XXA_SYSC_BOOT_SRC_MASK		0xF
#define TQMLS10XXA_SYSC_BOOT_SRC_QSPI		0x0
#define TQMLS10XXA_SYSC_BOOT_SRC_SD		0x2
#define TQMLS10XXA_SYSC_BOOT_SRC_EMMC		0x3
#define TQMLS10XXA_SYSC_BOOT_SRC_HARD_CODED	0xe

DECLARE_GLOBAL_DATA_PTR;

#if IS_ENABLED(CONFIG_OF_BOARD_FIXUP)
int board_fix_fdt(void *blob)
{
	struct udevice *dev;
	int offset, sdsel;
#if defined(CONFIG_FSL_LSCH3)
	static const char esdhc_node[] = "/esdhc@2140000";
#elif defined(CONFIG_FSL_LSCH2)
	static const char esdhc_node[] = "/soc/esdhc@1560000";
#endif
	/* get offset of sdhc node */
	offset = fdt_path_offset(blob, esdhc_node);

	i2c_get_chip_for_busnum(TQMLS10XXA_SYSC_BUS_NUM, TQMLS10XXA_SYSC_ADDR, 1, &dev);
	sdsel = dm_i2c_reg_read(dev, SYSC_REG_BOOT_SRC) & SYSC_REG_BOOT_SRC_SDSEL_MSK;

	fdt_increase_size(blob, 32);
	if (!sdsel) {
		/* eMMC */
		fdt_delprop(blob, offset, "cd-gpios");
		fdt_delprop(blob, offset, "wp-gpios");
		fdt_setprop_empty(blob, offset, "non-removable");
		fdt_setprop_empty(blob, offset, "mmc-hs200-1_8v");
	} else {
		/* SD-Card */
		fdt_setprop_empty(blob, offset, "no-1-8-v");
	}

	return tq_bb_board_fix_fdt(blob);
}
#endif

int fsl_initdram(void)
{
	gd->ram_size = tfa_get_dram_size();
	if (!gd->ram_size)
		gd->ram_size = fsl_ddr_sdram_size();

	return 0;
}

void tq_tqmls10xx_checkboard(void)
{
	u8 bootsrc, syscrev, cpldrev;
	struct udevice *dev;

	/* get further information from SysC */
	i2c_get_chip_for_busnum(TQMLS10XXA_SYSC_BUS_NUM, TQMLS10XXA_SYSC_ADDR, 1, &dev);
	bootsrc = dm_i2c_reg_read(dev, SYSC_REG_BOOT_SRC);
	syscrev = dm_i2c_reg_read(dev, SYSC_REG_SYSC_FW_VERS);
	cpldrev = dm_i2c_reg_read(dev, SYSC_REG_CPLD_FW_VERS);

	/* print SoM and baseboard name */
	switch (bootsrc & TQMLS10XXA_SYSC_BOOT_SRC_MASK) {
	case TQMLS10XXA_SYSC_BOOT_SRC_QSPI:
		puts("(Boot from QSPI)\n");
		break;
	case TQMLS10XXA_SYSC_BOOT_SRC_SD:
		puts("(Boot from SD)\n");
		break;
	case TQMLS10XXA_SYSC_BOOT_SRC_EMMC:
		puts("(Boot from eMMC)\n");
		break;
	case TQMLS10XXA_SYSC_BOOT_SRC_HARD_CODED:
		puts("(Boot from Hard Coded RCW)\n");
		break;
	default:
		puts("(Bootsource unknown)\n");
		break;
	}

	printf("         SysC FW Rev: %2d.%02d\n", (syscrev >> 4) & 0xF, syscrev & 0xF);
	printf("         CPLD FW Rev: %2d.%02d\n", (cpldrev >> 4) & 0xF, cpldrev & 0xF);
}

#if IS_ENABLED(CONFIG_FDT_FIXUP_PARTITIONS)
static void tqmls10xxa_fixup_partitions(void *blob, struct bd_info *bd)
{
	struct node_info nodes[] = {
		{"jedec,spi-nor", MTD_DEV_TYPE_NOR},
	};
	const unsigned int bus = CONFIG_SF_DEFAULT_BUS;
	const unsigned int cs = CONFIG_SF_DEFAULT_CS;
	const unsigned int speed = CONFIG_SF_DEFAULT_SPEED;
	const unsigned int mode = CONFIG_SF_DEFAULT_MODE;
	struct udevice *dev;
	int ret;

	ret = spi_flash_probe_bus_cs(bus, cs, speed, mode, &dev);
	if (!ret)
		fdt_fixup_mtdparts(blob, nodes, ARRAY_SIZE(nodes));
}
#endif

int ft_board_setup(void *blob, struct bd_info *bd)
{
	struct udevice *dev;
	u8 bootsrc;
	int offset;
	/* Attention: Path may change to /soc/mmc@ in future linux releases. */
#if defined(CONFIG_FSL_LSCH3)
	static const char esdhc_node[] = "/soc/esdhc@2140000";
#elif defined(CONFIG_FSL_LSCH2)
	static const char esdhc_node[] = "/soc/esdhc@1560000";
#endif

	arch_fixup_fdt(blob);
	ft_cpu_setup(blob, bd);

	/* get sdhc mux information from SysC */
	i2c_get_chip_for_busnum(TQMLS10XXA_SYSC_BUS_NUM, TQMLS10XXA_SYSC_ADDR, 1, &dev);
	bootsrc = dm_i2c_reg_read(dev, SYSC_REG_BOOT_SRC);

	/* get offset of sdhc node */
	offset = fdt_path_offset(blob, esdhc_node);
	if (offset < 0)
		return offset;

	/* delete eMMC specific properties if sd-card selected */
	if (bootsrc & SYSC_REG_BOOT_SRC_SDSEL_MSK) {
		/* SDHC_EXT_SEL = 1 => sd-card */
		fdt_delprop(blob, offset, "non-removable");
		fdt_delprop(blob, offset, "disable-wp");
		fdt_delprop(blob, offset, "mmc-hs200-1_8v");
		fdt_setprop_empty(blob, offset, "no-1-8-v");
	}

#if (IS_ENABLED(CONFIG_FDT_FIXUP_PARTITIONS))
	tqmls10xxa_fixup_partitions(blob, bd);
#endif

	return tq_bb_ft_board_setup(blob, bd);
}

void tq_tqmls10xxa_set_macaddrs(u8 *macaddr, int count)
{
	int i;

	for (i = 0; i < count; i++) {
		eth_env_set_enetaddr_by_index("eth", i, macaddr);

		if (++macaddr[5])
			continue;
		if (++macaddr[4])
			continue;
		if (++macaddr[3])
			continue;

		printf("Warning: MAC address wrapped around to %pM\n", macaddr);
	}
}

#if IS_ENABLED(CONFIG_MISC_INIT_R)
int misc_init_r(void)
{
	return tq_bb_board_misc_init_r();
}
#endif
