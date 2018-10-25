/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 TQ Systems GmbH
 * Max Merchel <Max.Merchel@tq-group.com>
 */

#include "tqmls1012al_bb.h"
#include <asm/gpio.h>
#include <dm.h>
#include <fm_eth.h>
#include <fsl_mdio.h>
#include <malloc.h>
#include <asm/types.h>
#include <fsl_dtsec.h>
#include <asm/arch-fsl-layerscape/config.h>
#include <asm/arch-fsl-layerscape/immap_lsch2.h>
#include <pfe_eth/pfe_eth.h>

int checkboard(void)
{
	puts("Board: MBLS1012AL\n");

	return 0;
}

int get_gpio_number(const char *gpio_name, unsigned int *gpio_number)
{
	int ret = 0;

#if defined(CONFIG_DM_GPIO)
	/*
	 * TODO:Once all GPIO drivers are converted to driver model,
	 * we can change the code here to use the GPIO uclass interface.
	 */
	ret = gpio_lookup_name(gpio_name, NULL, NULL, gpio_number);
#else
	/* turn the gpio name into a gpio number */
	gpio_number = name_to_gpio(gpio_name);
#endif

	return ret;
}

void reset_phy(void)
{
	/* reset#	->	reset: low >= 1 us*/
	/* PWRDWN#	->	power-down mode: low */
}

int board_eth_init(bd_t *bis)
{
	struct mii_dev *bus;
	struct mdio_info mac_mdio_info, mac_mdio1_info;

	unsigned int gpio_number_SW_RST, gpio_number_RST, gpio_number_PWRDWN;
	const char *gpio_name_SW_RST = "gpio@20_7";
	const char *gpio_name_RST = "gpio@20_8";
	const char *gpio_name_PWRDWN = "gpio@70_3";

	struct udevice *i2c_bus_dev, *eth_switch_dev;
	int ret;

	if (get_gpio_number(gpio_name_PWRDWN, &gpio_number_PWRDWN)) {
		printf("ETH: GPIO '%s' not found\n", gpio_name_PWRDWN);
	} else {
		gpio_request(gpio_number_PWRDWN, "eth_PWRDWN");
		gpio_direction_output(gpio_number_PWRDWN, 1);
	}

	if (get_gpio_number(gpio_name_SW_RST, &gpio_number_SW_RST)) {
		printf("ETH: GPIO '%s' not found\n", gpio_name_SW_RST);
	} else {
		gpio_request(gpio_number_SW_RST, "eth_SW_reset");
		gpio_direction_output(gpio_number_SW_RST, 1);
	}

	if (get_gpio_number(gpio_name_RST, &gpio_number_RST)) {
		printf("ETH: GPIO '%s' not found\n", gpio_name_RST);
	} else {
		gpio_request(gpio_number_RST, "eth_reset");
		gpio_direction_output(gpio_number_RST, 1);
	}

	/*
	 * KSZ9897
	 * Set Minimum 1.5 ns delay for RGMII Ingress Internal Delay and
	 * RGMII Egress Internal Delay in XMII Port Control 1 Register
	 */
	ret = uclass_get_device_by_name(UCLASS_I2C, TQMLS1012AL_I2C_BUS_NAME,
					&i2c_bus_dev);
	if (!ret) {
		ret = dm_i2c_probe(i2c_bus_dev, MBLS1012AL_KSZ_I2C_ADDR, 0,
				   &eth_switch_dev);
		if (!ret) {
			ret = i2c_set_chip_offset_len(eth_switch_dev, 2);
			if (!ret) {
				ret = dm_i2c_reg_write(eth_switch_dev,
						       MBLS1012AL_KSZ_ADDR,
						       MBLS1012AL_KSZ_VALUE);
			}
		}
	}

	if (ret)
		printf("%s: could not write to ethernet switch register",
		       __func__);

	init_pfe_scfg_dcfg_regs();

	/* Initialize bus on MDIO0 */
	mac_mdio_info.reg_base = (void *)EMAC1_BASE_ADDR;
	mac_mdio_info.name = DEFAULT_PFE_MDIO_NAME;

	bus = pfe_mdio_init(&mac_mdio_info);
	if (!bus) {
		printf("Failed to register mdio0\n");
		return -1;
	}

	/* Initialize bus on MDIO1 */
	mac_mdio1_info.reg_base = (void *)EMAC2_BASE_ADDR;
	mac_mdio1_info.name = DEFAULT_PFE_MDIO1_NAME;

	bus = pfe_mdio_init(&mac_mdio1_info);
	if (!bus) {
		printf("Failed to register mdio1\n");
		return -1;
	}

	/* Initialize SGMII PHYs on MDIO0 */
	pfe_set_mdio(0, miiphy_get_dev_by_name(DEFAULT_PFE_MDIO_NAME));
	pfe_set_phy_address_mode(0, EMAC1_PHY_ADDR,
				 PHY_INTERFACE_MODE_SGMII);

	/* Initialize TI DP83867CS PHY LEDs as:
	 * LED_3 = 0x0: Link established (not connected)
	 * LED_2 = 0x0: Link established (not connected)
	 * LED_1 = 0xB: Link established, blink for activity (green LED)
	 * LED_0 = 0x5: 1000BT link established (orange LED)
	 */
	miiphy_write(DEFAULT_PFE_MDIO_NAME, EMAC1_PHY_ADDR, 0x18, 0x00B5);

	/* MAC1 */
	pfe_set_mdio(1, miiphy_get_dev_by_name(DEFAULT_PFE_MDIO1_NAME));
	pfe_set_phy_address_mode(1, EMAC2_PHY_ADDR,
				 PHY_INTERFACE_MODE_RGMII);

	cpu_eth_init(bis);

	return pci_eth_init(bis);
}

void tqmls1012al_bb_late_init(void)
{
	/* USB */
	unsigned int gpio_number;
	const char *gpio_name_usb = "gpio@20_6";
	const char *gpio_name_pcie_clk_pd = "gpio@70_0";

	if (get_gpio_number(gpio_name_usb, &gpio_number)) {
		printf("USB: GPIO '%s' not found\n", gpio_name_usb);
	} else {
		gpio_request(gpio_number, "USB_rst");
		gpio_direction_output(gpio_number, 1);
	}

	/* PCIE_CLK_PD - Do not change */
	if (get_gpio_number(gpio_name_pcie_clk_pd, &gpio_number)) {
		printf("PCIE: GPIO '%s' not found\n", gpio_name_pcie_clk_pd);
	} else {
		gpio_request(gpio_number, "PCIE_CLK_PD");
		gpio_direction_output(gpio_number, 1);
	}
}
