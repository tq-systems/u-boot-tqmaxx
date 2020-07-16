// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 *
 */

#include <common.h>
#include <command.h>
#include <i2c.h>
#include <asm/gpio.h>
#include <netdev.h>
#include <malloc.h>
#include <fsl_mdio.h>
#include <miiphy.h>
#include <phy.h>
#include <fm_eth.h>
#include <asm/io.h>
#include <exports.h>
#include <asm/arch/fsl_serdes.h>
#include <fsl-mc/fsl_mc.h>
#include <fsl-mc/ldpaa_wriop.h>

DECLARE_GLOBAL_DATA_PTR;

struct mblx2160a_gpio {
	char name[20];
	char description[20];
	unsigned long flags;
	s8 initval;
};

struct mblx2160a_gpio mblx2160a_gpios[] = {
	{"gpio@20_0",		"QSFP_MODSEL",		GPIOD_IS_OUT, 0 },
	{"gpio@20_1",		"QSFP_RESET#",		GPIOD_IS_OUT, 1 },
	{"gpio@20_2",		"QSFP_MODPRS#",		GPIOD_IS_IN, 0 },
	{"gpio@20_3",		"QSFP_INT#",		GPIOD_IS_IN, 0 },
	{"gpio@20_4",		"QSFP_LPMOD",		GPIOD_IS_IN, 0 },
	{"gpio@20_5",		"QSFP_RETIMER_1#",	GPIOD_IS_IN, 0 },
	{"gpio@20_6",		"QSFP_RETIMER_2#",	GPIOD_IS_IN, 0 },
	{"gpio@20_7",		"MPCIE_1_WAKE#",	GPIOD_IS_OUT, 0 },
	{"gpio@20_8",		"MPCIE_1_DISABLE#",	GPIOD_IS_OUT, 0 },
	{"gpio@20_9",		"MPCIE_1_RESET#",	GPIOD_IS_OUT, 0 },
	{"gpio@20_10",		"MPCIE_2_WAKE#",	GPIOD_IS_OUT, 0 },
	{"gpio@20_11",		"MPCIE_2_DISABLE#",	GPIOD_IS_OUT, 0 },
	{"gpio@20_12",		"MPCIE_2_RESET#",	GPIOD_IS_OUT, 0 },
	{"gpio@20_13",		"EN_SILENT_CAN1",	GPIOD_IS_OUT, 0 },
	{"gpio@20_14",		"EN_SILENT_CAN2",	GPIOD_IS_OUT, 0 },
	{"gpio@20_15",		"SIM_CARD_DETECT",	GPIOD_IS_IN, 0 },

	{"gpio@21_0",		"RESET_USB_HUB#",	GPIOD_IS_OUT, 0 },
	{"gpio@21_1",		"RESET_ETH1",		GPIOD_IS_OUT, 0 },
	{"gpio@21_2",		"RESET_ETH2",		GPIOD_IS_OUT, 0 },
	{"gpio@21_3",		"RESET_ETH3",		GPIOD_IS_OUT, 0 },
	{"gpio@21_4",		"RESET_ETH4",		GPIOD_IS_OUT, 0 },
	{"gpio@21_5",		"RESET_ETH5",		GPIOD_IS_OUT, 0 },
	{"gpio@21_6",		"RESET_ETH6",		GPIOD_IS_OUT, 0 },
	{"gpio@21_7",		"RESET_ETH7",		GPIOD_IS_OUT, 0 },
	{"gpio@21_8",		"RESET_ETH8",		GPIOD_IS_OUT, 0 },
	{"gpio@21_9",		"RESET_ETH9",		GPIOD_IS_OUT, 0 },
	{"gpio@21_10",		"RESET_ETH10",		GPIOD_IS_OUT, 0 },
	{"gpio@21_11",		"CAN1_SEL",		GPIOD_IS_OUT, 1 },
	{"gpio@21_12",		"CAN2_SEL",		GPIOD_IS_OUT, 1 },
	{"gpio@21_13",		"RST_M2_SATA_1",	GPIOD_IS_OUT, 1 },
	{"gpio@21_14",		"RST_M2_SATA_2",	GPIOD_IS_OUT, 1 },
	{"gpio@21_15",		"EN_USER_LED_2",	GPIOD_IS_OUT, 1 },

