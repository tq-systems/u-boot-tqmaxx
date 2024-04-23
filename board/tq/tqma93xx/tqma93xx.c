// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <common.h>
#include <env.h>
#include <init.h>
#include <mtd_node.h>
#include <spl.h>
#include <asm/global_data.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch-imx9/ccm_regs.h>
#include <asm/arch-imx9/imx93_pins.h>
#include <asm/mach-imx/boot_mode.h>
#include <jffs2/load_kernel.h>

#include "../common/tq_bb.h"
#include "../common/tq_eeprom.h"
#include "../common/tq_som_features.h"

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	tq_bb_board_early_init_f();

	return 0;
}

#if !defined(CONFIG_SPL_BUILD)

/**
 * Global copy of EEPROM data to prevent multiple reads.
 *
 * Data is read during board_late_init
 */
static struct tq_eeprom_data eeprom;

/**
 * Translate detected CPU variant into TQ-Systems SOM variant short name
 *
 * return: string consisting of the name or string indicating unknown variant
 */
static const char *tq_get_boardname(void)
{
	switch (get_cpu_type()) {
	case MXC_CPU_IMX93:
		return "TQMa9352"; /* iMX93 Dual core with NPU */
	case MXC_CPU_IMX9351:
		return "TQMa9351"; /* iMX93 Single core with NPU */
	case MXC_CPU_IMX9332:
		return "TQMa9332"; /* iMX93 Dual core without NPU */
	case MXC_CPU_IMX9331:
		return "TQMa9331"; /* iMX93 Single core without NPU */
	}

	return "UNKNOWN";
}

int print_bootinfo(void)
{
	enum boot_device bt_dev;

	bt_dev = get_boot_device();

	puts("Boot:  ");
	switch (bt_dev) {
	case SD1_BOOT:
		puts("USDHC1(SD)\n");
		break;
	case SD2_BOOT:
		puts("USDHC2(SD)\n");
		break;
	case SD3_BOOT:
		puts("USDHC3(SD)\n");
		break;
	case MMC1_BOOT:
		puts("USDHC1(e-MMC)\n");
		break;
	case MMC2_BOOT:
		puts("USDHC2(e-MMC)\n");
		break;
	case MMC3_BOOT:
		puts("USDHC3(e-MMC)\n");
		break;
	case USB_BOOT:
		puts("USB\n");
		break;
	case QSPI_BOOT:
		puts("FlexSPI\n");
		break;
	default:
		printf("Unknown/Unsupported device %u\n", bt_dev);
		break;
	}

	return 0;
}

/**
 * print some useful board information
 *
 * When not calling late, VARD is not read yet. Trying to read EEPROM here
 * will succeed but for unknown reason calling imx9_probe_mu from event callback
 * EVT_DM_POST_INIT will lead to system failure, since DT data seems corrupt
 * when EVT_DM_POST_INIT will be issued from initr_dm main device model init.
 * TODO: check if using DISPLAY_BOARDINFO_LATE
 */
int checkboard(void)
{
	print_bootinfo();
	printf("Board: %s on a %s\n", tq_get_boardname(),
	       tq_bb_get_boardname());

	return tq_bb_checkboard();
}

int board_init(void)
{
	tq_bb_board_init();

	return 0;
}

static struct tq_som_feature tqma93xx_som_features[] = {
	{
		.feature = FEATURE_EMMC,
		.dt_path = "/soc@0/bus@42800000/mmc@42850000",
	}, {
		.feature = FEATURE_EEPROM,
		.dt_path = "/soc@0/bus@44000000/i2c@44340000/eeprom@57",
	}, {
		.feature = FEATURE_RTC,
		.dt_path = "/soc@0/bus@44000000/i2c@44340000/rtc@51",
	}, {
		.feature = FEATURE_SECELEM,
		/* TODO: no driver yet */
	}, {
		.feature = FEATURE_SPINOR,
		.dt_path = "/soc@0/bus@42000000/spi@425e0000/flash@0",
	},
};

static struct tq_som_feature_list tqma93xx_feature_list = {
	.list = tqma93xx_som_features,
	.entries = ARRAY_SIZE(tqma93xx_som_features),
};

const struct tq_som_feature_list *tq_board_detect_features(void)
{
	return &tqma93xx_feature_list;
}

#if CONFIG_IS_ENABLED(OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	if (CONFIG_IS_ENABLED(FDT_FIXUP_PARTITIONS)) {
		const char * const path = "/soc@0/bus@42000000/spi@425e0000";
		static const struct node_info nodes[] = {
			{ "jedec,spi-nor",	MTD_DEV_TYPE_NOR, },
		};

		if (tq_vard_valid(&eeprom.tq_hw_data.vard)) {
			if (tq_vard_has_spinor(&eeprom.tq_hw_data.vard)) {
				/*
				 * Update MTD partition nodes using info
				 * from mtdparts env var
				 * for [Q]SPI this needs the device probed.
				 */
				puts("   Updating MTD partitions...\n");
				tq_ft_spi_setup(blob, path, nodes,
						ARRAY_SIZE(nodes));
			}
		}
	}

	return tq_bb_ft_board_setup(blob, bd);
}
#endif

/* see MAX_CMDLINE_SIZE in boot/bootm.c */
#define BOARD_CMDLINE_SIZE SZ_4K
static char board_bootargs[BOARD_CMDLINE_SIZE];

char *board_fdt_chosen_bootargs(void)
{
	char *bootargs = env_get("bootargs");
	static const char *cortexm_args = "clk-imx93.mcore_booted=1";

	if (arch_auxiliary_core_check_up(0)) {
		size_t len = strlen(bootargs);

		board_bootargs[0] = '\0';
		strlcat(board_bootargs, bootargs, sizeof(board_bootargs));
		strlcat(board_bootargs, " ", sizeof(board_bootargs));
		strlcat(board_bootargs, cortexm_args, sizeof(board_bootargs));

		return board_bootargs;
	}

	return bootargs;
}

int board_late_init(void)
{
	const char *bname = tq_get_boardname();
	bool features_detected = false;
	int ret;

	ret = tq_read_module_eeprom(&eeprom);

	if (!ret) {
		tq_board_handle_eeprom_data(bname, &eeprom);
		if (tq_vard_valid(&eeprom.tq_hw_data.vard)) {
			/*
			 * set quartz load to 7.000 femtofarads
			 * only if RTC is assembled, to prevent warnings
			 */
			if (tq_vard_has_rtc(&eeprom.tq_hw_data.vard)) {
				/* set quartz load to 7.000 femtofarads */
				if (tq_pcf85063_adjust_capacity(0, 0x51, 7000))
					puts("PCF85063A: adjust error\n");
			}
			/* fill feature presence flags from vard */
			if (!tq_vard_detect_features(&eeprom.tq_hw_data.vard,
						     &tqma93xx_feature_list))
				features_detected = true;
		}
	} else {
		puts("EEPROM: read error\n");
	}

	/* mark list as empty to prevent further processing */
	if (!features_detected) {
		printf("VARD: data not present or invalid,\n"
		       "      no fixup in DT for optional devices!\n");
		tqma93xx_feature_list.entries = 0;
	}

	if (CONFIG_IS_ENABLED(ENV_VARS_UBOOT_RUNTIME_CONFIG)) {
		env_set("board_name", tq_bb_get_boardname());
		env_set("board_rev", bname);

		if (CONFIG_IS_ENABLED(AHAB_BOOT))
			env_set("sec_boot", "yes");
		else
			env_set("sec_boot", "no");
	}

	tq_bb_board_late_init();

	return 0;
}

#endif
