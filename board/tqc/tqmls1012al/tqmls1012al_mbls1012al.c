/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 TQ Systems GmbH
 * Authors:
 * 	Max Merchel <Max.Merchel@tq-group.com>
 * 	Matthias Schiffer <matthias.schiffer@tq-group.com>
 */

#include <common.h>

#include "tqmls1012al_bb.h"

#include <asm/gpio.h>
#include <i2c.h>
#include <net/pfe_eth/pfe_eth.h>
#include <dm/platform_data/pfe_dm_eth.h>


/* Ethernet */
#define MBLS1012AL_KSZ_I2C_BUS          0
#define MBLS1012AL_KSZ_I2C_ADDR         0x5f


int checkboard(void)
{
	puts("Board: MBLS1012AL\n");

	return 0;
}

static int gpio_lookup_name_and_request(const char *name,
				        struct gpio_desc *gpio,
				        const char *label, ulong flags)
{
	int ret;

	ret = dm_gpio_lookup_name(name, gpio);
	if (ret)
		return ret;

	ret = dm_gpio_request(gpio, label);
	if (ret)
		return ret;

	dm_gpio_set_dir_flags(gpio, flags);

	return 0;
}

static void mbls1012al_reset_one_gpio(const char *label, const char *name,
				     ulong flags, ulong usecs)
{
	struct gpio_desc gpio;
	int ret;

	ret = gpio_lookup_name_and_request(name, &gpio, label,
					   flags | GPIOD_IS_OUT |
					   GPIOD_IS_OUT_ACTIVE);
	if (ret) {
		printf("Failed to request %s GPIO: %d\n", label, ret);
		return;
	}

	udelay(usecs);
	dm_gpio_set_value(&gpio, 0);
}

static void mbls1012al_reset_phy(struct mii_dev *bus)
{
	/* reset#	->	reset: low >= 1 us*/
	/* PWRDWN#	->	power-down mode: low */

	struct gpio_desc pwrdwn;

	gpio_lookup_name_and_request("gpio@70_3", &pwrdwn, "ETH_LINK_PWRDWN#",
				   GPIOD_ACTIVE_LOW | GPIOD_IS_OUT);

	mbls1012al_reset_one_gpio("ETH_LINK_RST#", "gpio@20_8",
				  GPIOD_ACTIVE_LOW, 1);
	udelay(200);


	/*
	 * Initialize TI DP83867CS PHY LEDs as:
	 *
	 * LED_3 = 0x0, 0x1: NC, force low
	 * LED_2 = 0x5, 0x4: Orange LED: 1000BT link established, active high
	 * LED_1 = 0x0, 0x1: NC, force low
	 * LED_0 = 0xB, 0x4: Green LED: Link established, blink for activity, active high
	 *
	 * The PFE MDIO busses are not configured via Device Tree in U-Boot,
	 * so we do this manually for now.
	 */
	bus->write(bus, CONFIG_PFE_EMAC1_PHY_ADDR, MDIO_DEVAD_NONE,
		   0x18, 0x050b);
	bus->write(bus, CONFIG_PFE_EMAC1_PHY_ADDR, MDIO_DEVAD_NONE,
		   0x19, 0x1414);
}

static void mbls1012al_reset_switch(void)
{
	struct udevice *switch_dev;
	int ret;

	mbls1012al_reset_one_gpio("ETH_SW_RST#", "gpio@20_7",
				  GPIOD_ACTIVE_LOW, 10);
        udelay(100);

	ret = i2c_get_chip_for_busnum(MBLS1012AL_KSZ_I2C_BUS,
				      MBLS1012AL_KSZ_I2C_ADDR, 2, &switch_dev);
	if (ret) {
		printf("Error: failed to get Ethernet switch I2C device: %d\n",
		       ret);
		return;
	}

	/*
	 * KSZ9897
	 * Set Minimum 1.5 ns delay for RGMII Ingress Internal Delay and
	 * RGMII Egress Internal Delay in XMII Port Control 1 Register
	 */
	ret = dm_i2c_reg_write(switch_dev, 0x7301, 0x18);
	if (ret) {
		printf("Error: failed to set Ethernet switch register: %d\n",
		       ret);
		return;
	}
}

int pfe_eth_board_init(struct udevice *dev)
{
	struct pfe_eth_dev *priv = dev_get_priv(dev);
	struct pfe_mdio_info mac_mdio_info;
	int phy_id, phy_mode;
	struct mii_dev *bus;

	switch (priv->gemac_port) {
	case 0:
		/* MAC1 */
		mac_mdio_info.reg_base = (void *)EMAC1_BASE_ADDR;
		mac_mdio_info.name = "PFE_MDIO1";
		phy_id = CONFIG_PFE_EMAC1_PHY_ADDR;
		phy_mode = PHY_INTERFACE_MODE_SGMII;
		break;

	case 1:
		/* MAC2 */
		mac_mdio_info.reg_base = (void *)EMAC2_BASE_ADDR;
		mac_mdio_info.name = "PFE_MDIO2";
		phy_id = CONFIG_PFE_EMAC2_PHY_ADDR;
		phy_mode = PHY_INTERFACE_MODE_RGMII;
		break;

	default:
		return -ENODEV;
	}

	bus = pfe_mdio_init(&mac_mdio_info);
	if (!bus) {
		printf("Failed to register mdio\n");
		return -1;
	}

	switch (priv->gemac_port) {
	case 0:
		mbls1012al_reset_phy(bus);
		break;

	case 1:
		mbls1012al_reset_switch();
		break;
	}

	pfe_set_mdio(priv->gemac_port, bus);
	pfe_set_phy_address_mode(priv->gemac_port, phy_id, phy_mode);

	return 0;
}

void tqmls1012al_bb_late_init(void)
{
	struct gpio_desc pcie_clk_pd;

	/* USB */
	mbls1012al_reset_one_gpio("USB_RST#", "gpio@20_6", GPIOD_ACTIVE_LOW,
				  3000);

	/* PCIE_CLK_PD - Do not change */
	gpio_lookup_name_and_request("gpio@70_0", &pcie_clk_pd, "PCIE_CLK_PD#",
				     GPIOD_ACTIVE_LOW | GPIOD_IS_OUT);
}
