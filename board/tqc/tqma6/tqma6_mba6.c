/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 *
 * Copyright (C) 2013, 2014 - 2016 TQ Systems (ported SabreSD to TQMa6x)
 * Author: Markus Niebel <markus.niebel@tq-group.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/sys_proto.h>
#include <asm/errno.h>
#include <asm/gpio.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/imx-common/mxc_i2c.h>

#include <common.h>
#include <fdt_support.h>
#include <fsl_esdhc.h>
#include <i2c.h>
#include <libfdt.h>
#include <malloc.h>
#include <micrel.h>
#include <miiphy.h>
#include <mmc.h>
#include <netdev.h>
#include <usb.h>
#include <usb/ehci-fsl.h>

#include "../common/tqc_bb.h"
#include "../common/tqc_eeprom.h"
#include "tqma6_private.h"

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_CLK_PAD_CTRL (PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define GPIO_OUT_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_40ohm   | PAD_CTL_HYS)

#define GPIO_IN_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_40ohm   | PAD_CTL_HYS)

#define I2C_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
	PAD_CTL_DSE_80ohm | PAD_CTL_HYS |			\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#if defined(CONFIG_TQMA6Q) || defined(CONFIG_TQMA6Q_2GB) || \
	defined(CONFIG_TQMA6QP)

#define IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII	0x020e0790
#define IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM	0x020e07ac

#elif defined(CONFIG_TQMA6DL) || defined(CONFIG_TQMA6S)

#define IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII	0x020e0768
#define IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM	0x020e0788

#else

#error "need to select CPU"

#endif

#define ENET_RX_PAD_CTRL	(PAD_CTL_DSE_34ohm)
#define ENET_TX_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_DSE_34ohm)
#define ENET_CLK_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_HIGH | \
				 PAD_CTL_DSE_40ohm | PAD_CTL_HYS)
#define ENET_MDIO_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
				 PAD_CTL_DSE_60ohm)

/* disable on die termination for RGMII */
#define IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM_DISABLE	0x00000000
/* optimised drive strength for 1.0 .. 1.3 V signal on RGMII */
#define IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII_1P2V	0x00080000
/* optimised drive strength for 1.3 .. 2.5 V signal on RGMII */
#define IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII_1P5V	0x000C0000

#define ENET_PHY_RESET_GPIO IMX_GPIO_NR(1, 25)
#define ENET_PHY_INT_GPIO IMX_GPIO_NR(1, 28)

static iomux_v3_cfg_t const mba6_enet_pads[] = {
	NEW_PAD_CTRL(MX6_PAD_ENET_MDIO__ENET_MDIO,	ENET_MDIO_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET_MDC__ENET_MDC,	ENET_MDIO_PAD_CTRL),

	NEW_PAD_CTRL(MX6_PAD_RGMII_TXC__RGMII_TXC,	ENET_TX_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_TD0__RGMII_TD0,	ENET_TX_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_TD1__RGMII_TD1,	ENET_TX_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_TD2__RGMII_TD2,	ENET_TX_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_TD3__RGMII_TD3,	ENET_TX_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_TX_CTL__RGMII_TX_CTL,
		     ENET_TX_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET_REF_CLK__ENET_TX_CLK,	ENET_CLK_PAD_CTRL) & ~(MUX_MODE_SION),
	/*
	 * these pins are also used for config strapping by phy
	 */
	NEW_PAD_CTRL(MX6_PAD_RGMII_RD0__RGMII_RD0,	ENET_RX_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_RD1__RGMII_RD1,	ENET_RX_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_RD2__RGMII_RD2,	ENET_RX_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_RD3__RGMII_RD3,	ENET_RX_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_RXC__RGMII_RXC,	ENET_RX_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_RGMII_RX_CTL__RGMII_RX_CTL,
		     ENET_RX_PAD_CTRL),
	/* KSZ9031 PHY Reset */
	NEW_PAD_CTRL(MX6_PAD_ENET_CRS_DV__GPIO1_IO25,	GPIO_OUT_PAD_CTRL) |
		MUX_MODE_SION,
	/* FEC phy IRQ */
	NEW_PAD_CTRL(MX6_PAD_ENET_TX_EN__GPIO1_IO28,	GPIO_IN_PAD_CTRL),
};

