// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024-2025 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <dwc3-uboot.h>
#include <env.h>
#include <fdt_support.h>
#include <init.h>
#include <mtd_node.h>
#include <usb.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch-imx9/ccm_regs.h>
#include <jffs2/load_kernel.h>
#include "../common/imx9-dwc3.h"
#include "../common/imx9-scmi.h"
#include "../common/tq_bb.h"

DECLARE_GLOBAL_DATA_PTR;

int board_usb_init(int index, enum usb_init_type init)
{
	int ret = 0;

	if (is_usb_boot() && init != USB_INIT_DEVICE) {
		pr_warn("USB boot detected, USB host not available\n");
		return -ENODEV;
	}

	if (index == 0 && init == USB_INIT_DEVICE) {
		ret = imx9_scmi_power_domain_enable(IMX95_PD_HSIO_TOP, true);
		if (ret) {
			pr_err("SCMI_POWER_STATE_SET Failed for USB\n");
			return ret;
		}

		if (IS_ENABLED(CONFIG_USB_DWC3))
			return imx9_dwc3_device_init(index);
	}

	return 0;
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	if (index == 0 && init == USB_INIT_DEVICE) {
		if (IS_ENABLED(CONFIG_USB_DWC3))
			return imx9_dwc3_device_deinit(index);
	}

	return 0;
}

int board_early_init_f(void)
{
	/* UART7: A55, UART1: M33, UART<TBD>: M7 */
	/* UARTs are 0-indexed */
	init_uart_clk(6);

	return tq_bb_board_early_init_f();
}

#if !IS_ENABLED(CONFIG_XPL_BUILD)

static void netc_init(void)
{
	int ret;

	/* Power up the NETC MIX. */
	ret = imx9_scmi_power_domain_enable(IMX95_PD_NETC, true);
	if (ret) {
		pr_err("SCMI_POWER_STATE_SET Failed for NETC MIX\n");
		return;
	}

	set_clk_netc(ENET_125MHZ);

	pci_init();
}

int board_init(void)
{
	int ret;

	ret = imx9_scmi_power_domain_enable(IMX95_PD_HSIO_TOP, true);
	if (ret) {
		pr_err("SCMI_POWER_STATE_SET Failed for USB\n");
		return ret;
	}

	netc_init();

	return tq_bb_board_init();
}

int board_late_init(void)
{
	if (IS_ENABLED(CONFIG_AHAB_BOOT))
		env_set("sec_boot", "yes");
	else
		env_set("sec_boot", "no");

	return tq_bb_board_late_init();
}

#if IS_ENABLED(CONFIG_OF_BOARD_SETUP)

static const char * const usb2_device_paths[] = {
	"/soc/usb@4c200000",
	"/soc/usbmisc@4c200200",
	"/usbphynop"
};

int ft_board_setup(void *blob, struct bd_info *bd)
{
	size_t i;
	int off;

	if (IS_ENABLED(CONFIG_FDT_FIXUP_PARTITIONS)) {
		const char * const path = "/soc/bus@42000000/spi@425e0000";
		const struct node_info nodes[] = {
			{ "jedec,spi-nor",	MTD_DEV_TYPE_NOR, },
		};

		/*
		 * Update MTD partition nodes using info from
		 * mtdparts env var
		 * for [Q]SPI this needs the device probed.
		 */
		tq_ft_spi_setup(blob, path, nodes,
				ARRAY_SIZE(nodes));
	}

	if (is_usb_boot()) {
		printf("boot from USB - modify FDT\n");
		/* Chipidea is disconnected */
		for (i = 0; i < ARRAY_SIZE(usb2_device_paths); ++i) {
			off = fdt_path_offset(blob, usb2_device_paths[i]);
			if (off >= 0)
				fdt_set_node_status(blob, off, FDT_STATUS_DISABLED);
		}
		/* DWC3 is routed to X4 USB 2.0 A/B connector */
		off = fdt_path_offset(blob, "/soc/usb@4c010010/usb@4c100000");
		if (off >= 0) {
			fdt_setprop_string(blob, off, "dr_mode", "peripheral");
			fdt_setprop_string(blob, off, "maximum-speed", "high-speed");
		}
	}

	return tq_bb_ft_board_setup(blob, bd);
}
#endif

#if IS_ENABLED(CONFIG_OF_BOARD_FIXUP)
extern int board_fix_fdt_fuse(void *fdt);

int board_fix_fdt(void *fdt)
{
	/* Remove nodes based on fuses. */
	board_fix_fdt_fuse(fdt);

	return 0;
}
#endif

void board_quiesce_devices(void)
{
	int ret;

	ret = imx9_scmi_power_domain_enable(IMX95_PD_HSIO_TOP, false);
	if (ret) {
		pr_err("%s: Failed for HSIO MIX: %d\n", __func__, ret);
		return;
	}

	ret = imx9_scmi_power_domain_enable(IMX95_PD_NETC, false);
	if (ret) {
		pr_err("%s: Failed for NETC MIX: %d\n", __func__, ret);
		return;
	}

	tq_bb_board_quiesce_devices();
}

/**
 * Translate detected CPU variant into TQ-Systems SOM variant short name
 *
 * return: string consisting of the name or string indicating unknown variant
 */
const char *tq_get_boardname(void)
{
	static char *bname = "TQMa95xxSA";
	u32 cpurev = get_cpu_rev();
	const char *name = get_imx_type((cpurev & 0x1FF000) >> 12);

	if (name)
		sprintf(bname, "TQMa%sSA", name);

	return bname;
}

int checkboard(void)
{
	tq_print_bootinfo();
	printf("Board: %s on a %s\n", tq_get_boardname(),
	       tq_bb_get_boardname());

	return tq_bb_checkboard();
}

#endif /* !IS_ENABLED(CONFIG_XPL_BUILD) */
