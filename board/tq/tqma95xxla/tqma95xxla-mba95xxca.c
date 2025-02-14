// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <dwc3-uboot.h>
#include <env.h>
#include <fdt_support.h>
#include <init.h>
#include <usb.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch-imx9/ccm_regs.h>
#include "../common/imx9-dwc3.h"
#include "../common/imx9-scmi.h"
#include "../common/tq_bb.h"

#define BB_BOARD_NAME "MBa95xxCA"

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

int tq_bb_board_early_init_f(void)
{
	/* UART7: A55, UART1: M33, UART<TBD>: M7 */
	/* UARTs are 0-indexed */
	init_uart_clk(6);

	return 0;
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

int tq_bb_board_init(void)
{
	int ret;

	ret = imx9_scmi_power_domain_enable(IMX95_PD_HSIO_TOP, true);
	if (ret) {
		pr_err("SCMI_POWER_STATE_SET Failed for USB\n");
		return ret;
	}

	netc_init();

	return 0;
}

int tq_bb_board_late_init(void)
{
	if (IS_ENABLED(CONFIG_ENV_IS_IN_MMC))
		board_late_mmc_env_init();

	if (IS_ENABLED(CONFIG_AHAB_BOOT))
		env_set("sec_boot", "yes");
	else
		env_set("sec_boot", "no");

	return 0;
}

#if IS_ENABLED(CONFIG_OF_BOARD_SETUP)

static const char * const usb2_device_paths[] = {
	"/soc/usb@4c200000",
	"/soc/usbmisc@4c200200",
	"/usbphynop"
};

int tq_bb_ft_board_setup(void *blob, struct bd_info *bd)
{
	size_t i;
	int off;

	if (is_usb_boot()) {
		printf("boot from USB - modify FDT\n");
		/* Chipidea is disconnected */
		for (i = 0; i < ARRAY_SIZE(usb2_device_paths); ++i) {
			off = fdt_path_offset(blob, usb2_device_paths[i]);
			if (off >= 0)
				fdt_set_node_status(blob, off, FDT_STATUS_DISABLED);
		}
		/* DWC3 is routed to X9 USB 2.0 A/B connector */
		off = fdt_path_offset(blob, "/soc/usb@4c010010/usb@4c100000");
		if (off >= 0) {
			fdt_setprop_string(blob, off, "dr_mode", "peripheral");
			fdt_setprop_string(blob, off, "maximum-speed", "high-speed");
		}
	}

	return 0;
}
#endif

void tq_bb_board_quiesce_devices(void)
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
}

const char *tq_bb_get_boardname(void)
{
	return BB_BOARD_NAME;
}

#endif /* !IS_ENABLED(CONFIG_XPL_BUILD) */