static void mba6_setup_iomuxc_enet(void)
{
	struct iomuxc *const iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;

	/* clear gpr1[ENET_CLK_SEL] for external clock */
	clrbits_le32(&iomuxc_regs->gpr[1], IOMUXC_GPR1_ENET_CLK_SEL_MASK);
	if (is_mx6dqp()) {
		clrbits_le32(&iomuxc_regs->gpr[5], IOMUXC_GPR5_ENET_TXCLK_SEL_MASK);
	}

	__raw_writel(IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM_DISABLE,
		     (void *)IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM);
	__raw_writel(IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII_1P5V,
		     (void *)IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII);

	imx_iomux_v3_setup_multiple_pads(mba6_enet_pads,
					 ARRAY_SIZE(mba6_enet_pads));

	gpio_request(ENET_PHY_RESET_GPIO, "phy-rst#");
	gpio_request(ENET_PHY_INT_GPIO, "phy-int");
	gpio_direction_input(ENET_PHY_INT_GPIO);
	/* Reset PHY */
	gpio_direction_output(ENET_PHY_RESET_GPIO , 0);
	/* Need delay 10ms after power on according to KSZ9031 spec */
	mdelay(10);
	gpio_set_value(ENET_PHY_RESET_GPIO, 1);
	/*
	 * KSZ9031 manual: 100 usec wait time after reset before communication
	 * over MDIO
	 * BUGBUG: hardware has an RC const that needs > 10 msec from 0->1 on
	 * reset before the phy sees a high level
	 */
	mdelay(15);
}

static iomux_v3_cfg_t const mba6_uart2_pads[] = {
	NEW_PAD_CTRL(MX6_PAD_SD4_DAT4__UART2_RX_DATA, UART_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD4_DAT7__UART2_TX_DATA, UART_PAD_CTRL),
};

static void mba6_setup_iomuxc_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(mba6_uart2_pads,
					 ARRAY_SIZE(mba6_uart2_pads));
}

#define USDHC2_CD_GPIO	IMX_GPIO_NR(1, 4)
#define USDHC2_WP_GPIO	IMX_GPIO_NR(1, 2)

int tqc_bb_board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	if (cfg->esdhc_base == USDHC2_BASE_ADDR)
		ret = !gpio_get_value(USDHC2_CD_GPIO);

	return ret;
}

int tqc_bb_board_mmc_getwp(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	if (cfg->esdhc_base == USDHC2_BASE_ADDR)
		ret = gpio_get_value(USDHC2_WP_GPIO);

	return ret;
}

static struct fsl_esdhc_cfg mba6_usdhc_cfg = {
	.esdhc_base = USDHC2_BASE_ADDR,
	.max_bus_width = 4,
};

static iomux_v3_cfg_t const mba6_usdhc2_pads[] = {
	NEW_PAD_CTRL(MX6_PAD_SD2_CLK__SD2_CLK,		USDHC_CLK_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD2_CMD__SD2_CMD,		USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD2_DAT0__SD2_DATA0,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD2_DAT1__SD2_DATA1,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD2_DAT2__SD2_DATA2,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD2_DAT3__SD2_DATA3,	USDHC_PAD_CTRL),
	/* CD */
	NEW_PAD_CTRL(MX6_PAD_GPIO_4__GPIO1_IO04,	GPIO_IN_PAD_CTRL),
	/* WP */
	NEW_PAD_CTRL(MX6_PAD_GPIO_2__GPIO1_IO02,	GPIO_IN_PAD_CTRL),
};

