// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 - 2022 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <env.h>
#include <errno.h>
#include <asm/bootm.h>
#include <asm/io.h>
#include <asm/setup.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm-generic/gpio.h>
#include <asm/arch/imx8mp_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/arch/clock.h>
#include <spl.h>
#include <asm/mach-imx/dma.h>
#include <bloblist.h>
#include <jffs2/load_kernel.h>
#include <mtd_node.h>
#include <power/pmic.h>
#include <mmc.h>

#include "../common/tq_bb.h"
#include "../common/tq_blob.h"
#include "../common/tq_board_gpio.h"
#include "../common/tq_eeprom.h"
#include "../common/tq_som_features.h"

DECLARE_GLOBAL_DATA_PTR;

#define WDOG_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_ODE | PAD_CTL_PUE | PAD_CTL_PE)

static const iomux_v3_cfg_t wdog_pads[] = {
	MX8MP_PAD_GPIO1_IO02__WDOG1_WDOG_B  | MUX_PAD_CTRL(WDOG_PAD_CTRL),
};

int board_early_init_f(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;

	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));

	set_wdog_reset(wdog);

	return tq_bb_board_early_init_f();
}

#if !defined(CONFIG_SPL_BUILD)

/**
 * Global copy of EEPROM data to prevent multiple reads.
 *
 * Data is read during board_late_init
 */
static struct tq_eeprom_data eeprom;

#if IS_ENABLED(CONFIG_CMD_TQ_EEPROM_WRITE)

enum {
	EEPROM_WREN,
};

static struct tq_gpio_init_data gpio_init_data[] = {
	GPIO_INIT_DATA_ENTRY(EEPROM_WREN, "GPIO4_5",
			     GPIOD_IS_OUT | GPIOD_ACTIVE_LOW),
};

void tq_bb_eeprom_wren(void)
{
	dm_gpio_set_value(&gpio_init_data[EEPROM_WREN].desc, 1);
}

void tq_bb_eeprom_wrdi(void)
{
	dm_gpio_set_value(&gpio_init_data[EEPROM_WREN].desc, 0);
}

static void tqma8mpxs_init_eeprom(void)
{
	tq_board_gpio_init(gpio_init_data, ARRAY_SIZE(gpio_init_data));
	tq_bb_eeprom_wrdi();
}

#else

static void tqma8mpxs_init_eeprom(void) {}

#endif

#if CONFIG_IS_ENABLED(BLOBLIST)

int board_phys_sdram_size(phys_size_t *size)
{
	struct tq_raminfo *raminfo;

	if (!size)
		return -EINVAL;

	raminfo = bloblist_find(BLOBLISTT_TQ_RAMSIZE, sizeof(*raminfo));
	if (!raminfo) {
		printf("Unable to find RAMSIZE blob from SPL\n");
		return -ENOENT;
	}

	*size = raminfo->memsize;

	return 0;
}

#endif

/**
 * Translate detected CPU variante into TQ-Systems SOM variant short name
 *
 * return: string consisting of the name or string indicating unknown variant
 */
