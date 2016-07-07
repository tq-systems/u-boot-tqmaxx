/*
 * Copyright (C) 2016 TQ Systems
 * Author: Markus Niebel <markus.niebel@tq-group.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx7-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/errno.h>
#include <asm/gpio.h>
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
#include <spl.h>

#include "../common/tqc_bb.h"
#include "../common/tqc_eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL	(PAD_CTL_DSE_3P3V_49OHM | PAD_CTL_PUS_PU100KOHM | \
			 PAD_CTL_HYS)

#define USDHC_PAD_CTRL	(PAD_CTL_DSE_3P3V_32OHM | PAD_CTL_SRE_SLOW | \
	PAD_CTL_HYS | PAD_CTL_PUE | PAD_CTL_PUS_PU47KOHM)

#define USDHC_CLK_PAD_CTRL	(USDHC_PAD_CTRL)

#define GPIO_IN_PAD_CTRL	(PAD_CTL_PUS_PU5KOHM | PAD_CTL_DSE_3P3V_98OHM)
#define GPIO_OUT_PAD_CTRL (PAD_CTL_PUS_PU5KOHM | PAD_CTL_DSE_3P3V_98OHM)

#define I2C_PAD_CTRL	(PAD_CTL_DSE_3P3V_32OHM | PAD_CTL_SRE_SLOW | \
	PAD_CTL_HYS | PAD_CTL_PUE | PAD_CTL_PUS_PU100KOHM)

#define ENET_PAD_CTRL		(PAD_CTL_PUS_PU100KOHM | PAD_CTL_DSE_3P3V_49OHM)
#define ENET_RX_PAD_CTRL	(PAD_CTL_PUS_PU100KOHM | PAD_CTL_DSE_3P3V_49OHM)
#define ENET_PAD_CTRL_MII	(PAD_CTL_DSE_3P3V_32OHM)

static iomux_v3_cfg_t const mba7_fec1_pads[] = {
	MX7D_PAD_ENET1_RGMII_RX_CTL__ENET1_RGMII_RX_CTL | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX7D_PAD_ENET1_RGMII_RD0__ENET1_RGMII_RD0 | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX7D_PAD_ENET1_RGMII_RD1__ENET1_RGMII_RD1 | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX7D_PAD_ENET1_RGMII_RD2__ENET1_RGMII_RD2 | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX7D_PAD_ENET1_RGMII_RD3__ENET1_RGMII_RD3 | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX7D_PAD_ENET1_RGMII_RXC__ENET1_RGMII_RXC | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX7D_PAD_ENET1_RGMII_TX_CTL__ENET1_RGMII_TX_CTL | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX7D_PAD_ENET1_RGMII_TD0__ENET1_RGMII_TD0 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX7D_PAD_ENET1_RGMII_TD1__ENET1_RGMII_TD1 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX7D_PAD_ENET1_RGMII_TD2__ENET1_RGMII_TD2 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX7D_PAD_ENET1_RGMII_TD3__ENET1_RGMII_TD3 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX7D_PAD_ENET1_RGMII_TXC__ENET1_RGMII_TXC | MUX_PAD_CTRL(ENET_PAD_CTRL),

	MX7D_PAD_GPIO1_IO10__ENET1_MDIO | MUX_PAD_CTRL(ENET_PAD_CTRL_MII),
	MX7D_PAD_GPIO1_IO11__ENET1_MDC | MUX_PAD_CTRL(ENET_PAD_CTRL_MII),
	/* PHY reset */
	MX7D_PAD_ENET1_COL__GPIO7_IO15 | MUX_PAD_CTRL(GPIO_OUT_PAD_CTRL),
	/* INT */
	MX7D_PAD_GPIO1_IO09__GPIO1_IO9 | MUX_PAD_CTRL(GPIO_IN_PAD_CTRL),
};

#define ENET1_PHY_RESET_GPIO IMX_GPIO_NR(7, 15)
#define ENET1_PHY_INT_GPIO IMX_GPIO_NR(1, 9)