int tqc_bb_board_mmc_init(bd_t *bis)
{
	imx_iomux_v3_setup_multiple_pads(mba6_usdhc2_pads,
					 ARRAY_SIZE(mba6_usdhc2_pads));
	gpio_request(USDHC2_CD_GPIO, "usdhc2-cd");
	gpio_request(USDHC2_WP_GPIO, "usdhc2-wp");
	gpio_direction_input(USDHC2_CD_GPIO);
	gpio_direction_input(USDHC2_WP_GPIO);

	mba6_usdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
	if (fsl_esdhc_initialize(bis, &mba6_usdhc_cfg))
		puts("Warning: failed to initialize SD\n");

	return 0;
}

static struct i2c_pads_info mba6_i2c1_pads = {
/* I2C1: MBa6x */
	.scl = {
		.i2c_mode = NEW_PAD_CTRL(MX6_PAD_CSI0_DAT9__I2C1_SCL,
					 I2C_PAD_CTRL),
		.gpio_mode = NEW_PAD_CTRL(MX6_PAD_CSI0_DAT9__GPIO5_IO27,
					  I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(5, 27)
	},
	.sda = {
		.i2c_mode = NEW_PAD_CTRL(MX6_PAD_CSI0_DAT8__I2C1_SDA,
					 I2C_PAD_CTRL),
		.gpio_mode = NEW_PAD_CTRL(MX6_PAD_CSI0_DAT8__GPIO5_IO26,
					  I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(5, 26)
	}
};

static void mba6_setup_i2c(void)
{
	int ret;

	if (tqma6_get_system_i2c_bus() == 0)
		return;
	/*
	 * use logical index for bus, e.g. I2C1 -> 0
	 * warn on error
	 */
	ret = setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &mba6_i2c1_pads);
	if (ret)
		printf("setup I2C1 failed: %d\n", ret);
}

int board_phy_config(struct phy_device *phydev)
{
/*
 * optimized pad skew values depends on CPU variant on the TQMa6x module:
 * CONFIG_TQMA6Q: i.MX6Q/D
 * CONFIG_TQMA6S: i.MX6S
 * CONFIG_TQMA6DL: i.MX6DL
 */
#if defined(CONFIG_TQMA6Q) || defined(CONFIG_TQMA6Q_2GB) || \
	defined(CONFIG_TQMA6QP)
#define MBA6X_KSZ9031_CTRL_SKEW	0x0032
#define MBA6X_KSZ9031_CLK_SKEW	0x03ff
#define MBA6X_KSZ9031_RX_SKEW	0x3333
#define MBA6X_KSZ9031_TX_SKEW	0x2036
#elif defined(CONFIG_TQMA6S) || defined(CONFIG_TQMA6DL)
#define MBA6X_KSZ9031_CTRL_SKEW	0x0030
#define MBA6X_KSZ9031_CLK_SKEW	0x03ff
#define MBA6X_KSZ9031_RX_SKEW	0x3333
#define MBA6X_KSZ9031_TX_SKEW	0x2052
#else
#error
#endif
	/* min rx/tx ctrl delay */
	ksz9031_phy_extended_write(phydev, 2,
				   MII_KSZ9031_EXT_RGMII_CTRL_SIG_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC,
				   MBA6X_KSZ9031_CTRL_SKEW);
	/* min rx delay */
	ksz9031_phy_extended_write(phydev, 2,
				   MII_KSZ9031_EXT_RGMII_RX_DATA_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC,
				   MBA6X_KSZ9031_RX_SKEW);
	/* max tx delay */
	ksz9031_phy_extended_write(phydev, 2,
				   MII_KSZ9031_EXT_RGMII_TX_DATA_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC,
				   MBA6X_KSZ9031_TX_SKEW);
	/* rx/tx clk skew */
	ksz9031_phy_extended_write(phydev, 2,
				   MII_KSZ9031_EXT_RGMII_CLOCK_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC,
				   MBA6X_KSZ9031_CLK_SKEW);

	phydev->drv->config(phydev);

	return 0;
}

int board_eth_init(bd_t *bis)
{
	uint32_t base = IMX_FEC_BASE;
	struct mii_dev *bus = NULL;
	struct phy_device *phydev = NULL;
	int ret;

	bus = fec_get_miibus(base, -1);
	if (!bus)
		return -EINVAL;
	/* scan phy */
	phydev = phy_find_by_mask(bus, (0xf << CONFIG_FEC_MXC_PHYADDR),
					PHY_INTERFACE_MODE_RGMII);

	if (!phydev) {
		ret = -EINVAL;
		goto free_bus;
	}
	ret  = fec_probe(bis, -1, base, bus, phydev);
	if (ret)
		goto free_phydev;

	return 0;

free_phydev:
	free(phydev);
free_bus:
	free(bus);
	return ret;
}

int board_get_rtc_bus(void)
{
	return tqma6_get_system_i2c_bus();
}

int board_get_dtt_bus(void)
{
	return tqma6_get_system_i2c_bus();
}

#define MBA6_USB_PAD_CTRL	(PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW | \
				 PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST | \
				 PAD_CTL_HYS)

