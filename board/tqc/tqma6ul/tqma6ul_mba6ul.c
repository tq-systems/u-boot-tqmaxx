/*
 * Copyright (C) 2016 TQ Systems
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
#include <asm/imx-common/boot_device.h>
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

#define UART_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_CLK_PAD_CTRL (PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define GPIO_IN_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_40ohm   | PAD_CTL_HYS)

#define GPIO_OUT_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_40ohm   | PAD_CTL_HYS)

#define I2C_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |	\
	PAD_CTL_DSE_80ohm | PAD_CTL_HYS | PAD_CTL_ODE | 		\
	PAD_CTL_SRE_FAST)

#define ENET_RX_PAD_CTRL	(PAD_CTL_DSE_34ohm)
#define ENET_TX_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_DSE_34ohm)
#define ENET_CLK_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_HIGH | \
				 PAD_CTL_DSE_40ohm | PAD_CTL_HYS)
#define ENET_MDIO_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
				 PAD_CTL_DSE_60ohm)
#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_PUE |     \
	PAD_CTL_SPEED_HIGH   |                                   \
	PAD_CTL_DSE_48ohm   | PAD_CTL_SRE_FAST)

enum fec_device {
	FEC0,
	FEC1,
	FEC_ALL
};

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

	/* PHY reset*/
	/* There is a global Baseboard reset */

#if defined(CONFIG_MBA6UL_ENET1_INT)
	/* INT */
	/* TODO: enable on Basebord with R423 */
	NEW_PAD_CTRL(MX6_PAD_CSI_DATA03__GPIO4_IO24, GPIO_IN_PAD_CTRL),
#endif
};
#if defined(CONFIG_MBA6UL_ENET1_INT)
#define ENET1_PHY_INT_GPIO IMX_GPIO_NR(4, 24)
#endif

static iomux_v3_cfg_t const mba6ul_fec2_pads[] = {
	NEW_PAD_CTRL(MX6_PAD_ENET2_RX_DATA0__ENET2_RDATA00, ENET_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET2_RX_DATA1__ENET2_RDATA01, ENET_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET2_RX_EN__ENET2_RX_EN, ENET_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET2_RX_ER__ENET2_RX_ER, ENET_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET2_TX_DATA0__ENET2_TDATA00, ENET_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET2_TX_DATA1__ENET2_TDATA01, ENET_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET2_TX_EN__ENET2_TX_EN, ENET_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_ENET2_TX_CLK__ENET2_REF_CLK2, ENET_CLK_PAD_CTRL),

	/* MDIO */
	/* pins are shared with fec1 */

	/* PHY reset*/
	/* There is a global Baseboard reset */

#if defined(CONFIG_MBA6UL_ENET2_INT)	
	/* INT */
	/* TODO: enable on Basebord with R426 */
	NEW_PAD_CTRL(MX6_PAD_CSI_DATA03__GPIO4_IO24, GPIO_IN_PAD_CTRL),
#endif
};
#if defined(CONFIG_MBA6UL_ENET2_INT)	
#define ENET2_PHY_INT_GPIO IMX_GPIO_NR(4, 21)
#endif

static iomux_v3_cfg_t const mba6ul_fec_common_pads[] = {
	/* MDIO */
	NEW_PAD_CTRL(MX6_PAD_GPIO1_IO06__ENET1_MDIO, ENET_MDIO_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_GPIO1_IO07__ENET1_MDC, ENET_MDIO_PAD_CTRL),
#if defined(CONFIG_MBA6UL_ENET_RST)	
	/* PHY reset*/
	NEW_PAD_CTRL(MX6_PAD_GPIO1_IO02__GPIO1_IO02, GPIO_OUT_PAD_CTRL),
#endif
};
#if defined(CONFIG_MBA6UL_ENET_RST)
#define ENET_PHY_RESET_GPIO IMX_GPIO_NR(1, 2)
#endif

static void mba6ul_setup_iomuxc_enet(void)
{
	imx_iomux_v3_setup_multiple_pads(mba6ul_fec_common_pads,
					 ARRAY_SIZE(mba6ul_fec_common_pads));
	imx_iomux_v3_setup_multiple_pads(mba6ul_fec1_pads,
					 ARRAY_SIZE(mba6ul_fec1_pads));
	imx_iomux_v3_setup_multiple_pads(mba6ul_fec2_pads,
					 ARRAY_SIZE(mba6ul_fec2_pads));
}

int board_eth_init(bd_t *bis)
{
	int ret;

#if defined(CONFIG_MBA6UL_ENET1_INT)
		gpio_request(ENET1_PHY_INT_GPIO, "enet1-phy-int");
		gpio_direction_input(ENET1_PHY_INT_GPIO);
#endif

#if defined(CONFIG_MBA6UL_ENET2_INT)
		gpio_request(ENET2_PHY_INT_GPIO, "enet2-phy-int");
		gpio_direction_input(ENET2_PHY_INT_GPIO);
#endif

#if defined(CONFIG_MBA6UL_ENET_RST)
	/* Reset PHY1 and/or PHY2
	 * 
	 * R1422 -> PHY1 reset (RESOLDER R1417)
	 * R1522 -> PHY2 reset (RESOLDER R1517)
	 * R1422 & R1422 -> PHY1 & PHY2 reset
	 * */
	gpio_request(ENET_PHY_RESET_GPIO, "enet-phy-rst#");
	gpio_direction_output(ENET_PHY_RESET_GPIO , 1);
	udelay(100);
	gpio_direction_output(ENET_PHY_RESET_GPIO , 0);
	/* lan8720: 5.5.3 reset time should be 25ms */
	udelay(24900);
	gpio_set_value(ENET_PHY_RESET_GPIO, 1);
#endif

	ret = fecmxc_initialize_multi(bis, 0, TQMA6UL_ENET1_PHYADDR,
				      ENET_BASE_ADDR);
	if (ret)
		printf("FEC0 MXC: %s:failed %i\n", __func__, ret);

	ret = fecmxc_initialize_multi(bis, 1, TQMA6UL_ENET2_PHYADDR,
				      ENET2_BASE_ADDR);
	if (ret)
		printf("FEC1 MXC: %s:failed %i\n", __func__, ret);

	return ret;
}