	{"gpio@22_0",		"XFI1_TX_FAULT",	GPIOD_IS_IN, -1},
	{"gpio@22_1",		"XFI1_TX_DIS",		GPIOD_IS_OUT, 1},
	{"gpio@22_2",		"XFI1_MOD_DECTECT",	GPIOD_IS_IN, -1},
	{"gpio@22_3",		"XFI1_RX_LOSS",		GPIOD_IS_IN, -1},
	{"gpio@22_4",		"XFI2_TX_FAULT",	GPIOD_IS_IN, -1},
	{"gpio@22_5",		"XFI2_TX_DIS",		GPIOD_IS_OUT, 1},
	{"gpio@22_6",		"XFI2_MOD_DECTECT",	GPIOD_IS_IN, -1},
	{"gpio@22_7",		"XFI2_RX_LOSS",		GPIOD_IS_IN, -1},
	{"gpio@22_8",		"XFI1_RET_LOSS",	GPIOD_IS_IN, -1},
	{"gpio@22_9",		"XFI2_RET_LOSS",	GPIOD_IS_IN, -1},
	{"gpio@22_10",		"PCIE_1_PERST#",	GPIOD_IS_OUT, -1},
	{"gpio@22_11",		"PCIE_2_PERST#",	GPIOD_IS_OUT, -1},
	{"gpio@22_12",		"PCIE_WAKE#",		GPIOD_IS_OUT, -1},
	{"gpio@22_13",		"X8_PRSNT1#",		GPIOD_IS_IN, -1},
	{"gpio@22_14",		"X4_1_PRSNT1#",		GPIOD_IS_IN, -1},
	{"gpio@22_15",		"X4_2_PRSNT1#",		GPIOD_IS_IN, -1},
};

static int __gpio_idx_by_name(const char *name)
{
	int i;

	if ((name == NULL) || (strlen(name) <= 0))
		return -1;

	for (i = 0; i < ARRAY_SIZE(mblx2160a_gpios); i++) {
		if ((strlen(mblx2160a_gpios[i].description) == strlen(name)) &&
		   (strncmp(mblx2160a_gpios[i].description, name, strlen(name)) == 0)) {
			return i;
		}
	}

	return -1;
}

int mblx2160a_gpios_init(void)
{
	struct gpio_desc desc;
	int ret;

	for (int i = 0; i < ARRAY_SIZE(mblx2160a_gpios); i++) {
		ret = dm_gpio_lookup_name(mblx2160a_gpios[i].name, &desc);

		if (ret)
			return ret;

		dm_gpio_request(&desc, mblx2160a_gpios[i].description);

		if (ret)
			return ret;

		desc.flags = mblx2160a_gpios[i].flags;

		dm_gpio_set_dir(&desc);

		if (mblx2160a_gpios[i].flags & GPIOD_IS_OUT)
			ret = dm_gpio_set_value(&desc, mblx2160a_gpios[i].initval);

		if (ret)
			return ret;
	}

	return 0;
}

int mblx2160a_get_gpio(const char *name)
{
	int i;
	int ret;
	struct gpio_desc desc;

	i = __gpio_idx_by_name(name);

	ret = dm_gpio_lookup_name(mblx2160a_gpios[i].name, &desc);

	if (ret)
		return ret;

	desc.flags = mblx2160a_gpios[i].flags;

	return dm_gpio_get_value(&desc);
}

int mblx2160a_set_gpio(const char *name, int val)
{
	int i;
	int ret;
	struct gpio_desc desc;

	i = __gpio_idx_by_name(name);

	ret = dm_gpio_lookup_name(mblx2160a_gpios[i].name, &desc);

	if (ret) {
		printf("gpio not found %s: num: %d error: %d\n", name, i, ret);
		return ret;
	}

	desc.flags = mblx2160a_gpios[i].flags;

	return dm_gpio_set_value(&desc, val);
}

int tqc_bb_board_eth_init(bd_t *bis)
{
#if defined(CONFIG_FSL_MC_ENET)
	struct mii_dev *dev;
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 srds_s1, srds_s2;
	int ret;

	srds_s1 = in_le32(&gur->rcwsr[28]) &
				FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_MASK;
	srds_s1 >>= FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_SHIFT;

	srds_s2 = in_le32(&gur->rcwsr[28]) &
				FSL_CHASSIS3_RCWSR28_SRDS2_PRTCL_MASK;
	srds_s2 >>= FSL_CHASSIS3_RCWSR28_SRDS2_PRTCL_SHIFT;

	printf("Configuring Ethernet for SerDes2: %d.\n", srds_s2);
	switch (srds_s2) {
	case 0:
		wriop_set_phy_address(WRIOP1_DPMAC17, 0, 0x3);
		wriop_set_phy_address(WRIOP1_DPMAC18, 0, 0x4);
		dev = miiphy_get_dev_by_name(DEFAULT_WRIOP_MDIO2_NAME);
		wriop_set_mdio(WRIOP1_DPMAC17, dev);
		wriop_set_mdio(WRIOP1_DPMAC18, dev);
		break;
	case 7:
		wriop_set_phy_address(WRIOP1_DPMAC12, 0, 0x1);
		wriop_set_phy_address(WRIOP1_DPMAC13, 0, 0xa);
		wriop_set_phy_address(WRIOP1_DPMAC14, 0, 0xb);
		wriop_set_phy_address(WRIOP1_DPMAC16, 0, 0x4);
		wriop_set_phy_address(WRIOP1_DPMAC17, 0, 0x2);
		wriop_set_phy_address(WRIOP1_DPMAC18, 0, 0x3);
		dev = miiphy_get_dev_by_name(DEFAULT_WRIOP_MDIO1_NAME);
		wriop_set_mdio(WRIOP1_DPMAC12, dev);
		wriop_set_mdio(WRIOP1_DPMAC16, dev);
		wriop_set_mdio(WRIOP1_DPMAC17, dev);
		wriop_set_mdio(WRIOP1_DPMAC18, dev);
		/* Intentional fallthrough */
	case 8:
		printf("mod detect 1: %d, 2: %d\n", mblx2160a_get_gpio("XFI1_MOD_DECTECT"), mblx2160a_get_gpio("XFI2_MOD_DECTECT"));

		if (mblx2160a_get_gpio("XFI1_MOD_DECTECT") == 0)
			ret = mblx2160a_set_gpio("XFI1_TX_DIS", 0);

		if (mblx2160a_get_gpio("XFI2_MOD_DECTECT") == 0)
			ret |= mblx2160a_set_gpio("XFI2_TX_DIS", 0);

		if (ret)
			printf("Error Setting XFI GPIOS: %d\n", ret);

		break;
	case 12:
		wriop_set_phy_address(WRIOP1_DPMAC17, 0, 0x2);
		wriop_set_phy_address(WRIOP1_DPMAC18, 0, 0x3);
		dev = miiphy_get_dev_by_name(DEFAULT_WRIOP_MDIO1_NAME);
		wriop_set_mdio(WRIOP1_DPMAC17, dev);
		wriop_set_mdio(WRIOP1_DPMAC18, dev);
		break;
	default:
		printf("SerDes2 Protocol %d not supported\n", srds_s2);
	}

	if (srds_s1 == 12) {
		wriop_set_phy_address(WRIOP1_DPMAC9, 0, 0x1);
		wriop_set_phy_address(WRIOP1_DPMAC10, 0, 0x2);
		dev = miiphy_get_dev_by_name(DEFAULT_WRIOP_MDIO2_NAME);
		wriop_set_mdio(WRIOP1_DPMAC9, dev);
		wriop_set_mdio(WRIOP1_DPMAC10, dev);
	}

#endif /* CONFIG_FSL_MC_ENET */

	return 0;
}

extern int phy_read_mmd_indirect(struct phy_device *phydev, int prtad, int devad, int addr);
extern void phy_write_mmd_indirect(struct phy_device *phydev, int prtad, int devad, int addr, u32 data);

int board_phy_config(struct phy_device *phydev)
{
	int val;

	printf("init phy on addr 0x%x\n", phydev->addr);
	val = phy_read_mmd_indirect(phydev, 0x32, 0x1f, phydev->addr);
	val |= 0x0003;
	phy_write_mmd_indirect(phydev, 0x32,  0x1f, phydev->addr, val);

	/* set RGMII delay in both directions to 1,5ns */
	val = phy_read_mmd_indirect(phydev, 0x86, 0x1f, phydev->addr);
	val = (val & 0xFF00) | 0x0055;
	phy_write_mmd_indirect(phydev, 0x86,  0x1f, phydev->addr, val);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

int fdt_fixup_board_phy(void *fdt)
{
	int ret;

	ret = 0;

	return ret;
}