iomux_v3_cfg_t const mba6_usb_otg_pads[] = {
	NEW_PAD_CTRL(MX6_PAD_EIM_D21__USB_OTG_OC, MBA6_USB_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_GPIO_1__USB_OTG_ID, MBA6_USB_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_D22__GPIO3_IO22, MBA6_USB_PAD_CTRL),
};

/*
 * use gpio instead of PWR as log as he ehci driver does not support
 * board specific polarity
 */
#define MBA6_OTG_PWR_GPIO IMX_GPIO_NR(3, 22)

static void mba6_setup_iomux_usb(void)
{
	int ret;

	imx_iomux_v3_setup_multiple_pads(mba6_usb_otg_pads,
					 ARRAY_SIZE(mba6_usb_otg_pads));
	ret = gpio_request(MBA6_OTG_PWR_GPIO, "usb-otg1-pwr");
	if (!ret)
		gpio_direction_output(MBA6_OTG_PWR_GPIO, 0);
}

int board_ehci_hcd_init(int port)
{
	int ret = -ENODEV;

	switch (port) {
	case 0:
	case 1:
		ret = 0;
		break;
	case 2:
	case 3:
		printf("MXC USB port %d not yet supported\n", port);
		break;
	default:
		break;
	}

	return ret;
}

int board_ehci_power(int port, int on)
{
	int ret = -ENODEV;

	switch (port) {
	case 0:
		printf("MXC USB port %d power %d\n", port, on);
		gpio_set_value(MBA6_OTG_PWR_GPIO, on);
		ret = 0;
		break;
	case 1:
		ret = 0;
		break;
	case 2:
	case 3:
		printf("MXC USB port %d not yet supported\n", port);
		break;
	default:
		break;
	}

	return ret;
}

int board_usb_phy_mode(int port)
{
	int ret = -EINVAL;

	/* port index start at 0 */
	switch (port) {
	case 1:
		ret = USB_INIT_HOST;
		break;
	case 0:
		ret = usb_phy_mode(port);
		printf("MXC USB port %d: mode %d\n", port, ret);
		break;
	}

	return ret;
}

int tqc_bb_board_early_init_f(void)
{
	mba6_setup_iomuxc_uart();

	return 0;
}

int tqc_bb_board_init(void)
{
	mba6_setup_i2c();
	/* do it here - to have reset completed */
	mba6_setup_iomuxc_enet();

	mba6_setup_iomux_usb();

	return 0;
}

