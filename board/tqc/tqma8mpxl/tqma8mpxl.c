// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 - 2022 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <common.h>
#include <errno.h>
#include <asm/io.h>
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

#include "../common/tqc_bb.h"
#include "../common/tqc_blob.h"
#include "../common/tqc_eeprom.h"
#include "../common/tq_som_features.h"

DECLARE_GLOBAL_DATA_PTR;

#define WDOG_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_ODE | PAD_CTL_PUE | PAD_CTL_PE)

static iomux_v3_cfg_t const wdog_pads[] = {
	MX8MP_PAD_GPIO1_IO02__WDOG1_WDOG_B  | MUX_PAD_CTRL(WDOG_PAD_CTRL),
};

int board_early_init_f(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;

	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));
	set_wdog_reset(wdog);

	tqc_bb_board_early_init_f();

	return 0;
}

#if !defined(CONFIG_SPL_BUILD)

/**
 * Global copy of EEPROM data to prevent multiple reads.
 *
 * Data is read during board_late_init
 */
static struct tqc_eeprom_data eeprom;

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

/**
 * Translate detected CPU variante into TQ-Systems SOM variant short name
 *
 * return: string consisting of the name or string indicating unknown variant
 */
static const char *tqc_get_boardname(void)
{
	switch (get_cpu_type()) {
	/* all normal quad core variants */
	case MXC_CPU_IMX8MP:	/* NPU */
	case MXC_CPU_IMX8MP6:	/* w/o NPU */
		return "TQMa8MPQL";
	/* quad lite variant */
	case MXC_CPU_IMX8MPL:
		return "TQMa8MPQLL";
	/* dual core variant */
	case MXC_CPU_IMX8MPD:
		return "TQMa8MPDL";
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
	printf("Board: %s on a %s\n", tqc_get_boardname(),
	       tqc_bb_get_boardname());

	return tqc_bb_checkboard();
}

int board_init(void)
{
	tqc_bb_board_init();

	return 0;
}

static struct tq_som_feature tqma8mpxl_som_features[] = {
	{
		.feature = FEATURE_EMMC,
		.dt_path = "/soc@0/bus@30800000/mmc@30b60000",
	}, {
		.feature = FEATURE_EEPROM,
		.dt_path = "/soc@0/bus@30800000/i2c@30a20000/eeprom@57",
	}, {
		.feature = FEATURE_SPINOR,
		.dt_path = "/soc@0/bus@30800000/spi@30bb0000/flash@0",
	}, {
		.feature = FEATURE_SECELEM,
		/* TODO: no driver yet */
	}, {
		.feature = FEATURE_RTC,
		.dt_path = "/soc@0/bus@30800000/i2c@30a20000/rtc@51",
	},
};

static struct tq_som_feature_list tqma8mpxl_feature_list = {
	.list = tqma8mpxl_som_features,
	.entries = ARRAY_SIZE(tqma8mpxl_som_features),
};

struct tq_som_feature_list *tq_board_detect_features(void)
{
	return &tqma8mpxl_feature_list;
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, bd_t *bd)
{
#ifdef CONFIG_IMX8M_DRAM_INLINE_ECC
#error
#ifdef CONFIG_TARGET_IMX8MP_DDR4_EVK
	int rc;
	phys_addr_t ecc_start = 0x120000000;
	size_t ecc_size = 0x20000000;

	rc = add_res_mem_dt_node(blob, "ecc", ecc_start, ecc_size);
	if (rc < 0) {
		printf("Could not create ecc reserved-memory node.\n");
		return rc;
	}
#else
	int rc;
	phys_addr_t ecc0_start = 0xb0000000;
	phys_addr_t ecc1_start = 0x130000000;
	phys_addr_t ecc2_start = 0x1b0000000;
	size_t ecc_size = 0x10000000;

	rc = add_res_mem_dt_node(blob, "ecc", ecc0_start, ecc_size);
	if (rc < 0) {
		printf("Could not create ecc0 reserved-memory node.\n");
		return rc;
	}

	rc = add_res_mem_dt_node(blob, "ecc", ecc1_start, ecc_size);
	if (rc < 0) {
		printf("Could not create ecc1 reserved-memory node.\n");
		return rc;
	}

	rc = add_res_mem_dt_node(blob, "ecc", ecc2_start, ecc_size);
	if (rc < 0) {
		printf("Could not create ecc2 reserved-memory node.\n");
		return rc;
	}
#endif
#endif

	if (CONFIG_IS_ENABLED(FDT_FIXUP_PARTITIONS)) {
		const char * const path = "/soc@0/bus@30800000/spi@30bb0000";
		const struct node_info nodes[] = {
			{ "jedec,spi-nor",	MTD_DEV_TYPE_NOR, },
			{ "nxp,imx8mp-fspi",	MTD_DEV_TYPE_NOR, },
			/* fallback, used by older TQ BSP kernel */
			{ "nxp,imx8qxp-fspi",	MTD_DEV_TYPE_NOR, },
		};

		if (tq_vard_valid(&eeprom.tq_hw_data.vard)) {
			if (tq_vard_has_spinor(&eeprom.tq_hw_data.vard)) {
				/*
				 * Update MTD partition nodes using info from
				 * mtdparts env var
				 * for [Q]SPI this needs the device probed.
				 */
				puts("   Updating MTD partitions...\n");
				tqc_ft_spi_setup(blob, path, nodes,
						 ARRAY_SIZE(nodes));
			}
		}
	}

	return tqc_bb_ft_board_setup(blob, bd);
}
#endif

int board_late_init(void)
{
	const char *bname = tqc_get_boardname();
	int ret;

	if (CONFIG_IS_ENABLED(I2C_EEPROM))
		ret = tq_read_module_eeprom(&eeprom);
	else
		ret = tqc_read_eeprom_at(0, 0x53, 1, 0, &eeprom);

	if (!ret) {
		tqc_board_handle_eeprom_data(bname, &eeprom);
		if (tq_vard_valid(&eeprom.tq_hw_data.vard)) {
			/*
			 * set quartz load to 7.000 femtofarads
			 * only if RTC is assembled, to prevent warnings
			 */
			if (tq_vard_has_rtc(&eeprom.tq_hw_data.vard)) {
				if (tqc_pcf85063_adjust_capacity(0,
								 0x51,
								 7000))
					puts("PCF85063: adjust error\n");
			}
			/*
			 * fill features present flag from vard
			 * set list to empty if error
			 */
			if (tq_vard_detect_features(&eeprom.tq_hw_data.vard,
						    &tqma8mpxl_feature_list))
				tqma8mpxl_feature_list.entries = 0;
		}
	} else {
		pr_err("EEPROM: read error %d\n", ret);
	}

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", tqc_bb_get_boardname());
	env_set("board_rev", tqc_get_boardname());
#endif

	tqc_bb_board_late_init();

	return 0;
}

#endif
