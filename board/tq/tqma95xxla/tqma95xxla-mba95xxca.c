// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <dwc3-uboot.h>
#include <env.h>
#include <fdt_support.h>
#include <i2c.h>
#include <init.h>
#include <scmi_agent.h>
#include <usb.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch-imx9/ccm_regs.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <power/regulator.h>

#include "../common/imx9-dwc3.h"
#include "../common/imx9-scmi.h"
#include "../common/tcpc.h"
#include "../common/tq_bb.h"

#define BB_BOARD_NAME "MBa95xxCA"

#if IS_ENABLED(CONFIG_USB_TCPC)

static struct tcpc_port typec_port;

static struct tcpc_port_config port_config = {
	.i2c_bus = 1, /* i2c2 */
	.addr = 0x50,
	.port_type = TYPEC_PORT_UFP,
	.disable_pd = true,
};

/*
 * USB3 / DWC3 port handling. The USB 2.0 lane of this IP
 * is muxed to the TYPE-C port controller if the board is
 * configured for serial download mode. In this case the
 * IP should be limitied to device mode with HS speed.
 *
 * USB3_PHY_TCA register base address
 */
static ulong tca_base;

void tca_mux_select(enum typec_cc_polarity pol)
{
	u32 val;

	if (!tca_base)
		return;

	/* reset XBar block */
	setbits_le32(tca_base, BIT(9));

	/* Set OP mode to System configure Mode */
	clrbits_le32(tca_base + 0x10, 0x3);
	/* read TCA_PSTATE */
	val = readl(tca_base + 0x30);
	/* PIPE0_POWERDOWN[1:0] status/value from the PIPE */
	WARN_ON((val & GENMASK(1, 0)) != 0x3);
	/* RX_PLL_STATE */
	WARN_ON((val & BIT(2)) != 0);
	/* TX_STATE */
	WARN_ON((val & BIT(3)) != 0);
	/* TX_CM_STATE */
	WARN_ON((val & BIT(4)) != 0);
	/* TCA_SYSMODE_CFG: set TYPEC_DISABLE */
	setbits_le32(tca_base + 0x18, BIT(3));
	udelay(1);
	/* TCA_SYSMODE_CFG: set TYPEC_FLIP based on polarity */
	if (pol == TYPEC_POLARITY_CC1)
		clrbits_le32(tca_base + 0x18, BIT(2));
	else
		setbits_le32(tca_base + 0x18, BIT(2));

	udelay(1);
	/* TCA_SYSMODE_CFG: clear TYPEC_DISABLE, stanadard operation */
	clrbits_le32(tca_base + 0x18, BIT(3));
}

static void setup_typec(void)
{
	int ret;

	if (is_usb_boot()) {
		/* use USB 3.0 TCA register block and add handler for mux select */
		tca_base = USB1_BASE_ADDR + 0xfc000;

		ret = tcpc_init(&typec_port, port_config, &tca_mux_select);
	} else {
		/* no special PHY handling for USB 2.0 phy */
		ret = tcpc_init(&typec_port, port_config, NULL);
	}
	if (ret) {
		printf("%s: tcpc init failed, err=%d\n", __func__, ret);
		return;
	}
}

int board_ehci_usb_phy_mode(struct udevice *dev)
{
	enum typec_cc_polarity pol;
	enum typec_cc_state state;
	struct tcpc_port *port_ptr;
	int ret = 0;

	/*
	 * dev_seq == 0: USB3 / DWC3 (should never happen)
	 * dev_seq == 1: USB2
	 */
	if (dev_seq(dev) == 0)
		return USB_INIT_DEVICE;

	tcpc_setup_ufp_mode(&typec_port);
	ret = tcpc_get_cc_status(&typec_port, &pol, &state);

	tcpc_print_log(&typec_port);
	if (!ret) {
		if (state == TYPEC_STATE_SRC_RD_RA || state == TYPEC_STATE_SRC_RD)
			return USB_INIT_HOST;
	} else {
		printf("ERROR: getting TypeC CC status %d\n", ret);
	}

	return USB_INIT_DEVICE;
}

static int mba95xxca_typec_init(enum usb_init_type init)
{
	int ret;

	switch (init) {
	case USB_INIT_DEVICE:
		ret = tcpc_setup_ufp_mode(&typec_port);
		break;
	case USB_INIT_HOST:
		ret = tcpc_setup_dfp_mode(&typec_port);
		break;
	default:
		pr_info("TYPE-C: unsupported init type\n");
		ret = -EINVAL;
	}

	return ret;
}

static int mba95xxca_typec_deinit(enum usb_init_type init)
{
	int ret = 0;

	if (init == USB_INIT_HOST)
		ret = tcpc_disable_src_vbus(&typec_port);

	return ret;
}