static int mba6ul_setup_fec(int fec_id)
{
	struct iomuxc_gpr_base_regs *const iomuxc_gpr_regs
		= (struct iomuxc_gpr_base_regs *) IOMUXC_GPR_BASE_ADDR;
	int ret;
	u32 clrbits, setbits;

	switch (fec_id) {
	case FEC0:
		if (check_module_fused(MX6_MODULE_ENET1))
			return -1;

		/*
		 * Use 50M anatop loopback REF_CLK1 for ENET1,
		 * clear gpr1[13], set gpr1[17]
		 */
		clrsetbits_le32(&iomuxc_gpr_regs->gpr[1], IOMUX_GPR1_FEC1_MASK,
				IOMUX_GPR1_FEC1_CLOCK_MUX1_SEL_MASK);
		break;
	case FEC1:
		if (check_module_fused(MX6_MODULE_ENET2))
			return -1;
		/*
		 * Use 50M anatop loopback REF_CLK1 for ENET2,
		 * clear gpr1[13], set gpr1[17]
		 */
		clrsetbits_le32(&iomuxc_gpr_regs->gpr[1], IOMUX_GPR1_FEC2_MASK,
				IOMUX_GPR1_FEC2_CLOCK_MUX2_SEL_MASK);
		break;
	case FEC_ALL:
		/* TODO: Check if both clocks can be enabled at the same time !*/
		if (check_module_fused(MX6_MODULE_ENET1) &&
		    check_module_fused(MX6_MODULE_ENET2))
			return -1;
		/*
		 * Use 50M anatop loopback REF_CLK1 for ENET1/2,
		 * clear gpr1[13], set gpr1[17]
		 */
		clrbits = (IOMUX_GPR1_FEC1_MASK | IOMUX_GPR1_FEC2_MASK);
		setbits = (IOMUX_GPR1_FEC1_CLOCK_MUX1_SEL_MASK | IOMUX_GPR1_FEC2_CLOCK_MUX2_SEL_MASK);
		clrsetbits_le32(&iomuxc_gpr_regs->gpr[1], clrbits, setbits);
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
	/* TODO: set skew values using phy_read_mmd_indirec from */

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

static struct i2c_pads_info mba6ul_i2c2_pads = {
/* I2C2: MBa6UL */
	.scl = {
		.i2c_mode = NEW_PAD_CTRL(MX6_PAD_CSI_HSYNC__I2C2_SCL,
					 I2C_PAD_CTRL),
		.gpio_mode = NEW_PAD_CTRL(MX6_PAD_CSI_HSYNC__GPIO4_IO20,
					  I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(4, 20)
	},
	.sda = {
		.i2c_mode = NEW_PAD_CTRL(MX6_PAD_CSI_VSYNC__I2C2_SDA,
					 I2C_PAD_CTRL),
		.gpio_mode = NEW_PAD_CTRL(MX6_PAD_CSI_VSYNC__GPIO4_IO19,
					  I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(4, 19)
	}
};

static void mba6ul_setup_i2c(void)
{
	int ret;

	/*
	 * use logical index for bus, e.g. I2C2 -> 1
	 * warn on error
	 */
	ret = setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &mba6ul_i2c2_pads);
	if (ret)
		printf("setup I2C2 failed: %d\n", ret);
}

int tqc_bb_board_early_init_f(void)
{
	mba6ul_setup_iomuxc_uart();
	return 0;
}

int tqc_bb_board_init(void)
{
	mba6ul_setup_i2c();
	/* do it here - to have reset completed */
	mba6ul_setup_iomuxc_enet();
	mba6ul_setup_fec(FEC0);

	return 0;
}

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
	switch (imx_boot_device()) {
	case BOOT_DEVICE_MMC1:
		setenv("boot_dev", "mmc");
		switch (imx_boot_device_instance()) {
		/* SD (USDHC1)*/
		case 0:
		/*
		 * This assumes that the kernel registered the baseboard
		 * as the first boot device ...
		 */
			printf("USDHC%u (SD)\n", imx_boot_device_instance() + 1);
			setenv("mmcblkdev", "0");
			setenv("mmcdev", "1");
			break;
		/* eMMC (USDHC2)*/
		case 1:
			printf("USDHC%u (eMMC)\n", imx_boot_device_instance() + 1);
			setenv("mmcblkdev", "1");
			setenv("mmcdev", "0");
			break;
		default:
			puts("unhandled boot device\n");
			setenv("mmcblkdev", "");
			setenv("mmcdev", "");
		};
		break;
/* TODO: QSPI -> not supported in imx_boot_device(), add qspi enum in arch/arm/include/asm/spl.h */
	case BOOT_DEVICE_SPI:
		/* only ine qspi controller */
		printf("QSPI1\n");
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
	return "MBa6UL";
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
		 * Note:
		 * eSDHC1 -> SD
		 * eSDHC2 -> eMMC
		 */
		return (0 == inst) ? 1 : 0;
	}

	return -1;
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
