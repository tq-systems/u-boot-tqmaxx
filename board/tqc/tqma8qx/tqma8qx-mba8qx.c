/*
 * Copyright 2018-2019 TQ Systems GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <dm.h>
#include <environment.h>
#include <errno.h>
#include <fdt_support.h>
#include <fsl_esdhc.h>
#include <fsl_ifc.h>
#include <linux/libfdt.h>
#include <malloc.h>
#include <netdev.h>
#include <power-domain.h>

#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/mach-imx/sci/sci.h>
#include <asm/arch/imx8-pins.h>
#include <asm/arch/iomux.h>
#include <asm/arch/lpcg.h>
#include <asm/arch/sys_proto.h>
#include <power-domain.h>

#include "../common/tqc_bb.h"
#include "../common/tqc_eeprom.h"


DECLARE_GLOBAL_DATA_PTR;

#define ESDHC_PAD_CTRL	((SC_PAD_CONFIG_NORMAL << PADRING_CONFIG_SHIFT) | (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) \
						| (SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define ESDHC_CLK_PAD_CTRL	((SC_PAD_CONFIG_OUT_IN << PADRING_CONFIG_SHIFT) | (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) \
						| (SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))


#define ENET_INPUT_PAD_CTRL	((SC_PAD_CONFIG_OD_IN << PADRING_CONFIG_SHIFT) | (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) \
						| (SC_PAD_28FDSOI_DSE_18V_10MA << PADRING_DSE_SHIFT) | (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define ENET_NORMAL_PAD_CTRL	((SC_PAD_CONFIG_NORMAL << PADRING_CONFIG_SHIFT) | (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) \
						| (SC_PAD_28FDSOI_DSE_18V_10MA << PADRING_DSE_SHIFT) | (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define FSPI_PAD_CTRL	((SC_PAD_CONFIG_NORMAL << PADRING_CONFIG_SHIFT) | (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) \
						| (SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define GPIO_PAD_CTRL	((SC_PAD_CONFIG_NORMAL << PADRING_CONFIG_SHIFT) | (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) \
						| (SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define UART_PAD_CTRL	((SC_PAD_CONFIG_OUT_IN << PADRING_CONFIG_SHIFT) | (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) \
						| (SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

static iomux_cfg_t uart1_pads[] = {
	SC_P_UART1_RX | MUX_MODE_ALT(0) | MUX_PAD_CTRL(UART_PAD_CTRL),
	SC_P_UART1_TX | MUX_MODE_ALT(0) | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx8_iomux_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
}

int tqc_bb_board_early_init_f(void)
{
	sc_ipc_t ipcHndl = 0;
	sc_err_t sciErr = 0;

	ipcHndl = gd->arch.ipc_channel_handle;

	/* Power up UART1 */
	sciErr = sc_pm_set_resource_power_mode(ipcHndl, SC_R_UART_1, SC_PM_PW_MODE_ON);
	if (sciErr != SC_ERR_NONE)
		return 0;

	/* Set UART1 clock root to 80 MHz */
	sc_pm_clock_rate_t rate = 80000000;
	sciErr = sc_pm_set_clock_rate(ipcHndl, SC_R_UART_1, 2, &rate);
	if (sciErr != SC_ERR_NONE)
		return 0;

	/* Enable UART1 clock root */
	sciErr = sc_pm_clock_enable(ipcHndl, SC_R_UART_1, 2, true, false);
	if (sciErr != SC_ERR_NONE)
		return 0;

	LPCG_AllClockOn(LPUART_1_LPCG);

	setup_iomux_uart();

	return 0;
}

static iomux_cfg_t usdhc1_sd[] = {
	SC_P_USDHC1_CLK | MUX_PAD_CTRL(ESDHC_CLK_PAD_CTRL),
	SC_P_USDHC1_CMD | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC1_DATA0 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC1_DATA1 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC1_DATA2 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC1_DATA3 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC1_WP    | MUX_PAD_CTRL(ESDHC_PAD_CTRL), /* Mux for WP, GPIO4,21 in MUX_MODE_ALT(4) */
	SC_P_USDHC1_CD_B  | MUX_MODE_ALT(4) | MUX_PAD_CTRL(ESDHC_PAD_CTRL), /* Mux for CD,  GPIO4 IO22 */
	SC_P_USDHC1_RESET_B | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC1_VSELECT | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
};

#define USDHC1_CD_GPIO	IMX_GPIO_NR(4, 22)

static struct fsl_esdhc_cfg usdhc_cfg = {
	USDHC2_BASE_ADDR, 0, 4,
};

int tqc_bb_board_mmc_init(bd_t *bis)
{
	int ret;
	struct power_domain pd;

	/*
	 * According to the board_mmc_init() the following map is done:
	 * (U-boot device node)    (Physical Port)
	 * mmc0                    USDHC1
	 * mmc1                    USDHC2
	 */
	if (!power_domain_lookup_name("conn_sdhc1", &pd))
		power_domain_on(&pd);
	imx8_iomux_setup_multiple_pads(usdhc1_sd, ARRAY_SIZE(usdhc1_sd));
	init_clk_usdhc(1);
	usdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
	gpio_request(USDHC1_CD_GPIO, "sd1_cd");
	gpio_direction_input(USDHC1_CD_GPIO);

	ret = fsl_esdhc_initialize(bis, &usdhc_cfg);
	if (ret) {
		printf("Warning: failed to initialize USDHC2\n");
		return ret;
	}

	return ret;
}