#endif

static void regulator_switch_by_name(const char *devname, bool enable)
{
	struct udevice *dev;
	int ret;

	ret = regulator_get_by_devname(devname, &dev);
	if (ret) {
		pr_warn("%s regulator not found: %d\n", devname, ret);
		return;
	}

	ret = regulator_set_enable_if_allowed(dev, enable);
	if (ret) {
		pr_info("%s %s regulator failed: %d\n",
			devname, enable ? "Enable" : "Disable", ret);
		return;
	}
}

int board_usb_init(int index, enum usb_init_type init)
{
	/* USB2 / chipidea */
	if (index == 1 && !is_usb_boot())
		return mba95xxca_typec_init(init);

	if (index == 0) {
		if (is_usb_boot() && init != USB_INIT_DEVICE) {
			pr_warn("USB boot detected, USB host not available\n");
			return -ENODEV;
		}

		if (IS_ENABLED(CONFIG_USB_DWC3)) {
			bool init_host = (init == USB_INIT_HOST);
			int ret;

			ret = imx9_scmi_power_domain_enable(IMX95_PD_HSIO_TOP, true);
			if (ret) {
				pr_err("SCMI_POWER_STATE_SET Failed for USB\n");
				return ret;
			}

			if (init_host)
				regulator_switch_by_name("regulator-vbus-usb3", true);
			return imx9_dwc3_device_init(index, init_host);
		}
	}

	/* invalid port */
	pr_err("USB%d not available\n", index);
	return -ENODEV;
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	/* USB2 / chipidea */
	if (index == 1 && !is_usb_boot())
		return mba95xxca_typec_deinit(init);
	if (index == 0) {
		if (IS_ENABLED(CONFIG_USB_DWC3)) {
			if (init == USB_INIT_HOST)
				regulator_switch_by_name("regulator-vbus-usb3", false);
			return imx9_dwc3_device_deinit(index);
		}
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

	if (IS_ENABLED(CONFIG_USB_TCPC))
		setup_typec();

	netc_init();

	return 0;
}

int tq_bb_board_late_init(void)
{
	if (IS_ENABLED(CONFIG_ENV_IS_IN_MMC))
		board_late_mmc_env_init();

	tq_set_boot_targets();

	return 0;
}

static const char * const usb2_device_paths[] = {
	"/soc/usb@4c200000",
	"/soc/usbmisc@4c200200",
	"/usbphynop"
};

static const char * const usb3_device_path = "/soc/usb@4c010010/usb@4c100000";

static void tqma95_fdt_fixup_usb(void *blob)
{
	int off;
	int rc = 0;

	do {
		/*
		 * centralized error handling and loop termination before changing
		 * the fdt:
		 * - if no error the loop is executed once
		 * - if a fixup step signals FDT_ERR_NOSPACE, we try to allocate
		 *   and apply the fixups again
		 * - teminate loop on all other errors
		 */
		if (rc == -FDT_ERR_NOSPACE)
			rc = fdt_increase_size(blob, 512);

		if (rc) {
			pr_err("ERROR: unable fixup DTB for USB\n");
			break;
		}

		if (is_usb_boot()) {
			/* Chipidea USB 2.0 is disconnected */
			for (size_t i = 0; i < ARRAY_SIZE(usb2_device_paths); ++i) {
				off = fdt_path_offset(blob, usb2_device_paths[i]);
				if (off >= 0) {
					rc = fdt_set_node_status(blob, off,
								 FDT_STATUS_DISABLED);
					if (rc)
						continue;
				}
			}

			off = fdt_path_offset(blob, usb3_device_path);
			/* DWC3 connected as USB 2.0 device to X9 */
			rc = fdt_setprop_string(blob, off,
						"dr_mode", "peripheral");
			if (rc)
				continue;
			rc = fdt_setprop_string(blob, off,
						"maximum-speed", "high-speed");
			if (rc)
				continue;
		} else {
			off = fdt_path_offset(blob, usb3_device_path);
			/* DWC3 connected as USB 3 host to hub */
			rc = fdt_setprop_string(blob, off,
						"dr_mode", "host");
			if (rc)
				continue;
		}
	} while (rc != 0);

	pr_info("Fixup DTB for USB2/3\n");
}

#if IS_ENABLED(CONFIG_OF_BOARD_SETUP)

int tq_bb_ft_board_setup(void *blob, struct bd_info *bd)
{
	tqma95_fdt_fixup_usb(blob);

	return 0;
}
#endif

#if IS_ENABLED(CONFIG_OF_BOARD_FIXUP)
int tq_bb_board_fix_fdt(void *fdt)
{
	tqma95_fdt_fixup_usb(fdt);

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