const char *tq_get_boardname(void)
{
	switch (get_cpu_type()) {
	/* all normal quad core variants */
	case MXC_CPU_IMX8MP:	/* NPU */
	case MXC_CPU_IMX8MP6:	/* w/o NPU */
		return "TQMa8MPQS";
	/* quad lite variant */
	case MXC_CPU_IMX8MPL:
		return "TQMa8MPQLS";
	/* dual core variant */
	case MXC_CPU_IMX8MPD:
		return "TQMa8MPDS";
	/* add more if new variants will be assembled */
	default:
		return "??";
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

int checkboard(void)
{
	print_bootinfo();
	printf("Board: %s on a %s\n", tq_get_boardname(),
	       tq_bb_get_boardname());
	if (IS_ENABLED(CONFIG_IMX8M_DRAM_INLINE_ECC))
		puts("ECC: Inline\n");
	else
		puts("ECC: None\n");

	return tq_bb_checkboard();
}

int board_init(void)
{
	tqma8mpxs_init_eeprom();

	tq_bb_board_init();

	return 0;
}

static struct tq_som_feature tqma8mpxs_som_features[] = {
	{
		.feature = FEATURE_EMMC,
		.dt_path = "/soc@0/bus@30800000/mmc@30b60000",
	}, {
		.feature = FEATURE_EEPROM,
		.dt_path = "/soc@0/bus@30800000/i2c@30a20000/eeprom@54",
	}, {
		.feature = FEATURE_EEPROM,
		.dt_path = "/soc@0/bus@30800000/i2c@30a20000/eeprom@5c",
	}, {
		.feature = FEATURE_IMU,
		.dt_path = "/soc@0/bus@30800000/i2c@30a20000/imu@6b",
	}, {
		.feature = FEATURE_RTC,
		.dt_path = "/soc@0/bus@30800000/i2c@30a20000/rtc@51",
	}, {
		.feature = FEATURE_SPINOR,
		.dt_path = "/soc@0/bus@30800000/spi@30bb0000/flash@0",
	}, {
		.feature = FEATURE_SECELEM,
		/* TODO: no driver yet */
	}, {
		.feature = FEATURE_TEMP_DISCRETE,
		.dt_path = "/soc@0/bus@30800000/i2c@30a30000/temperature-sensor@4a",
	},
};

static struct tq_som_feature_list tqma8mpxs_feature_list = {
	.list = tqma8mpxs_som_features,
	.entries = ARRAY_SIZE(tqma8mpxs_som_features),
};

const struct tq_som_feature_list *tq_board_detect_features(void)
{
	return &tqma8mpxs_feature_list;
}

#ifdef CONFIG_OF_BOARD_SETUP
int fixup_ecc_reserved_mem(void *blob, struct bd_info *bis)
{
	int ret = 0;

	if (IS_ENABLED(CONFIG_IMX8M_DRAM_INLINE_ECC)) {
		phys_size_t ram_size = 0x0;
		phys_addr_t ecc_start = 0x0;
		phys_size_t ecc_size = 0x0;

		board_phys_sdram_size(&ram_size);

		/*
		 * In case of inline ECC, the available ram size has been reduced by
		 * SPL to 7/8 of total ram size, leaving 1/8 untouched for ECC parity
		 * data.
		 * The reserved memory node has therefore a size of 1/7 of the
		 * available ram size (as returned by board_phys_sdram_size), starting
		 * on top of the available ram space.
		 */
		ecc_size = ram_size / 7ULL;
		ecc_start = CFG_SYS_SDRAM_BASE + ram_size;

		ret = add_res_mem_dt_node(blob, "ecc", ecc_start, ecc_size);
		if (ret < 0) {
			printf("Could not create ecc reserved-memory node.\n");
			return ret;
		}
	}

	return ret;
}

int ft_board_setup(void *blob, struct bd_info *bis)
{
	fixup_ecc_reserved_mem(blob, bis);

	if (IS_ENABLED(CONFIG_FDT_FIXUP_PARTITIONS)) {
		const char * const path = "/soc@0/bus@30800000/spi@30bb0000";
		const struct node_info nodes[] = {
			{ "jedec,spi-nor",	MTD_DEV_TYPE_NOR, },
		};

		if (tq_vard_valid(&eeprom.tq_hw_data.vard)) {
			if (tq_vard_has_spinor(&eeprom.tq_hw_data.vard)) {
				/*
				 * Update MTD partition nodes using info from
				 * mtdparts env var
				 * for [Q]SPI this needs the device probed.
				 */
				puts("   Updating MTD partitions...\n");
				tq_ft_spi_setup(blob, path, nodes,
						ARRAY_SIZE(nodes));
			}
		}
	}

	return tq_bb_ft_board_setup(blob, bis);
}
#endif

int board_late_init(void)
{
	const char *bname = tq_get_boardname();
	int ret;
	bool features_detected = false;

	ret = tq_read_module_eeprom(&eeprom);

	if (!ret) {
		tq_board_handle_eeprom_data(bname, &eeprom);
		if (tq_vard_valid(&eeprom.tq_hw_data.vard)) {
			/*
			 * set quartz load to 7.000 femtofarads
			 * only if RTC is assembled, to prevent warnings
			 */
			if (tq_vard_has_rtc(&eeprom.tq_hw_data.vard)) {
				if (tq_pcf85063_adjust_capacity(0,
								0x51,
								7000))
					puts("PCF85063: adjust error\n");
			}
			/*
			 * fill feature presence flags from vard
			 */
			if (!tq_vard_detect_features(&eeprom.tq_hw_data.vard,
						     &tqma8mpxs_feature_list))
				features_detected = true;
		}
	} else {
		pr_err("EEPROM: read error %d\n", ret);
	}

	/* mark list as empty to prevent further processing */
	if (!features_detected) {
		printf("VARD: data not present or invalid,\n"
		       "      no fixup in DT for optional devices!\n");
		tqma8mpxs_feature_list.entries = 0;
	}

	if (IS_ENABLED(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG)) {
		struct tag_serialnr serialnr;

		/*
		 * CPU UID Query - get_board_serial returns only part of the
		 * UID but prints complete UID data for i.MX8MP SOC
		 */
		get_board_serial(&serialnr);

		env_set("board_name", tq_bb_get_boardname());
		env_set("board_rev", tq_get_boardname());
	}

	tq_bb_board_late_init();

	return 0;
}

#endif
