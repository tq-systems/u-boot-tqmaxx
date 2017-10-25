/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 *
 * Copyright (C) 2013, 2014 TQ Systems (ported SabreSD to TQMa6x)
 * Author: Markus Niebel <markus.niebel@tq-group.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>

#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/mach-imx/boot_device.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <fsl_esdhc.h>
#include <i2c.h>
#include <libfdt.h>
#include <linux/errno.h>
#include <malloc.h>
#include <micrel.h>
#include <miiphy.h>
#include <mmc.h>
#include <netdev.h>
#include <spl.h>

#include "tqma6_bb.h"
#include "tqma6_eeprom.h"

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

#define SPI_PAD_CTRL (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
	PAD_CTL_DSE_80ohm | PAD_CTL_SRE_FAST | PAD_CTL_HYS)

#define I2C_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
	PAD_CTL_DSE_80ohm | PAD_CTL_HYS |			\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#define TQMA6Q_IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII	0x020e0790
#define TQMA6Q_IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM	0x020e07ac

#define TQMA6DL_IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII	0x020e0768
#define TQMA6DL_IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM	0x020e0788

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
	IOMUX_PADS(PAD_ENET_MDIO__ENET_MDIO |	MUX_PAD_CTRL(ENET_MDIO_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_MDC__ENET_MDC |	MUX_PAD_CTRL(ENET_MDIO_PAD_CTRL)),

	IOMUX_PADS(PAD_RGMII_TXC__RGMII_TXC |	MUX_PAD_CTRL(ENET_TX_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD0__RGMII_TD0 |	MUX_PAD_CTRL(ENET_TX_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD1__RGMII_TD1 |	MUX_PAD_CTRL(ENET_TX_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD2__RGMII_TD2 |	MUX_PAD_CTRL(ENET_TX_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD3__RGMII_TD3 |	MUX_PAD_CTRL(ENET_TX_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TX_CTL__RGMII_TX_CTL | MUX_PAD_CTRL(ENET_TX_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_REF_CLK__ENET_TX_CLK | (MUX_PAD_CTRL(ENET_CLK_PAD_CTRL) &
		   (~(MUX_MODE_SION)))),
	/*
	 * these pins are also used for config strapping by phy
	 */
	IOMUX_PADS(PAD_RGMII_RD0__RGMII_RD0 |	MUX_PAD_CTRL(ENET_RX_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD1__RGMII_RD1 |	MUX_PAD_CTRL(ENET_RX_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD2__RGMII_RD2 |	MUX_PAD_CTRL(ENET_RX_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD3__RGMII_RD3 |	MUX_PAD_CTRL(ENET_RX_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RXC__RGMII_RXC |	MUX_PAD_CTRL(ENET_RX_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RX_CTL__RGMII_RX_CTL | MUX_PAD_CTRL(ENET_RX_PAD_CTRL)),
	/* KSZ9031 PHY Reset */
	IOMUX_PADS(PAD_ENET_CRS_DV__GPIO1_IO25 | (MUX_PAD_CTRL(GPIO_OUT_PAD_CTRL) |
		   MUX_MODE_SION)),
	/* FEC phy IRQ */
	IOMUX_PADS(PAD_ENET_TX_EN__GPIO1_IO28 |	MUX_PAD_CTRL(GPIO_IN_PAD_CTRL)),
};

static void mba6_setup_iomuxc_enet(void)
{
	struct iomuxc *const iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;

	/* set gpr1[ENET_CLK_SEL] for ENET_REF_CLK / PTP from anatop */
	setbits_le32(&iomuxc_regs->gpr[1], IOMUXC_GPR1_ENET_CLK_SEL_MASK);

	if (is_mx6dqp() || is_mx6dq()) {
		__raw_writel(IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM_DISABLE,
			     (void *)TQMA6Q_IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM);
		__raw_writel(IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII_1P5V,
			     (void *)TQMA6Q_IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII);
	} else {
		__raw_writel(IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM_DISABLE,
			     (void *)TQMA6DL_IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM);
		__raw_writel(IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII_1P5V,
			     (void *)TQMA6DL_IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII);
	};

	SETUP_IOMUX_PADS(mba6_enet_pads);

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
	IOMUX_PADS(PAD_SD4_DAT4__UART2_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT7__UART2_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
};

static void mba6_setup_iomuxc_uart(void)
{
	SETUP_IOMUX_PADS(mba6_uart2_pads);
}

#define USDHC2_CD_GPIO	IMX_GPIO_NR(1, 4)
#define USDHC2_WP_GPIO	IMX_GPIO_NR(1, 2)

int tqma6_bb_board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	if (cfg->esdhc_base == USDHC2_BASE_ADDR)
		ret = !gpio_get_value(USDHC2_CD_GPIO);

	return ret;
}

int tqma6_bb_board_mmc_getwp(struct mmc *mmc)
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
	IOMUX_PADS(PAD_SD2_CLK__SD2_CLK |	MUX_PAD_CTRL(USDHC_CLK_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_CMD__SD2_CMD |	MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT0__SD2_DATA0 |	MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT1__SD2_DATA1 |	MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT2__SD2_DATA2 |	MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT3__SD2_DATA3 |	MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	/* CD */
	IOMUX_PADS(PAD_GPIO_4__GPIO1_IO04 |	MUX_PAD_CTRL(GPIO_IN_PAD_CTRL)),
	/* WP */
	IOMUX_PADS(PAD_GPIO_2__GPIO1_IO02 |	MUX_PAD_CTRL(GPIO_IN_PAD_CTRL)),
};

int tqma6_bb_board_mmc_init(bd_t *bis)
{
	SETUP_IOMUX_PADS(mba6_usdhc2_pads);
	gpio_request(USDHC2_CD_GPIO, "usdhc2-cd");
	gpio_request(USDHC2_WP_GPIO, "usdhc2-wp");
	gpio_direction_input(USDHC2_CD_GPIO);
	gpio_direction_input(USDHC2_WP_GPIO);

	mba6_usdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
	if (fsl_esdhc_initialize(bis, &mba6_usdhc_cfg))
		puts("Warning: failed to initialize SD\n");

	return 0;
}


static struct i2c_pads_info mba6dl_i2c1_pads = {
/* I2C1: MBa6x */
	.scl = {
		.i2c_mode = NEW_PAD_CTRL(MX6DL_PAD_CSI0_DAT9__I2C1_SCL,
					 I2C_PAD_CTRL),
		.gpio_mode = NEW_PAD_CTRL(MX6DL_PAD_CSI0_DAT9__GPIO5_IO27,
					  I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(5, 27)
	},
	.sda = {
		.i2c_mode = NEW_PAD_CTRL(MX6DL_PAD_CSI0_DAT8__I2C1_SDA,
					 I2C_PAD_CTRL),
		.gpio_mode = NEW_PAD_CTRL(MX6DL_PAD_CSI0_DAT8__GPIO5_IO26,
					  I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(5, 26)
	}
};

static struct i2c_pads_info mba6q_i2c1_pads = {
/* I2C1: MBa6x */
	.scl = {
		.i2c_mode = NEW_PAD_CTRL(MX6Q_PAD_CSI0_DAT9__I2C1_SCL,
					 I2C_PAD_CTRL),
		.gpio_mode = NEW_PAD_CTRL(MX6Q_PAD_CSI0_DAT9__GPIO5_IO27,
					  I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(5, 27)
	},
	.sda = {
		.i2c_mode = NEW_PAD_CTRL(MX6Q_PAD_CSI0_DAT8__I2C1_SDA,
					 I2C_PAD_CTRL),
		.gpio_mode = NEW_PAD_CTRL(MX6Q_PAD_CSI0_DAT8__GPIO5_IO26,
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
	if (is_mx6dqp() || is_mx6dq())
		ret = setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &mba6q_i2c1_pads);
	else
		ret = setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &mba6dl_i2c1_pads);

	if (ret)
		printf("setup I2C1 failed: %d\n", ret);
}

struct mba6_phy_skew {
	u16 address;
	u16 value;
};

/*
 * optimized pad skew values depends on CPU variant on the TQMa6x module:
 * i.MX6QP/Q/D
 * i.MX6S/DL
 */
static struct mba6_phy_skew const mba6q_phy_skews[] = {
	/* min rx/tx ctrl delay */
	{ MII_KSZ9031_EXT_RGMII_CTRL_SIG_SKEW, 0x0032 },
	/* min rx delay */
	{ MII_KSZ9031_EXT_RGMII_RX_DATA_SKEW, 0x3333 },
	/* max tx delay */
	{ MII_KSZ9031_EXT_RGMII_TX_DATA_SKEW, 0x2036 },
	/* rx/tx clk skew */
	{ MII_KSZ9031_EXT_RGMII_CLOCK_SKEW, 0x03ff },
};

static struct mba6_phy_skew const mba6dl_phy_skews[] = {
	{ MII_KSZ9031_EXT_RGMII_CTRL_SIG_SKEW, 0x0030 },
	{ MII_KSZ9031_EXT_RGMII_RX_DATA_SKEW, 0x3333 },
	{ MII_KSZ9031_EXT_RGMII_TX_DATA_SKEW, 0x2052 },
	{ MII_KSZ9031_EXT_RGMII_CLOCK_SKEW, 0x03ff },
};

int board_phy_config(struct phy_device *phydev)
{
	unsigned int i;

	if (is_mx6dqp() || is_mx6dq()) {
		for (i = 0; i < ARRAY_SIZE(mba6q_phy_skews); ++i)
			ksz9031_phy_extended_write(phydev, 2,
						   mba6q_phy_skews[i].address,
						   MII_KSZ9031_MOD_DATA_NO_POST_INC,
						   mba6q_phy_skews[i].value
						  );
	} else {
		for (i = 0; i < ARRAY_SIZE(mba6dl_phy_skews); ++i)
			ksz9031_phy_extended_write(phydev, 2,
						   mba6dl_phy_skews[i].address,
						   MII_KSZ9031_MOD_DATA_NO_POST_INC,
						   mba6dl_phy_skews[i].value
						  );
	}

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

int tqma6_bb_board_early_init_f(void)
{
	mba6_setup_iomuxc_uart();

	return 0;
}

int tqma6_bb_board_init(void)
{
	mba6_setup_i2c();
	/* do it here - to have reset completed */
	mba6_setup_iomuxc_enet();

	return 0;
}

int tqma6_bb_board_late_init(void)
{
	int ret;
	struct tqma6_eeprom_data eedat;
	char mac[20];

	ret = tqma6_read_eeprom(tqma6_get_system_i2c_bus(), 0x57, &eedat);
	if (!ret) {
		tqma6_parse_eeprom_mac(&eedat, mac, ARRAY_SIZE(mac));
		env_set("usbethaddr", mac);
		tqma6_show_eeprom(&eedat, "MBA6");
	} else {
		printf("%s EEPROM: err %d\n", tqma6_bb_get_boardname(), ret);
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
	switch (imx_boot_device()) {
	case BOOT_DEVICE_MMC1:
		printf("ESDHC%u\n", imx_boot_device_instance() + 1);
		env_set("boot_dev", "mmc");
		switch (mmc_get_env_devno()) {
		/* eMMC (USDHC3)*/
		case 0:
			env_set("mmcblkdev", "0");
			env_set("mmcdev", "0");
			break;
		/* ext SD (USDHC2)*/
		case 1:
			env_set("mmcblkdev", "1");
			env_set("mmcdev", "1");
			break;
		default:
			puts("unhandled boot device\n");
			env_set("mmcblkdev", "");
			env_set("mmcdev", "");
		};
		break;
	case BOOT_DEVICE_SPI:
		printf("ECSPI%u\n", imx_boot_device_instance() + 1);
		env_set("boot_dev", "spi");
		/* mmcdev is dev index for u-boot */
		env_set("mmcdev", "0");
		/* mmcblkdev is dev index for linux env */
		env_set("mmcblkdev", "0");
		break;
	default:
		puts("unhandled boot device\n");
		env_set("mmcblkdev", "");
		env_set("mmcdev", "");
	}

	/* provide default setting for fdt_file if nothing in env is set */
	if (NULL == env_get("fdt_file")) {
		int enw = (tqma6_get_enet_workaround() > 0);
		u32 cpurev = get_cpu_rev();

		switch ((cpurev & 0xFF000) >> 12) {
		case MXC_CPU_MX6SOLO:
		case MXC_CPU_MX6DL:
			env_set("fdt_file",
				(enw) ? "imx6dl-mba6a.dtb" : "imx6dl-mba6b.dtb");
			break;
		case MXC_CPU_MX6D:
		case MXC_CPU_MX6Q:
			env_set("fdt_file",
				(enw) ? "imx6q-mba6a.dtb" : "imx6q-mba6b.dtb");
			break;
		default:
			debug("unknown CPU");
		}
	}

	return 0;
}

const char *tqma6_bb_get_boardname(void)
{
	return "MBa6x";
}

int mmc_get_env_devno(void)
{
#if defined(CONFIG_SPL_BUILD)
	return 0;
#else
	u32 dev = imx_boot_device();

	if (BOOT_DEVICE_MMC1 == dev) {
		u32 inst = imx_boot_device_instance();
		/*
		 * This assumes that the baseboard registered
		 * the boot device first ...
		 * Note: SDHC3 == idx2
		 */
		return (2 == inst) ? 0 : 1;
	}

	return -1;
#endif
}

/*
 * Device Tree Support
 */
#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
void tqma6_bb_ft_board_setup(void *blob, bd_t *bd)
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