int tqc_bb_board_late_init(void)
{
	int ret;
	struct tqc_eeprom_data eedat;
	char mac[20];
	enum boot_device bd;

	ret = tqc_read_eeprom(tqma6_get_system_i2c_bus(), 0x57, &eedat);
	if (!ret) {
		tqc_parse_eeprom_mac(&eedat, mac, ARRAY_SIZE(mac));
		setenv("usbethaddr", mac);
		tqc_show_eeprom(&eedat, "MBA6");
	} else {
		printf("%s EEPROM: err %d, bus %d\n", tqc_bb_get_boardname(), ret, tqma6_get_system_i2c_bus());
	}

	/*
	* try to get sd card slots in order:
	* eMMC: on Module
	* -> therefore index 0 for bootloader
	* index n in kernel (controller instance 3) -> patches needed for
	* alias indexing
	* SD2: on Mainboard
	* index n in kernel (controller instance 2) -> patches needed for
	* alias indexing
	* we assume to have a kernel patch that will present mmcblk dev
	* indexed like controller devs
	*/
	puts("Boot:\t");
	bd = get_boot_device();
	switch (bd) {
	case MMC3_BOOT:
		setenv("boot_dev", "mmc");
		puts("USDHC3 (eMMC)\n");
		setenv("mmcblkdev", "0");
		setenv("mmcdev", "0");
		break;
	case SD2_BOOT:
		setenv("boot_dev", "mmc");
		puts("USDHC2 (SD)\n");
		setenv("mmcblkdev", "1");
		setenv("mmcdev", "1");
		break;
	case SPI_NOR_BOOT:
		puts("SPI NOR\n");
		setenv("boot_dev", "spi");
		/* mmcdev is dev index for u-boot */
		setenv("mmcdev", "0");
		/* mmcblkdev is dev index for linux env */
		setenv("mmcblkdev", "0");
		break;
	default:
		printf("unhandled boot device %d\n",(int)bd);
		setenv("mmcblkdev", "");
		setenv("mmcdev", "");
	}

	/* provide default setting for fdt_file if nothing in env is set */
	if (NULL == getenv("fdt_file")) {
		int enw = (tqma6_get_enet_workaround() > 0);
		u32 cpurev = get_cpu_rev();

		switch ((cpurev & 0xFF000) >> 12) {
		case MXC_CPU_MX6SOLO:
		case MXC_CPU_MX6DL:
			setenv("fdt_file",
				(enw) ? "imx6dl-mba6a.dtb" : "imx6dl-mba6b.dtb");
			break;
		case MXC_CPU_MX6D:
		case MXC_CPU_MX6Q:
			setenv("fdt_file",
				(enw) ? "imx6q-mba6a.dtb" : "imx6q-mba6b.dtb");
			break;
		case MXC_CPU_MX6DP:
		case MXC_CPU_MX6QP:
			setenv("fdt_file", "imx6qp-mba6b.dtb");
			break;
		default:
			debug("unknown CPU");
		}
	}

	return 0;
}

const char *tqc_bb_get_boardname(void)
{
	return "MBa6x";
}

int board_mmc_get_env_dev(int devno)
{
	/*
	 * eMMC:	USDHC3 -> 0
	 * SD:		USDHC1 -> 1
	 */
	return (2 == devno) ? 0 : 1;
}

/*
 * Device Tree Support
 */
#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
void tqc_bb_ft_board_setup(void *blob, bd_t *bd)
{
	int offset, len, chk;
	const char *compatible, *expect;

	if (tqma6_get_enet_workaround())
		expect = "tq,mba6a";
	else
		expect = "tq,mba6b";

	offset = fdt_path_offset(blob, "/");
	compatible = fdt_getprop(blob, offset, "compatible", &len);
	if (len > 0)
		printf("   Device Tree: /compatible = %s\n", compatible);

	chk = fdt_node_check_compatible(blob, offset, expect);
	switch (chk) {
		case 0:
			break;
		case 1:
			printf("   WARNING! Wrong DT variant: "
						"Expecting %s!\n", expect);
			break;
		default:
			printf("   Device Tree: cannot read /compatible"
						"to identify tree variant!\n");
	}

}
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */
