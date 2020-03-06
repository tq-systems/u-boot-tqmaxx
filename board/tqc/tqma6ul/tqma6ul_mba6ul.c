/*
 * Copyright (C) 2016-2018 TQ Systems GmbH
 * Author: Marco Felsch <Marco.Felsch@tq-group.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/errno.h>
#include <asm/gpio.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/mxc_i2c.h>
#include <asm/imx-common/spi.h>
#include <asm/io.h>
#include <common.h>
#include <fsl_esdhc.h>
#include <libfdt.h>
#include <linux/sizes.h>
#include <malloc.h>
#include <i2c.h>
#include <miiphy.h>
#include <mmc.h>
#include <netdev.h>
#include <pca953x.h>
#include <spl.h>
#include <usb.h>
#include <usb/ehci-fsl.h>

#include "../common/tqc_bb.h"
#include "../common/tqc_eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |			\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |			\
	PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP | PAD_CTL_SPEED_LOW |	\
	PAD_CTL_DSE_80ohm | PAD_CTL_SRE_FAST | PAD_CTL_HYS)

#define USDHC_CLK_PAD_CTRL (PAD_CTL_PUS_47K_UP | PAD_CTL_SPEED_LOW |	\
	PAD_CTL_DSE_48ohm | PAD_CTL_SRE_FAST | PAD_CTL_HYS)

#define GPIO_IN_PAD_CTRL (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_LOW |	\
	PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define GPIO_OUT_PAD_CTRL (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_LOW |	\
	PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define I2C_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |	\
	PAD_CTL_DSE_80ohm | PAD_CTL_HYS | PAD_CTL_ODE | 		\
	PAD_CTL_SRE_FAST)

#define ENET_RX_PAD_CTRL	(PAD_CTL_DSE_34ohm)
#define ENET_TX_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_DSE_60ohm)
#define ENET_CLK_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_HIGH | \
				 PAD_CTL_DSE_40ohm | PAD_CTL_HYS)
#define ENET_MDIO_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
				 PAD_CTL_DSE_60ohm)
#define ENET_PAD_CTRL (PAD_CTL_PUS_100K_UP | PAD_CTL_PUE |		\
	PAD_CTL_SPEED_HIGH | PAD_CTL_DSE_48ohm | PAD_CTL_SRE_FAST)

#define USB_ID_PAD_CTRL (PAD_CTL_PUS_100K_DOWN | PAD_CTL_SPEED_MED |	\
	PAD_CTL_DSE_120ohm | PAD_CTL_SRE_FAST | PAD_CTL_HYS)

#define USB_OC_PAD_CTRL (PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |	\
	PAD_CTL_HYS | PAD_CTL_PKE)

#define WDOG_PAD_CTRL		(PAD_CTL_PUS_22K_UP | PAD_CTL_PUE | \
	PAD_CTL_DSE_40ohm)

/*
 * pin conflicts for fec1 and fec2, GPIO1_IO06 and GPIO1_IO07 can only
 * be used for ENET1 or ENET2, cannot be used for both.
 */
static iomux_v3_cfg_t const mba6ul_fec1_pads[] = {
	NEW_PAD_CTRL(MX6_PAD_ENET1_RX_DATA0__ENET1_RDATA00, ENET_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET1_RX_DATA1__ENET1_RDATA01, ENET_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET1_RX_EN__ENET1_RX_EN, ENET_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET1_RX_ER__ENET1_RX_ER, ENET_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET1_TX_DATA0__ENET1_TDATA00, ENET_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET1_TX_DATA1__ENET1_TDATA01, ENET_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET1_TX_EN__ENET1_TX_EN, ENET_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET1_TX_CLK__ENET1_REF_CLK1, ENET_CLK_PAD_CTRL),

	/* MDIO */
	/* pins are shared with fec2 */
};

static iomux_v3_cfg_t const mba6ul_fec2_pads[] = {
	NEW_PAD_CTRL(MX6_PAD_ENET2_RX_DATA0__ENET2_RDATA00, ENET_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET2_RX_DATA1__ENET2_RDATA01, ENET_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET2_RX_EN__ENET2_RX_EN, ENET_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET2_RX_ER__ENET2_RX_ER, ENET_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET2_TX_DATA0__ENET2_TDATA00, ENET_TX_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET2_TX_DATA1__ENET2_TDATA01, ENET_TX_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET2_TX_EN__ENET2_TX_EN, ENET_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET2_TX_CLK__ENET2_REF_CLK2, ENET_CLK_PAD_CTRL),

	/* MDIO */
	/* pins are shared with fec1 */
};

static iomux_v3_cfg_t const mba6ul_fec_common_pads[] = {
	/* MDIO */
	NEW_PAD_CTRL(MX6_PAD_GPIO1_IO06__ENET1_MDIO, ENET_MDIO_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_GPIO1_IO07__ENET1_MDC, ENET_MDIO_PAD_CTRL),
};

static void mba6ul_setup_iomuxc_enet(void)
{
	int old_bus;

	imx_iomux_v3_setup_multiple_pads(mba6ul_fec_common_pads,
					 ARRAY_SIZE(mba6ul_fec_common_pads));
	imx_iomux_v3_setup_multiple_pads(mba6ul_fec1_pads,
					 ARRAY_SIZE(mba6ul_fec1_pads));
	imx_iomux_v3_setup_multiple_pads(mba6ul_fec2_pads,
					 ARRAY_SIZE(mba6ul_fec2_pads));

	old_bus = i2c_get_bus_num();
	i2c_set_bus_num(3);

	pca953x_set_val(0x22, 0x06, 0x00);
	pca953x_set_dir(0x22, 0x06, 0x00);
	/* lan8720: 5.5.3 reset time should be 25ms */
	udelay(30000);
	pca953x_set_val(0x22, 0x06, 0x06);

	i2c_set_bus_num(old_bus);
}

int board_eth_init(bd_t *bis)
{
	int ret = 0;

	/*
	 * FEC0 and FEC1 shares the mdio bus. Therefore the
	 * CONFIG_FEC_MXC_MDIO_BASE macro is used to specify the bus.
	 * This makes the ENET_BASE_ADDR and ENET2_BASE_ADDR macro
	 * useless for the fecmxc_initialize_multi function.
	 * also note, that dev_id param is used internally to pick up the
	 * correct fused MAC address - renaming can be done under linux
	 * using udev / systemd
	 */
	if (check_module_fused(MX6_MODULE_ENET1)) {
		puts("FEC0: fused\n");
	} else {
		ret = fecmxc_initialize_multi(bis, 0, TQMA6UL_ENET1_PHYADDR,
					      ENET_BASE_ADDR);
		if (ret)
			printf("FEC0 MXC: %s:failed %i\n", __func__, ret);
	}
	if (check_module_fused(MX6_MODULE_ENET2)) {
		puts("FEC1: fused\n");
	} else {
		ret = fecmxc_initialize_multi(bis, 1, TQMA6UL_ENET2_PHYADDR,
					      ENET2_BASE_ADDR);
		if (ret)
			printf("FEC1 MXC: %s:failed %i\n", __func__, ret);
	}

	return ret;
}

enum fec_device {
	FEC0,
	FEC1
};

static int mba6ul_setup_fec(int fec_id)
{
	struct iomuxc_gpr_base_regs *const iomuxc_gpr_regs
		= (struct iomuxc_gpr_base_regs *) IOMUXC_GPR_BASE_ADDR;
	int ret;

	switch (fec_id) {
	case FEC0:
		if (check_module_fused(MX6_MODULE_ENET1)) {
			puts("FEC0: fused\n");
			return -1;
		}
		/*
		 * Use 50M anatop loopback REF_CLK1 for ENET1,
		 * clear gpr1[13], set gpr1[17]
		 */
		clrsetbits_le32(&iomuxc_gpr_regs->gpr[1], IOMUX_GPR1_FEC1_MASK,
				IOMUX_GPR1_FEC1_CLOCK_MUX1_SEL_MASK);
		break;
	case FEC1:
		if (check_module_fused(MX6_MODULE_ENET2)) {
			puts("FEC1: fused\n");
			return -1;
		}
		/*
		 * Use 50M anatop loopback REF_CLK1 for ENET2,
		 * clear gpr1[14], set gpr1[18]
		 */
		clrsetbits_le32(&iomuxc_gpr_regs->gpr[1], IOMUX_GPR1_FEC2_MASK,
				IOMUX_GPR1_FEC2_CLOCK_MUX1_SEL_MASK);
		break;
	default:
		printf("FEC%d: unsupported\n", fec_id);
		return -1;
	}

	ret = enable_fec_anatop_clock(fec_id, ENET_50MHZ);
	if (ret)
		return ret;
	enable_enet_clk(1);

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);
	return 0;
}


