// SPDX-License-Identifier: GPL-2.0+
/*
 * Board specific initialization for AM654 EVM
 *
 * Copyright (C) 2017-2018 Texas Instruments Incorporated - http://www.ti.com/
 * Copyright (C) 2020 TQ Systems
 *
 */

#include <common.h>
#include <dm.h>
#include <init.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/hardware.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/omap_common.h>
#include <env.h>
#include <spl.h>
#include <board.h>
#include <soc.h>
#include <asm/arch/sys_proto.h>
#include <i2c.h>


#include "../common/board_detect.h"

#define board_is_am65x_base_board()	board_ti_is("AM6-COMPROCEVM")
#define MAX_DAUGHTER_CARDS	8

/* Match data for SR1 vs SR2 dtb selection */
struct tqma65xx_rev_fdt_data {
	const char *findfdt_cmd_override;
	const char *fit_config_name;
};

static const struct tqma65xx_rev_fdt_data tqma65xx_fdt_data = {
	.findfdt_cmd_override = NULL,
	.fit_config_name = "tqma654-base-board",
};

static const struct soc_device_attribute tqma65xx_rev_fdt_match[] = {
	{
		.family = "TQMA65X",
		.revision = "SR2.0",
		.data = &tqma65xx_fdt_data,
	},
	{ /* sentinel */ }
};
/* Daughter card presence detection signals */
enum {
	TQMA65XX_MBA65XX_APP_BRD_DET,
	TQMA65XX_MBA65XX_LCD_BRD_DET,
	TQMA65XX_MBA65XX_SERDES_BRD_DET,
	TQMA65XX_MBA65XX_HDMI_GPMC_BRD_DET,
	TQMA65XX_MBA65XX_BRD_DET_COUNT,
};

/* Max number of MAC addresses that are parsed/processed per daughter card */
#define DAUGHTER_CARD_NO_OF_MAC_ADDR	8

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}



int dram_init(void)
{
#ifdef CONFIG_PHYS_64BIT
#ifdef CONFIG_SYS_SDRAM_SIZE == SIZE_2GB
	gd->ram_size = 0x80000000;
#elif CONFIG_SYS_SDRAM_SIZE == SIZE_4GB
	gd->ram_size = 0x100000000;
#elif
	gd->ram_size = 0x100000000;
#endif

#else
	gd->ram_size = 0x80000000;
#endif

	return 0;
}


ulong board_get_usable_ram_top(ulong total_size)
{
#ifdef CONFIG_PHYS_64BIT
	/* Limit RAM used by U-Boot to the DDR low region */
	if (gd->ram_top > 0x100000000)
		return 0x100000000;
#endif

	return gd->ram_top;
}

int dram_init_banksize(void)
{
	/* Bank 0 declares the memory available in the DDR low region */
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = 0x80000000;
	gd->ram_size = 0x80000000;

#ifdef CONFIG_PHYS_64BIT
	/* Bank 1 declares the memory available in the DDR high region */
	gd->bd->bi_dram[1].start = CONFIG_SYS_SDRAM_BASE1;
#ifdef CONFIG_SYS_SDRAM_SIZE == SIZE_2GB
	gd->bd->bi_dram[1].size = 0;
	gd->ram_size = 0x80000000;
#elif CONFIG_SYS_SDRAM_SIZE == SIZE_4GB
	gd->bd->bi_dram[1].size = 0x80000000;
	gd->ram_size = 0x100000000;
#else
	gd->bd->bi_dram[1].size = 0x80000000;
	gd->ram_size = 0x100000000;
#endif
#endif


	return 0;
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
#ifdef CONFIG_TARGET_TQMA65XX_MBA65XX_A53
	const struct soc_device_attribute *match;
	const struct tqma65xx_rev_fdt_data *fdt_data;

	match = soc_device_match(tqma65xx_rev_fdt_match);
	if (!match) {
		/* Default to SR2.0 */
		match = &tqma65xx_rev_fdt_match[0];
	}

	fdt_data = match->data;

	if (!strcmp(name, fdt_data->fit_config_name))
		return 0;
#endif

	return -1;
}
#endif

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	int ret;

	ret = fdt_fixup_msmc_ram(blob, "/interconnect@100000", "sram@70000000");
	if (ret) {
		printf("%s: fixing up msmc ram failed %d\n", __func__, ret);
		return ret;
	}

#if defined(CONFIG_TI_SECURE_DEVICE)
	/* Make HW RNG reserved for secure world use */
	ret = fdt_disable_node(blob, "/interconnect@100000/trng@4e10000");
	if (ret)
		printf("%s: disabling TRGN failed %d\n", __func__, ret);
#endif

	return 0;
}
#endif

const char *k3_dtbo_list[MAX_DAUGHTER_CARDS] = {NULL};

int do_board_detect(void)
{
	int ret;

	ret = ti_i2c_eeprom_am6_get_base(CONFIG_EEPROM_BUS_ADDRESS,
					 CONFIG_EEPROM_CHIP_ADDRESS);
	if (ret)
		pr_err("Reading on-board EEPROM at 0x%02x failed %d\n",
		       CONFIG_EEPROM_CHIP_ADDRESS, ret);

	return ret;
}

int checkboard(void)
{
	struct ti_am6_eeprom *ep = TI_AM6_EEPROM_DATA;

	if (do_board_detect())
		/* EEPROM not populated */
		printf("Board: %s rev %s\n", "AM6-COMPROCEVM", "E3");
	else
		printf("Board: %s rev %s\n", ep->name, ep->version);

	return 0;
}

static void setup_board_eeprom_env(void)
{
	char *name = "tqma65xx";

	if (do_board_detect())
		goto invalid_eeprom;

	if (board_is_am65x_base_board())
		name = "tqma65xx";
	else
		printf("Unidentified board claims %s in eeprom header\n",
		       board_ti_get_name());

invalid_eeprom:
	set_board_info_env_am6(name);
}

static int init_daughtercard_det_gpio(char *gpio_name, struct gpio_desc *desc)
{
	int ret;

	memset(desc, 0, sizeof(*desc));	

	ret = dm_gpio_lookup_name(gpio_name, desc);
	if (ret < 0)
		return ret;

	/* Request GPIO, simply re-using the name as label */
	ret = dm_gpio_request(desc, gpio_name);
	if (ret < 0)
		return ret;

	return dm_gpio_set_dir_flags(desc, GPIOD_IS_IN);
}

static int probe_daughtercards(void)
{
	struct ti_am6_eeprom ep;
	struct gpio_desc board_det_gpios[TQMA65XX_MBA65XX_BRD_DET_COUNT];
	char mac_addr[DAUGHTER_CARD_NO_OF_MAC_ADDR][TI_EEPROM_HDR_ETH_ALEN];
	u8 mac_addr_cnt;
	char name_overlays[1024] = { 0 };
	int i, nb_dtbos = 0;
	int ret;
	struct udevice *soc;
	char str[SOC_MAX_STR_SIZE];

	/*
	 * Daughter card presence detection signal name to GPIO (via I2C I/O
	 * expander @ address 0x38) name and EEPROM I2C address mapping.
	 */
	const struct {
		char *gpio_name;
		u8 i2c_addr;
	} slot_map[TQMA65XX_MBA65XX_BRD_DET_COUNT] = {
		{ "gpio@38_0", 0x52, },	/* TQMA65XX_MBA65XX_APP_BRD_DET */
		{ "gpio@38_1", 0x55, },	/* TQMA65XX_MBA65XX_LCD_BRD_DET */
		{ "gpio@38_2", 0x54, },	/* TQMA65XX_MBA65XX_SERDES_BRD_DET */
		{ "gpio@38_3", 0x53, },	/* TQMA65XX_MBA65XX_HDMI_GPMC_BRD_DET */
	};

	/* Declaration of daughtercards to probe */
	const struct {
		u8 slot_index;		/* Slot the card is installed */
		char *card_name;	/* EEPROM-programmed card name */
		char *dtbo_name;	/* Device tree overlay to apply */
		u8 eth_offset;		/* ethXaddr MAC address index offset */
	} cards[] = {
		{
			TQMA65XX_MBA65XX_APP_BRD_DET,
			"AM6-GPAPPEVM",
			"tqma654-gp.dtbo",
			0,
		},
		{
			TQMA65XX_MBA65XX_APP_BRD_DET,
			"AM6-IDKAPPEVM",
			"tqma654-idk.dtbo",
			3,
		},
		{
			TQMA65XX_MBA65XX_SERDES_BRD_DET,
			"SER-PCIE2LEVM",
			"tqma654-pcie-usb2.dtbo",
			0,
		},
		{
			TQMA65XX_MBA65XX_SERDES_BRD_DET,
			"SER-PCIEUSBEVM",
			"tqma654-pcie-usb3.dtbo",
			0,
		},
		{
			TQMA65XX_MBA65XX_LCD_BRD_DET,
			"OLDI-LCD1EVM",
			"tqma654-evm-oldi-lcd1evm.dtbo",
			0,
		},
	};

	/*
	 * Initialize GPIO used for daughtercard slot presence detection and
	 * keep the resulting handles in local array for easier access.
	 */
	for (i = 0; i < TQMA65XX_MBA65XX_BRD_DET_COUNT; i++) {
		ret = init_daughtercard_det_gpio(slot_map[i].gpio_name,
						 &board_det_gpios[i]);
		if (ret < 0)
			return ret;
	}

	memset(k3_dtbo_list, 0, sizeof(k3_dtbo_list));
	for (i = 0; i < ARRAY_SIZE(cards); i++) {
		/* Obtain card-specific slot index and associated I2C address */
		u8 slot_index = cards[i].slot_index;
		u8 i2c_addr = slot_map[slot_index].i2c_addr;
		const char *dtboname;

		/*
		 * The presence detection signal is active-low, hence skip
		 * over this card slot if anything other than 0 is returned.
		 */
		ret = dm_gpio_get_value(&board_det_gpios[slot_index]);
		if (ret < 0)
			return ret;
		else if (ret)
			continue;

		/* Get and parse the daughter card EEPROM record */
		ret = ti_i2c_eeprom_am6_get(CONFIG_EEPROM_BUS_ADDRESS, i2c_addr,
					    &ep,
					    (char **)mac_addr,
					    DAUGHTER_CARD_NO_OF_MAC_ADDR,
					    &mac_addr_cnt);
		if (ret) {
			pr_err("Reading daughtercard EEPROM at 0x%02x failed %d\n",
			       i2c_addr, ret);
			/*
			 * Even this is pretty serious let's just skip over
			 * this particular daughtercard, rather than ending
			 * the probing process altogether.
			 */
			continue;
		}

		/* Only process the parsed data if we found a match */
		if (strncmp(ep.name, cards[i].card_name, sizeof(ep.name)))
			continue;

		printf("Detected: %s rev %s\n", ep.name, ep.version);

#ifndef CONFIG_SPL_BUILD
		int j;

		/*
		 * Populate any MAC addresses from daughtercard into the U-Boot
		 * environment, starting with a card-specific offset so we can
		 * have multiple cards contribute to the MAC pool in a well-
		 * defined manner.
		 */
		for (j = 0; j < mac_addr_cnt; j++) {
			if (!is_valid_ethaddr((u8 *)mac_addr[j]))
				continue;

			eth_env_set_enetaddr_by_index("eth",
						      cards[i].eth_offset + j,
						      (uchar *)mac_addr[j]);
		}
#endif

		/* Skip if no overlays are to be added */
		if (!strlen(cards[i].dtbo_name))
			continue;

		dtboname = cards[i].dtbo_name;

		if (strncmp(ep.name, "AM6-IDKAPPEVM", sizeof(ep.name)))
			goto dflt;

		if (soc_get(&soc))
			goto dflt;

		if (soc_get_revision(soc, str, sizeof(str)))
			goto dflt;

dflt:
		k3_dtbo_list[nb_dtbos++] = dtboname;

		/*
		 * Make sure we are not running out of buffer space by checking
		 * if we can fit the new overlay, a trailing space to be used
		 * as a separator, plus the terminating zero.
		 */
		if (strlen(name_overlays) + strlen(dtboname) + 2 >
		    sizeof(name_overlays))
			return -ENOMEM;

		/* Append to our list of overlays */
		strcat(name_overlays, dtboname);
		strcat(name_overlays, " ");
	}

#ifndef CONFIG_SPL_BUILD
	/* Apply device tree overlay(s) to the U-Boot environment, if any */
	if (strlen(name_overlays))
		return env_set("name_overlays", name_overlays);
#endif

	return 0;
}

int board_late_init(void)
{
	struct ti_am6_eeprom *ep = TI_AM6_EEPROM_DATA;
	const struct soc_device_attribute *match;
	const struct tqma65xx_rev_fdt_data *fdt_data;

	setup_board_eeprom_env();

	/*
	 * The first MAC address for ethernet a.k.a. ethernet0 comes from
	 * efuse populated via the tqma654 gigabit eth switch subsystem driver.
	 * All the other ones are populated via EEPROM, hence continue with
	 * an index of 1.
	 */
	board_ti_am6_set_ethaddr(1, ep->mac_addr_cnt);

	/* If we are on SR1 silicon set env to use sr1 dtb for kernel */
	match = soc_device_match(tqma65xx_rev_fdt_match);
	if (!match) {
		/* Default to SR2.0 */
		match = &tqma65xx_rev_fdt_match[0];
	}

	fdt_data = match->data;

	if (fdt_data->findfdt_cmd_override)
		env_set("findfdt", fdt_data->findfdt_cmd_override);

	/* Check for and probe any plugged-in daughtercards */
	probe_daughtercards();

	return 0;
}

void spl_board_init(void)
{
	struct udevice *board;

	/* Check for and probe any plugged-in daughtercards */
	probe_daughtercards();

	/* Probe the board driver and call its detection method*/
	if (!board_get(&board))
		board_detect(board);
}
