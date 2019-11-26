/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2018 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/uclass-internal.h>
#include <malloc.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm-generic/gpio.h>
#include <fsl_esdhc.h>
#include <mmc.h>
#include <asm/arch/imx8mq_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/arch/clock.h>
#include <spl.h>
#include <power/pmic.h>
#include <power/pfuze100_pmic.h>
// #include "../common/pfuze.h"
#include "../common/tqc_bb.h"

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_FSL_QSPI) && defined(CONFIG_DM_SPI)

static int board_qspi_init(void)
{
	struct udevice *bus;
	struct uclass *uc;
	int count = 0;
	int ret;

	set_clk_qspi();

	ret = uclass_get(UCLASS_SPI, &uc);
	if (ret)
		return ret;

	uclass_foreach_dev(bus, uc) {
		/* init SPI controllers */
		printf("SPI%d:   ", count);
		count++;

		ret = device_probe(bus);
		if (ret == -ENODEV) {	/* No such device. */
			puts("SPI not available.\n");
			continue;
		}

		if (ret) {		/* Other error. */
			printf("probe failed, error %d\n", ret);
			continue;
		}

		puts("\n");
	}

	return 0;
}
#else
static inline int board_qspi_init(void) { return 0; }
#endif

int board_early_init_f(void)
{
	return tqc_bb_board_early_init_f();
}

#ifdef CONFIG_BOARD_POSTCLK_INIT
int board_postclk_init(void)
{
	/* TODO */
	return 0;
}
#endif

int dram_init(void)
{
	/* rom_pointer[1] contains the size of TEE occupies */
	if (rom_pointer[1])
		gd->ram_size = PHYS_SDRAM_SIZE - rom_pointer[1];
	else
		gd->ram_size = PHYS_SDRAM_SIZE;

	return 0;
}

#ifdef CONFIG_OF_BOARD_SETUP
static void tqc_delete_node(void *blob, const char *nodepath) {
	int nodeoff;
	int rc;

	nodeoff = fdt_path_offset(blob, nodepath);
	if (nodeoff < 0) {
		printf("Unable to find %s\n", nodepath);
	} else {
		rc = fdt_del_node(blob, nodeoff);
		if (rc < 0)
			printf("Unable to delete %s, err=%s\n",
				nodepath, fdt_strerror(rc));
	}
}

static const char *rev010x_delete_nodes[] = {
	"/tqma8mx-vdd-arm",
	"/reg_tqma8mx_overdrive",
};

int ft_board_setup(void *blob, bd_t *bd)
{
	u32 rev = get_cpu_rev() & 0xfff;
	int i;

	/*
	 * TODO: only temporary supported. REV.010x with old CPU rev.
	 * is not intended for mass production
	 */
	if (rev < CHIP_REV_2_1) {
		int nodeoff;
		int rc;

		printf("cleanup dt for old CPU rev.\n");

		nodeoff = fdt_path_offset(blob, "/cpus/cpu@0/");
		if (nodeoff < 0) {
			printf("Unable to find /cpus/cpu@0\n");
		} else {
			rc = fdt_delprop(blob, nodeoff, "dc-supply");
			if (rc < 0)
				printf("Unable to delete prop dc-supply, err=%s\n",
					fdt_strerror(rc));
			rc = fdt_delprop(blob, nodeoff, "arm-supply");
			if (rc < 0)
				printf("Unable to delete prop arm-supply, err=%s\n",
					fdt_strerror(rc));
		}

		for (i = 0; i < ARRAY_SIZE(rev010x_delete_nodes); ++i)
			tqc_delete_node(blob, rev010x_delete_nodes[i]);
	}

	return tqc_bb_ft_board_setup(blob, bd);
}
#endif

int board_init(void)
{
	board_qspi_init();

	return tqc_bb_board_init();
}

static const char *tqma8mx_get_boardname(void)
{
	switch (get_cpu_type()) {
	case MXC_CPU_IMX8MD:
		return "TQMa8MD";
		break;
	case MXC_CPU_IMX8MQ:
		return "TQMa8MQ";
		break;
	case MXC_CPU_IMX8MQL:
		return "TQMa8MQL";
		break;
	default:
		return "??";
	};
	return "UNKNOWN";
}

int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", tqc_bb_get_boardname());
	env_set("board_rev", tqma8mx_get_boardname());
#endif

	return tqc_bb_board_late_init();
}

int checkboard(void)
{
	printf("Board: %s on a %s\n", tqma8mx_get_boardname(),
	       tqc_bb_get_boardname());
	return 0;
}