static iomux_v3_cfg_t const mba6ul_uart1_pads[] = {
	NEW_PAD_CTRL(MX6_PAD_UART1_TX_DATA__UART1_DCE_TX, UART_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_UART1_RX_DATA__UART1_DCE_RX, UART_PAD_CTRL),
};

static void mba6ul_setup_iomuxc_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(mba6ul_uart1_pads,
					 ARRAY_SIZE(mba6ul_uart1_pads));
}

static iomux_v3_cfg_t const mba6ul_usdhc1_pads[] = {
	NEW_PAD_CTRL(MX6_PAD_SD1_CLK__USDHC1_CLK,	USDHC_CLK_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD1_CMD__USDHC1_CMD,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD1_DATA0__USDHC1_DATA0,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD1_DATA1__USDHC1_DATA1,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD1_DATA2__USDHC1_DATA2,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD1_DATA3__USDHC1_DATA3,	USDHC_PAD_CTRL),

	/* VSELECT not usable in standard variant, signal is used as USB OTG ID */
	/* MX6_PAD_GPIO1_IO05__USDHC1_VSELECT | MUX_PAD_CTRL(USDHC_PAD_CTRL), */
	/* CD */
	NEW_PAD_CTRL(MX6_PAD_UART1_RTS_B__GPIO1_IO19,	GPIO_IN_PAD_CTRL),
	/* WP */
	/* TODO: should we mux it as MX6_PAD_UART1_CTS_B__USDHC1_WP? */
	NEW_PAD_CTRL(MX6_PAD_UART1_CTS_B__GPIO1_IO18,	GPIO_IN_PAD_CTRL),
};

#define USDHC1_CD_GPIO	IMX_GPIO_NR(1, 19)
#define USDHC1_WP_GPIO	IMX_GPIO_NR(1, 18)

int tqc_bb_board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	if (cfg->esdhc_base == USDHC1_BASE_ADDR)
		ret = !gpio_get_value(USDHC1_CD_GPIO);

	return ret;
}

int tqc_bb_board_mmc_getwp(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	if (cfg->esdhc_base == USDHC1_BASE_ADDR)
		ret = gpio_get_value(USDHC1_WP_GPIO);

	return ret;
}

static struct fsl_esdhc_cfg mba6ul_usdhc_cfg = {
	.esdhc_base = USDHC1_BASE_ADDR,
	.max_bus_width = 4,
};

int tqc_bb_board_mmc_init(bd_t *bis)
{
	imx_iomux_v3_setup_multiple_pads(mba6ul_usdhc1_pads,
					 ARRAY_SIZE(mba6ul_usdhc1_pads));
	gpio_request(USDHC1_CD_GPIO, "usdhc1-cd");
	gpio_request(USDHC1_WP_GPIO, "usdhc1-wp");
	gpio_direction_input(USDHC1_CD_GPIO);
	gpio_direction_input(USDHC1_WP_GPIO);

	mba6ul_usdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
	if (fsl_esdhc_initialize(bis, &mba6ul_usdhc_cfg))
		puts("Warning: failed to initialize SD\n");

	return 0;
}

#ifdef CONFIG_USB_EHCI_MX6
#define USB_OTHERREGS_OFFSET	0x800
#define UCTRL_PWR_POL		BIT(9)
#define UCTRL_OC_POL		BIT(8)
#define UCTRL_OC_DISABLE	BIT(7)