static iomux_v3_cfg_t const mba7_fec2_pads[] = {
	MX7D_PAD_EPDC_SDCE0__ENET2_RGMII_RX_CTL | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX7D_PAD_EPDC_SDCLK__ENET2_RGMII_RD0 | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX7D_PAD_EPDC_SDLE__ENET2_RGMII_RD1  | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX7D_PAD_EPDC_SDOE__ENET2_RGMII_RD2  | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX7D_PAD_EPDC_SDSHR__ENET2_RGMII_RD3 | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX7D_PAD_EPDC_SDCE1__ENET2_RGMII_RXC | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX7D_PAD_EPDC_GDRL__ENET2_RGMII_TX_CTL | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX7D_PAD_EPDC_SDCE2__ENET2_RGMII_TD0 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX7D_PAD_EPDC_SDCE3__ENET2_RGMII_TD1 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX7D_PAD_EPDC_GDCLK__ENET2_RGMII_TD2 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX7D_PAD_EPDC_GDOE__ENET2_RGMII_TD3  | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX7D_PAD_EPDC_GDSP__ENET2_RGMII_TXC  | MUX_PAD_CTRL(ENET_PAD_CTRL),

	MX7D_PAD_SD2_CD_B__ENET2_MDIO | MUX_PAD_CTRL(ENET_PAD_CTRL_MII),
	MX7D_PAD_SD2_WP__ENET2_MDC | MUX_PAD_CTRL(ENET_PAD_CTRL_MII),

	/* PHY reset */
	MX7D_PAD_EPDC_BDR0__GPIO2_IO28 | MUX_PAD_CTRL(GPIO_OUT_PAD_CTRL),
	/* INT */
	MX7D_PAD_EPDC_PWR_STAT__GPIO2_IO31 | MUX_PAD_CTRL(GPIO_IN_PAD_CTRL),
};

#define ENET2_PHY_RESET_GPIO IMX_GPIO_NR(2, 28)
#define ENET2_PHY_INT_GPIO IMX_GPIO_NR(2, 31)

static void mba7_setup_iomuxc_enet(void)
{
	imx_iomux_v3_setup_multiple_pads(mba7_fec1_pads,
					 ARRAY_SIZE(mba7_fec1_pads));

	gpio_request(ENET1_PHY_RESET_GPIO, "enet1-phy-rst#");
	gpio_request(ENET1_PHY_INT_GPIO, "enet1-phy-int");
	gpio_direction_input(ENET1_PHY_INT_GPIO);
	/* Reset PHY */
	gpio_direction_output(ENET1_PHY_RESET_GPIO , 0);
	udelay(20);
	gpio_set_value(ENET1_PHY_RESET_GPIO, 1);
	udelay(250);
	/* TODO: only for TQMa7D, not available on TQMa7S */
	imx_iomux_v3_setup_multiple_pads(mba7_fec2_pads,
					 ARRAY_SIZE(mba7_fec2_pads));
	gpio_request(ENET2_PHY_RESET_GPIO, "enet2-phy-rst#");
	gpio_request(ENET2_PHY_INT_GPIO, "enet2-phy-int");
	gpio_direction_input(ENET2_PHY_INT_GPIO);
	/* Reset PHY */
	gpio_direction_output(ENET2_PHY_RESET_GPIO , 0);
	udelay(20);
	gpio_set_value(ENET2_PHY_RESET_GPIO, 1);
	udelay(250);
}

int board_eth_init(bd_t *bis)
{
	int ret;

	ret = fecmxc_initialize_multi(bis, 0, TQMA7_ENET1_PHYADDR,
				      ENET_IPS_BASE_ADDR);
	if (ret)
		printf("FEC0 MXC: %s:failed %i\n", __func__, ret);

	ret = fecmxc_initialize_multi(bis, 1, TQMA7_ENET2_PHYADDR,
				      ENET2_IPS_BASE_ADDR);
	if (ret)
		printf("FEC1 MXC: %s:failed %i\n", __func__, ret);

	return ret;
}

static int mba7_setup_fec(int fec_id)
{
	struct iomuxc_gpr_base_regs *const iomuxc_gpr_regs
		= (struct iomuxc_gpr_base_regs *) IOMUXC_GPR_BASE_ADDR;
	int ret;

	switch (fec_id) {
	case 0:
		/* Use 125M anatop REF_CLK1 for ENET1, clear gpr1[13], gpr1[17]*/
		clrsetbits_le32(&iomuxc_gpr_regs->gpr[1],
			(IOMUXC_GPR_GPR1_GPR_ENET1_TX_CLK_SEL_MASK |
			 IOMUXC_GPR_GPR1_GPR_ENET1_CLK_DIR_MASK), 0);
		break;
	case 1:
		/* Use 125M anatop REF_CLK2 for ENET2, clear gpr1[14], gpr1[18]*/
		clrsetbits_le32(&iomuxc_gpr_regs->gpr[1],
			(IOMUXC_GPR_GPR1_GPR_ENET2_TX_CLK_SEL_MASK |
			 IOMUXC_GPR_GPR1_GPR_ENET2_CLK_DIR_MASK), 0);
	default:
		printf("FEC%d: unsupported\n", fec_id);
		return -1;
	}

	ret = set_clk_enet(ENET_125MHz);
	if (ret)
		return ret;

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	/* TODO: set skew values using phy_read_mmd_indirec from 
	* /driver/net/phy/ti - see also LS102xa for detaiuls about LED config
	*/

	if (phydev->drv->config)
		phydev->drv->config(phydev);
	return 0;
}


static iomux_v3_cfg_t const mba7_uart2_pads[] = {
	NEW_PAD_CTRL(MX7D_PAD_EPDC_DATA08__UART6_DCE_RX, UART_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_EPDC_DATA09__UART6_DCE_TX, UART_PAD_CTRL),
};

static void mba7_setup_iomuxc_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(mba7_uart2_pads,
					 ARRAY_SIZE(mba7_uart2_pads));
}

static iomux_v3_cfg_t const mba7_usdhc2_pads[] = {
	NEW_PAD_CTRL(MX7D_PAD_SD1_CLK__SD1_CLK,		USDHC_CLK_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_SD1_CMD__SD1_CMD,		USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_SD1_DATA0__SD1_DATA0,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_SD1_DATA1__SD1_DATA1,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_SD1_DATA2__SD1_DATA2,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_SD1_DATA3__SD1_DATA3,	USDHC_PAD_CTRL),
	/* CD */
	NEW_PAD_CTRL(MX7D_PAD_SD1_CD_B__GPIO5_IO0,	GPIO_IN_PAD_CTRL),
	/* WP */
	NEW_PAD_CTRL(MX7D_PAD_SD1_WP__GPIO5_IO1,	GPIO_IN_PAD_CTRL),
};

#define USDHC1_CD_GPIO	IMX_GPIO_NR(5, 0)
#define USDHC1_WP_GPIO	IMX_GPIO_NR(5, 1)

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

static struct fsl_esdhc_cfg mba7_usdhc_cfg = {
	.esdhc_base = USDHC1_BASE_ADDR,
	.max_bus_width = 4,
};

int tqc_bb_board_mmc_init(bd_t *bis)
{
	imx_iomux_v3_setup_multiple_pads(mba7_usdhc2_pads,
					 ARRAY_SIZE(mba7_usdhc2_pads));
	gpio_request(USDHC1_CD_GPIO, "usdhc1-cd");
	gpio_request(USDHC1_WP_GPIO, "usdhc1-wp");
	gpio_direction_input(USDHC1_CD_GPIO);
	gpio_direction_input(USDHC1_WP_GPIO);

	mba7_usdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
	if (fsl_esdhc_initialize(bis, &mba7_usdhc_cfg))
		puts("Warning: failed to initialize SD\n");

	return 0;
}