static iomux_cfg_t pad_enet1[] = {
	SC_P_SPDIF0_EXT_CLK | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),

	SC_P_SPDIF0_TX | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_SPDIF0_RX | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ESAI0_TX3_RX2 | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ESAI0_TX2_RX3 | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ESAI0_TX1 | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ESAI0_TX0 | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ESAI0_SCKR | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ESAI0_TX4_RX1 | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ESAI0_TX5_RX0 | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ESAI0_FST  | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ESAI0_SCKT | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ESAI0_FSR  | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
#if 0
	/* Shared MDIO */
	SC_P_ENET0_MDC | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET0_MDIO | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
#endif
	SC_P_CSI_RESET | MUX_MODE_ALT(4) | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
};

static iomux_cfg_t pad_enet0[] = {
	SC_P_ENET0_REFCLK_125M_25M | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),

	SC_P_ENET0_RGMII_RX_CTL | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ENET0_RGMII_RXD0 | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ENET0_RGMII_RXD1 | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ENET0_RGMII_RXD2 | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ENET0_RGMII_RXD3 | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ENET0_RGMII_RXC | MUX_PAD_CTRL(ENET_INPUT_PAD_CTRL),
	SC_P_ENET0_RGMII_TX_CTL | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET0_RGMII_TXD0 | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET0_RGMII_TXD1 | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET0_RGMII_TXD2 | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET0_RGMII_TXD3 | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET0_RGMII_TXC | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),

	/* Shared MDIO */
	SC_P_ENET0_MDC | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
	SC_P_ENET0_MDIO | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),

	SC_P_CSI_EN | MUX_MODE_ALT(4) | MUX_PAD_CTRL(ENET_NORMAL_PAD_CTRL),
};

struct fec_port_info {
	unsigned id;
	unsigned base;
	unsigned phy_addr;
	const char *pwr_domain;
	const char *rst_gpio_name;
	const char *rst_gpio_label;
	iomux_cfg_t *pad_cfg;
	size_t num_pads;
};

static const struct fec_port_info fpi[] = {
	{
		.id = 0U,
		.base = MX8QX_FEC1_BASE,
		.phy_addr = 0U,
		.pwr_domain = "conn_enet0",
		.rst_gpio_label = "gpio@3_02",
		.rst_gpio_name = "enet0_reset",
		.pad_cfg = pad_enet0,
		.num_pads = ARRAY_SIZE(pad_enet0),
	}, {
		.id = 1,
		.base = MX8QX_FEC2_BASE,
		.phy_addr = 3,
		.pwr_domain = "conn_enet1",
		.rst_gpio_label = "gpio@3_03",
		.rst_gpio_name = "enet1_reset",
		.pad_cfg = pad_enet1,
		.num_pads = ARRAY_SIZE(pad_enet1),
	},
};

static void enet_phy_reset_one(const struct fec_port_info *fpi)
{
	struct gpio_desc desc;
	int ret;

	ret = dm_gpio_lookup_name(fpi->rst_gpio_name, &desc);
	if (ret)
		return;

	ret = dm_gpio_request(&desc, fpi->rst_gpio_label);
	if (ret)
		return;

	dm_gpio_set_dir_flags(&desc, GPIOD_IS_OUT);
	dm_gpio_set_value(&desc, 0);
	udelay(50);
	dm_gpio_set_value(&desc, 1);

	/* TODO: */
	/* The board has a long delay for this reset to become stable */
	mdelay(200);
}

int board_eth_init(bd_t *bis)
{
	int ret;
	unsigned idx;
	struct power_domain pd;

	printf("[%s] %d\n", __func__, __LINE__);

	for (idx = 0; idx < ARRAY_SIZE(fpi); ++idx) {
		const struct fec_port_info *fpti = &fpi[idx];

		if (!power_domain_lookup_name(fpti->pwr_domain, &pd))
			power_domain_on(&pd);

		imx8_iomux_setup_multiple_pads(fpti->pad_cfg, fpti->num_pads);

		enet_phy_reset_one(fpti);

		ret = fecmxc_initialize_multi(bis, fpti->id, fpti->phy_addr,
					      fpti->base);

		if (ret)
			printf("FEC%u MXC: %s:failed\n", fpti->id, __func__);
	}

	return 0;
}

int tqc_bb_checkboard(void)
{
	puts("Board: TQMa8QX on MBa8QX (iMX8QXP)\n");

	return 0;
}

int tqc_bb_board_init(void)
{
	unsigned idx;

	for (idx = 0; idx < ARRAY_SIZE(fpi); ++idx) {
		const struct fec_port_info *fpti = &fpi[idx];

		enet_phy_reset_one(fpti);
	}

	return 0;
}

#ifdef CONFIG_OF_BOARD_SETUP
int tqc_bb_ft_board_setup(void *blob, bd_t *bd)
{
	return 0;
}
#endif

int board_mmc_get_env_dev(int devno)
{
	return devno;
}

int tqc_bb_board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", "MBa8QX");
	env_set("board_rev", "iMX8QXP");
#endif
	return 0;

#ifdef CONFIG_ENV_IS_IN_MMC
	board_late_mmc_env_init();
#endif
}