static iomux_v3_cfg_t const usb_otg1_pads[] = {
	NEW_PAD_CTRL(MX6_PAD_GPIO1_IO01__USB_OTG1_OC,	USB_OC_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_GPIO1_IO00__ANATOP_OTG1_ID, USB_ID_PAD_CTRL),
	/* OTG1_PWR */
	NEW_PAD_CTRL(MX6_PAD_GPIO1_IO04__GPIO1_IO04,	GPIO_OUT_PAD_CTRL),
};
#define USB_OTG1_PWR	IMX_GPIO_NR(1, 4)

static void mba6ul_setup_usb(void)
{
	imx_iomux_v3_setup_multiple_pads(usb_otg1_pads,
					 ARRAY_SIZE(usb_otg1_pads));
}

int board_usb_phy_mode(int port)
{
	/* port index start at 0 */
	if (port > (CONFIG_USB_MAX_CONTROLLER_COUNT - 1) || port < 0)
		return -EINVAL;

	if (1 == port)
		return USB_INIT_HOST;
	else
		return usb_phy_mode(port);
}

int board_ehci_hcd_init(int port)
{
	u32 *usbnc_usb_ctrl;

	/* port index start at 0 */
	if (port > (CONFIG_USB_MAX_CONTROLLER_COUNT - 1) || port < 0)
		return -EINVAL;

	usbnc_usb_ctrl = (u32 *)(USB_BASE_ADDR + USB_OTHERREGS_OFFSET +
				 port * 4);

	switch (port) {
		case 0:
			if (check_module_fused(MX6_MODULE_USB_OTG1)) {
				puts("OTG1: fused\n");
				return -ENODEV;
			}
			/* Set Power polarity */
			setbits_le32(usbnc_usb_ctrl, UCTRL_PWR_POL);
			/* Set Overcurrent polarity */
			setbits_le32(usbnc_usb_ctrl, UCTRL_OC_POL);
			break;
		case 1:
			if (check_module_fused(MX6_MODULE_USB_OTG2)) {
				puts("OTG2: fused\n");
				return -ENODEV;
			}
			/* Disable Overcurrent detection */
			/* managed by 2517i 7port usb hub */
			setbits_le32(usbnc_usb_ctrl, UCTRL_OC_DISABLE);
			break;
		default:
			printf ("USB%d: Initializing port not supported\n", port);
	}

	return 0;
}

int board_ehci_power(int port, int on)
{
	int old_bus;
	/* port index start at 0 */
	if (port > (CONFIG_USB_MAX_CONTROLLER_COUNT - 1) || port < 0)
		return -EINVAL;

	switch (port) {
		case 0:
			gpio_request(USB_OTG1_PWR, "usb-otg1-pwr");
			if (on) {
				/* enable usb-otg */
				gpio_direction_output(USB_OTG1_PWR , 1);
			} else {
				/* disable usb-otg */
				gpio_direction_output(USB_OTG1_PWR , 0);
			}
			break;
		case 1:
			/* power managed by 2517i 7port usb hub */
			old_bus = i2c_get_bus_num();
			i2c_set_bus_num(3);
			pca953x_set_val(0x22, 0x01, (on) ? 1 : 0);
			udelay(10000);
			i2c_set_bus_num(old_bus);
			break;
		default:
			printf ("USB%d: Powering port is not supported\n", port);
	}
	return 0;
}
#endif

int tqc_bb_board_early_init_f(void)
{
	mba6ul_setup_iomuxc_uart();

	return 0;
}

int tqc_bb_board_init(void)
{
	int old_bus;

	/*
	 * init GPIO expander here to have all in place
	 */
	old_bus = i2c_get_bus_num();
	i2c_set_bus_num(3);

	/* TBD: PCIE PWREN HIGH */
	pca953x_set_val(0x20, 0xff, 0x00);
	pca953x_set_dir(0x20, 0xff, 0x80);

	/* all input */
	pca953x_set_dir(0x21, 0xff, 0xff);

	/* 0..6 out, 7 NC */
	pca953x_set_val(0x22, 0xff, 0x00);
	pca953x_set_dir(0x22, 0xff, 0x80);
	/* LED */
	pca953x_set_val(0x22, 0x30, 0x30);

	i2c_set_bus_num(old_bus);

	/* do it here - to have reset completed */
	mba6ul_setup_iomuxc_enet();
	/*
	 * Only ENET2_MII can manage multiple phy's so we need to
	 * init FEC1 first.
	 * TODO:
	 *   Check if we can change this with MBa6UL-REV.0200
	 */
	mba6ul_setup_fec(FEC0);
	mba6ul_setup_fec(FEC1);
	mba6ul_setup_usb();

	return 0;
}