static struct i2c_pads_info mba7_i2c2_pads = {
/* I2C2: MBa7x */
	.scl = {
		.i2c_mode = NEW_PAD_CTRL(MX7D_PAD_I2C2_SCL__I2C2_SCL,
					 I2C_PAD_CTRL),
		.gpio_mode = NEW_PAD_CTRL(MX7D_PAD_I2C2_SCL__GPIO4_IO10,
					  I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(4, 10)
	},
	.sda = {
		.i2c_mode = NEW_PAD_CTRL(MX7D_PAD_I2C2_SDA__I2C2_SDA,
					 I2C_PAD_CTRL),
		.gpio_mode = NEW_PAD_CTRL(MX7D_PAD_I2C2_SDA__GPIO4_IO11,
					  I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(4, 11)
	}
};

static void mba7_setup_i2c(void)
{
	int ret;

	/*
	 * use logical index for bus, e.g. I2C1 -> 0
	 * warn on error
	 */
	ret = setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &mba7_i2c2_pads);
	if (ret)
		printf("setup I2C2 failed: %d\n", ret);
}

int tqc_bb_board_early_init_f(void)
{
	mba7_setup_iomuxc_uart();

	return 0;
}

int tqc_bb_board_init(void)
{
	mba7_setup_i2c();
	/* do it here - to have reset completed */
	mba7_setup_iomuxc_enet();
	mba7_setup_fec(0);
	/* TODO: only for TQMa7D */
	mba7_setup_fec(1);

	return 0;
}

int tqc_bb_board_late_init(void)
{
	struct bootrom_sw_info **p =
		(struct bootrom_sw_info **)ROM_SW_INFO_ADDR;

	u8 boot_type = (*p)->boot_dev_type;
	u8 dev_no = (*p)->boot_dev_instance;
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

	switch (boot_type) {
	case BOOT_TYPE_SD:
	case BOOT_TYPE_MMC:
		printf("USDHC%u\n", (unsigned)dev_no + 1);
		setenv("boot_dev", "mmc");
		switch (mmc_get_env_devno()) {
		/* eMMC (USDHC3)*/
		case 0:
			setenv("mmcblkdev", "0");
			setenv("mmcdev", "0");
			break;
		/* ext SD (USDHC1)*/
		case 1:
			setenv("mmcblkdev", "1");
			setenv("mmcdev", "1");
			break;
		default:
			puts("unhandled boot device\n");
			setenv("mmcblkdev", "");
			setenv("mmcdev", "");
		};
		break;
/* TODO: QSPI */
	case BOOT_TYPE_QSPI:
		printf("QSPI%u\n", (unsigned)dev_no + 1);
		setenv("boot_dev", "qspi");
		/* mmcdev is dev index for u-boot */
		setenv("mmcdev", "0");
		/* mmcblkdev is dev index for linux env */
		setenv("mmcblkdev", "0");
		break;
	default:
		puts("unhandled boot device\n");
		setenv("mmcblkdev", "");
		setenv("mmcdev", "");
	}

	return 0;
}

const char *tqc_bb_get_boardname(void)
{
	return "MBa7";
}


int mmc_get_env_devno(void)
{
#if defined(CONFIG_SPL_BUILD)
	return 0;
#else
	struct bootrom_sw_info **p =
		(struct bootrom_sw_info **)ROM_SW_INFO_ADDR;

	u8 boot_type = (*p)->boot_dev_type;
	u8 dev_no = (*p)->boot_dev_instance;

	/* If not boot from sd/mmc, use default value */
	if ((boot_type != BOOT_TYPE_SD) && (boot_type != BOOT_TYPE_MMC))
		return CONFIG_SYS_MMC_ENV_DEV;

	if (2 == dev_no)
		dev_no = 1;

	return dev_no;
#endif
}

/*
 * Device Tree Support
 */
#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
void tqc_bb_ft_board_setup(void *blob, bd_t *bd)
{

}
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */
