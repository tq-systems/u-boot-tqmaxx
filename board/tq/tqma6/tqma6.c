// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 *
 * ported SabreSD to TQMa6x
 * Copyright (c) 2013-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <init.h>
#include <asm/arch/clock.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/sys_proto.h>
#include <env.h>
#include <fdt_support.h>
#include <asm/global_data.h>
#include <linux/errno.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <common.h>
#include <linux/libfdt.h>
#include <mmc.h>

#include "tqma6.h"
#include "../common/tq_bb.h"
#include "../common/tq_emmc.h"

DECLARE_GLOBAL_DATA_PTR;

const u16 tqma6_emmc_dsr = 0x0100;

/* board-specific MMC card detection / modification */
void board_mmc_detect_card_type(struct mmc *mmc)
{
	struct mmc *emmc = find_mmc_device(0);

	if (emmc != mmc)
		return;

	if (tq_emmc_need_dsr(mmc) > 0)
		mmc_set_dsr(mmc, tqma6_emmc_dsr);
}

/*!
 * Rev. 0200 and newer optionally implements GIGE errata fix.
 * Use a global var to signal presence of the fix. This should be checked
 * early. If fix is present, system I2C is I2C1 - otherwise I2C3
 */
static int has_enet_workaround = -1;

int tqma6_has_enet_workaround(void)
{
	return has_enet_workaround;
}

#define GPIO_REVDET_PAD_CTRL  (PAD_CTL_PUS_100K_DOWN | PAD_CTL_SPEED_LOW | \
				PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

static const iomux_v3_cfg_t tqma6_revdet_pads[] = {
	MX6_PAD_GPIO_6__GPIO1_IO06 | MUX_PAD_CTRL(GPIO_REVDET_PAD_CTRL),
};

#define TQMA6_REVDET_GPIO IMX_GPIO_NR(1, 6)

void tqma6_detect_enet_workaround(void)
{
#if defined(CONFIG_SPL_BUILD)
	int ret;

	imx_iomux_v3_setup_multiple_pads(tqma6_revdet_pads,
					 ARRAY_SIZE(tqma6_revdet_pads));

	ret = gpio_request(TQMA6_REVDET_GPIO, "tqma6-revdet");
	if (ret) {
		pr_err("error: gpio request for enet workaround %d\n", ret);
	} else {
		gpio_direction_input(TQMA6_REVDET_GPIO);
		if (gpio_get_value(TQMA6_REVDET_GPIO) == 0) {
			has_enet_workaround = 1;
		} else if (gpio_get_value(TQMA6_REVDET_GPIO) > 0) {
			has_enet_workaround = 0;
			gpio_direction_output(TQMA6_REVDET_GPIO, 1);
		}
		gpio_free(TQMA6_REVDET_GPIO);
	}
#else
	int ret;
	struct gpio_desc desc;

	imx_iomux_v3_setup_multiple_pads(tqma6_revdet_pads,
					 ARRAY_SIZE(tqma6_revdet_pads));

	ret = dm_gpio_lookup_name("GPIO1_6", &desc);
	if (ret) {
		pr_err("error: gpio lookup for enet workaround %d\n", ret);
	} else {
		ret = dm_gpio_request(&desc, "rev_det");
		if (ret) {
			pr_err("error: gpio request  for enet workaround %d\n", ret);
		} else {
			dm_gpio_set_dir_flags(&desc, GPIOD_IS_IN);
			if (dm_gpio_get_value(&desc) == 0) {
				has_enet_workaround = 1;
			} else if (dm_gpio_get_value(&desc) > 0) {
				has_enet_workaround = 0;
				dm_gpio_set_dir_flags(&desc, GPIOD_IS_OUT |
						       GPIOD_IS_OUT_ACTIVE);
			}
		}
	}
#endif
}

const char *tqma6_get_fdt_configuration(void)
{
	if (is_mx6dq())
		return !tqma6_has_enet_workaround() ? "imx6q-mba6b" : "imx6q-mba6a";
	if (is_mx6dqp())
		return "imx6qp-mba6b";
	if (is_mx6sdl())
		return !tqma6_has_enet_workaround() ? "imx6dl-mba6b" : "imx6dl-mba6a";

	return NULL;
}

int board_early_init_f(void)
{
	tq_bb_board_early_init_f();

	tqma6_detect_enet_workaround();

	return 0;
}

#if !defined(CONFIG_SPL_BUILD)

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	tq_bb_board_init();

	return 0;
}

static const char *tqma6_get_boardname(void)
{
	u32 cpurev = get_cpu_rev();

	switch ((cpurev & 0xFF000) >> 12) {
	case MXC_CPU_MX6SOLO:
		return "TQMa6S";
	case MXC_CPU_MX6DL:
		return "TQMa6DL";
	case MXC_CPU_MX6D:
		return "TQMa6D";
	case MXC_CPU_MX6DP:
		return "TQMa6DP";
	case MXC_CPU_MX6Q:
		return "TQMa6Q";
	case MXC_CPU_MX6QP:
		return "TQMa6QP";
	default:
		return "??";
	};
}

#define FDTFILE_STRLEN 32u
int board_late_init(void)
{
	char fdtfile[FDTFILE_STRLEN];
	const char *config = tqma6_get_fdt_configuration();

	env_set("board_name", tqma6_get_boardname());

	if (!env_get("fdtfile")) {
		if (config) {
			snprintf(fdtfile, FDTFILE_STRLEN, "%s.dtb", config);
			env_set("fdtfile", fdtfile);
		} else {
			pr_err("ENV: Could not set kernel devicetree, ${fdtfile} remains unset\n");
		}
	}

	tq_bb_board_late_init();
	board_late_mmc_env_init();

	return 0;
}

int checkboard(void)
{
	printf("Board: %s on a %s\n", tqma6_get_boardname(),
	       tq_bb_get_boardname());

	puts("Enet workaround: ");
	switch (tqma6_has_enet_workaround()) {
	case 0:
		puts("NO");
		break;
	case 1:
		puts("OK");
		break;
	default:
		puts("???");
		break;
	};
	puts("\n");

	return 0;
}

#if (CONFIG_IS_ENABLED(LDO_BYPASS_CHECK))
/* TODO, use external pmic, for now always leave default */
void ldo_mode_set(int ldo_bypass)
{
}
#endif

/*
 * Device Tree Support
 */
#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	struct mmc *mmc = find_mmc_device(0);
	int err;

	fdt_fixup_memory(blob, (u64)PHYS_SDRAM, (u64)gd->ram_size);

	/* bring in eMMC dsr settings if needed */
	if (mmc && (!mmc_init(mmc))) {
		if (tq_emmc_need_dsr(mmc) > 0) {
			err = tq_ft_fixup_emmc_dsr(blob,
						   "/soc/bus@2100000/mmc@2198000",
						   (u32)tqma6_emmc_dsr);
			if (err)
				puts("ERROR: failed to patch e-MMC DSR in DT\n");
		}
	} else {
		puts("e-MMC: not present?\n");
	}

	return 0;
}
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */

#endif
