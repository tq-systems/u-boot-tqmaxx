// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2021 - 2022 TQ-Systems GmbH
 * Markus Niebel <Markus.Niebel@tq-group.com>
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
#include <jffs2/load_kernel.h>
#include <mtd_node.h>
#include <power/pmic.h>
#include <mmc.h>

#include "../common/tqc_bb.h"
#include "../common/tqc_eeprom.h"

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

static const char *tqc_get_boardname(void)
{
	switch (get_cpu_type()) {
	case MXC_CPU_IMX8MP:
		return "TQMa8MP";
	case MXC_CPU_IMX8MP6:
		return "TQMa8MP6";
	case MXC_CPU_IMX8MPL:
		return "TQMa8MPL";
	case MXC_CPU_IMX8MPD:
		return "TQMa8MPD";
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

	const char * const path = "/bus@5d000000/spi@5d120000";
	static const struct node_info nodes[] = {
		{ "jedec,spi-nor",	MTD_DEV_TYPE_NOR, },
		{ "nxp,imx8qxp-fspi",	MTD_DEV_TYPE_NOR, },
	};

	tqc_ft_spi_setup(blob, path, nodes, ARRAY_SIZE(nodes));

	return tqc_bb_ft_board_setup(blob, bd);
}
#endif

int board_late_init(void)
{
	struct tqc_eeprom_data eeprom;
	const char *bname = tqc_get_boardname();
	int ret;

	if (CONFIG_IS_ENABLED(I2C_EEPROM))
		ret = tq_read_module_eeprom(&eeprom);
	else
		ret = tqc_read_eeprom_at(0, 0x53, 1, 0, &eeprom);

	if (ret)
		puts("EEPROM: read error\n");
	else
		tqc_board_handle_eeprom_data(bname, &eeprom);

	/* set quartz load to 7.000 femtofarads */
	if (tqc_pcf85063_adjust_capacity(0, 0x51, 7000))
		puts("PCF85063: adjust error\n");

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", tqc_bb_get_boardname());
	env_set("board_rev", tqc_get_boardname());
#endif

	tqc_bb_board_late_init();

	return 0;
}

#endif

#ifdef CONFIG_IMX_BOOTAUX
ulong board_get_usable_ram_top(ulong total_size)
{
	/*
	 * Reserve 16M memory used by M core vring/buffer, which begins at
	 * 16MB before optee
	 */
	if (rom_pointer[1])
		return gd->ram_top - SZ_16M;

	return gd->ram_top;
}
#endif