static iomux_v3_cfg_t const mba6ul_wdog_pads[] = {
	NEW_PAD_CTRL(MX6_PAD_GPIO1_IO08__WDOG1_WDOG_B, WDOG_PAD_CTRL),
};

int tqc_bb_board_late_init(void)
{
	/*
	* try to get sd card slots in order:
	* eMMC: on Module
	* -> therefore index 0 for bootloader
	* index n in kernel (controller instance 3) -> patches needed for
	* alias indexing
	* SD1: on Mainboard
	* index n in kernel (controller instance 1) -> patches needed for
	* alias indexing
	* we assume to have a kernel patch that will present mmcblk dev
	* indexed like controller devs
	*/
	puts("Boot:\t");

	switch (get_boot_device()) {
	case MMC2_BOOT:
		printf("USDHC2\n");
		setenv("boot_dev", "mmc");
		/* eMMC (USDHC2)*/
		setenv("mmcblkdev", "0");
		setenv("mmcdev", "0");
		break;
	case SD1_BOOT:
		printf("USDHC1\n");
		setenv("boot_dev", "mmc");
		/* ext SD (USDHC1)*/
		setenv("mmcblkdev", "1");
		setenv("mmcdev", "1");
		break;
	case QSPI_BOOT:
		/* only one qspi controller */
		printf("QSPI1\n");
		setenv("boot_dev", "qspi");
		/* mmcdev is dev index for u-boot */
		setenv("mmcdev", "0");
		/* mmcblkdev is dev index for linux */
		setenv("mmcblkdev", "0");
		break;
	default:
		printf("unhandled boot device %d\n", get_boot_device());
		setenv("mmcblkdev", "");
		setenv("mmcdev", "");
	}

	/* provide default setting for fdt_file if nothing in env is set */
	if (NULL == getenv("fdt_file")) {
		u32 cpurev = get_cpu_rev();
		int use_g1 = check_module_fused(MX6_MODULE_ENET2);

		switch ((cpurev & 0xFF000) >> 12) {
		case MXC_CPU_MX6UL:
			setenv("fdt_file", (use_g1) ?
#if defined(CONFIG_TQMA6UL_VARIANT_STANDARD)
				"imx6ul-mba6ulx-g1.dtb" : "imx6ul-mba6ulx.dtb"
#elif defined(CONFIG_TQMA6UL_VARIANT_LGA)
				"imx6ul-mba6ulx-g1-lga.dtb" : "imx6ul-mba6ulx-lga.dtb"
#else
#error
#endif
			);
			break;
		case MXC_CPU_MX6ULL:
			setenv("fdt_file", (use_g1) ?
#if defined(CONFIG_TQMA6UL_VARIANT_STANDARD)
				"imx6ull-mba6ulx-y1.dtb" : "imx6ull-mba6ulx.dtb"
#elif defined(CONFIG_TQMA6UL_VARIANT_LGA)
				"imx6ull-mba6ulx-y1-lga.dtb" : "imx6ull-mba6ulx-lga.dtb"
#else
#error
#endif
			);
			break;
		default:
			debug("unknown CPU");
		}
	}

	imx_iomux_v3_setup_multiple_pads(mba6ul_wdog_pads,
					 ARRAY_SIZE(mba6ul_wdog_pads));
	set_wdog_reset((struct wdog_regs *)WDOG1_BASE_ADDR);

	return 0;
}

const char *tqc_bb_get_boardname(void)
{
	return "MBa6UL";
}

int board_mmc_get_env_dev(int devno)
{
	/*
	 * This assumes that the baseboard registered
	 * the boot device first ...
	 * Note:
	 * eSDHC1 -> SD
	 * eSDHC2 -> eMMC
	 */
	return (0 == devno) ? 1 : 0;
}

/*
 * Device Tree Support
 */
#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
void tqc_bb_ft_board_setup(void *blob, bd_t *bd)
{

}
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */
