// SPDX-License-Identifier: GPL-2.0+
/*
 * Board specific initialization for MBA65XX
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


#include <command.h>
#include <console.h>
#include <mmc.h>
#include <sparse_format.h>
#include <image-sparse.h>

#include "../common/board_detect.h"

#define board_is_am65x_base_board()	board_ti_is("TQMA6548-P")
#define MAX_INTERFACES_CONFIG	4

/* Match data for SR1 vs SR2 dtb selection */
struct tqma65xx_rev_fdt_data {
	const char *findfdt_cmd_override;
	const char *fit_config_name;
};

static const struct tqma65xx_rev_fdt_data tqma65xx_fdt_data = {
	.findfdt_cmd_override = NULL,
#ifdef CONFIG_TQMA6548_MODULE_VERSION_P2
	.fit_config_name = "am654-mba65xx-p2",
#else
	.fit_config_name = "am654-mba65xx",
#endif
};

static const struct soc_device_attribute tqma65xx_rev_fdt_match[] = {
	{
		.family = "TQMA65X",
		.revision = "SR2.0",
		.data = &tqma65xx_fdt_data,
	},
	{ /* sentinel */ }
};

/* Max number of MAC addresses that are parsed/processed per daughter card */
#define DAUGHTER_CARD_NO_OF_MAC_ADDR	8

#define MAC_ARR_EEPROM_OFFSET     0x23
#define BOOT_DEVICE_EEPROM_OFFSET 0x53

#define GPIO_PRG_0_ETH 100
#define GPIO_PRG_1_ETH 101

#define PRG_0_ETHERNET_GPIO_NAME "gpio@20_2"
#define PRG_1_ETHERNET_GPIO_NAME "gpio@20_5"

#define PRG_0_ETHERNET_DTB_NAME "am654-mba65xx-eth-prg0.dtbo"
#define PRG_1_ETHERNET_DTB_NAME "am654-mba65xx-eth-prg1.dtbo"
#define AUDIO_DTB_NAME          "am654-mba65xx-audio.dtbo"
#define DISPLAY_DTB_NAME        "am654-mba65xx-lvds-display.dtbo"


DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}



int dram_init(void)
{
#ifdef CONFIG_PHYS_64BIT 
#ifdef CONFIG_TQMA6548_MODULE_VERSION_P2
	gd->ram_size = 0x80000000;
#else
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
#ifdef CONFIG_TQMA6548_MODULE_VERSION_P2
	gd->bd->bi_dram[1].size = 0;
	gd->ram_size = 0x80000000;
#else
	gd->bd->bi_dram[1].start = CONFIG_SYS_SDRAM_BASE1;
	gd->bd->bi_dram[1].size = 0x80000000;
	gd->ram_size = 0xFFFFFFF0;
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

const char *k3_dtbo_list[MAX_INTERFACES_CONFIG] = {NULL};

// Store globally  gpio value for read the stat from register only once
static int gpio_value_prg0_eth = -1;
static int gpio_value_prg1_eth = -1;

int do_board_detect(void)
{
	int ret;

	ret = ti_i2c_eeprom_am6_get_base(CONFIG_EEPROM_BUS_ADDRESS, CONFIG_EEPROM_CHIP_ADDRESS);

	return ret;
}

int checkboard(void)
{
	struct ti_am6_eeprom *ep = TI_AM6_EEPROM_DATA;

	if (do_board_detect())
		/* EEPROM not populated */
		printf("Board: %s rev %s\n", "MBa65xx", "SP.0100");
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

// Read value of gpio input from system
static int read_gpio_in_value(const char *gpio_name)
{
	int ret;
	struct gpio_desc gpio;

	memset(&gpio, 0, sizeof(gpio));

	ret = dm_gpio_lookup_name(gpio_name, &gpio);
	if (ret < 0) {
		pr_err("%s: gpio %s not found: %d\n", __func__, gpio_name, ret);
		return ret;
	}

	/* Request GPIO, simply re-using the name as label */
	ret = dm_gpio_request(&gpio, gpio_name);
	if (ret < 0) {
		pr_err("%s: gpio %s request error: %d\n", __func__, gpio_name, ret);
		return ret;
	}

	ret = dm_gpio_set_dir_flags(&gpio, GPIOD_IS_IN);
	if (ret < 0) {
		pr_err("%s: gpio %s set direction error: %d\n", __func__, gpio_name, ret);
		return ret;
	}

	ret = dm_gpio_get_value(&gpio);
	if (ret < 0) {
		pr_err("%s: gpio %s geting value error: %d\n", __func__, gpio_name, ret);
	}

	return ret;
}

// Get gpio value
// If value for this GPIO was already read - return value without reading
static int get_gpio_in_value(int gpio_id)
{
	int ret;

	if (gpio_id == GPIO_PRG_0_ETH) {
		// read gpio if it wasn't read
		if( gpio_value_prg0_eth < 0 ) {
			gpio_value_prg0_eth = read_gpio_in_value(PRG_0_ETHERNET_GPIO_NAME);
		}	
		ret = gpio_value_prg0_eth;

	} else if (gpio_id == GPIO_PRG_1_ETH) {
		// read gpio if it wasn't read
		if( gpio_value_prg1_eth < 0 ) {
			gpio_value_prg1_eth = read_gpio_in_value(PRG_1_ETHERNET_GPIO_NAME);
		}	
		ret = gpio_value_prg1_eth;

	} else {
		pr_err("%s: unknown gpio id %d\n", __func__, gpio_id);
		ret = -1;
	}

	return ret;
}


static int add_mba_interfaces(void)
{
	char name_overlays[1024] = { 0 };
	//int nb_dtbos = 0; // not needed for SPL
	int ret;

	ret = 0;
	memset(k3_dtbo_list, 0, sizeof(k3_dtbo_list));

	// Check PRG_Etherenet_0
	ret = get_gpio_in_value(GPIO_PRG_0_ETH);
	if (ret < 0) {
		pr_err("%s: get gpio %s value error: %d\n", 
			__func__, PRG_0_ETHERNET_GPIO_NAME, ret);
	} else if (ret == 1) {
		// Add dtbo for prg0 ethernet
		//k3_dtbo_list[nb_dtbos++] = PRG_0_ETHERNET_DTB_NAME; // not needed for SPL
		strcat(name_overlays, PRG_0_ETHERNET_DTB_NAME);
		strcat(name_overlays, " ");
	} else {
		// Add dtbo for audio
		//k3_dtbo_list[nb_dtbos++] = AUDIO_DTB_NAME; // not needed for SPL
		strcat(name_overlays, AUDIO_DTB_NAME);
		strcat(name_overlays, " ");
	} 

	// Check PRG_Etherenet_1
	ret = get_gpio_in_value(GPIO_PRG_1_ETH);
	if (ret < 0) {
		pr_err("%s: get gpio %s value error: %d\n", 
			__func__, PRG_1_ETHERNET_GPIO_NAME, ret);
	} else if (ret == 0) {
		// Add dtbo for prg1 ethernet
		//k3_dtbo_list[nb_dtbos++] = PRG_1_ETHERNET_DTB_NAME; // not needed for SPL
		strcat(name_overlays, PRG_1_ETHERNET_DTB_NAME);
		strcat(name_overlays, " ");
	}

	// Add display
	//k3_dtbo_list[nb_dtbos++] = DISPLAY_DTB_NAME; // not needed for SPL
	strcat(name_overlays, DISPLAY_DTB_NAME);
	strcat(name_overlays, " ");

	#ifndef CONFIG_SPL_BUILD
	/* Apply device tree overlay(s) to the U-Boot environment, if any */
	if (strlen(name_overlays))
		return env_set("name_overlays", name_overlays);
#endif
	return 0;
}

// Generation default values for ethernet
static void get_def_mac(int index, u8 *mac_addr)
{
	int i;

	if (index < 0 || index > AM6_EEPROM_HDR_NO_OF_MAC_ADDR) {
		for (i = 0; i < TI_EEPROM_HDR_ETH_ALEN; ++i) {
			mac_addr[i] = 0x0;
		}
	} else {
			mac_addr[0] = 0xAA;
		for (i = 1; i < TI_EEPROM_HDR_ETH_ALEN; ++i) {
			mac_addr[i] = 15 * (index + 1) + index + 1;
		}
	}
}

// Read MAC array from eeprom
static int read_mac_arr(int bus_addr, int dev_addr,	u32 size, 
	uint8_t mac_arr[AM6_EEPROM_HDR_NO_OF_MAC_ADDR][TI_EEPROM_HDR_ETH_ALEN])
{
	int ret;

	struct udevice *dev;
	struct udevice *bus;

	ret = uclass_get_device_by_seq(UCLASS_I2C, bus_addr, &bus);
	if (ret)
		return ret;

	ret = dm_i2c_probe(bus, dev_addr, 0, &dev);
	if (ret)
		return ret;

	ret = i2c_set_chip_offset_len(dev, 2);
	if (ret)
		return ret;

	ret = dm_i2c_read(dev, MAC_ARR_EEPROM_OFFSET, &mac_arr[0][0], size);

	return ret;
}

// Save spl boot device to eeprom
static int save_spl_boot_device_eeprom(u32 boot_dev)
{
	const int bus_addr = 0x00;
	const int dev_addr = 0x50;
	
	int ret;

	struct udevice *dev;
	struct udevice *bus;

	u32 buffer = boot_dev;

	ret = uclass_get_device_by_seq(UCLASS_I2C, bus_addr, &bus);
	if (ret)
		return ret;

	ret = dm_i2c_probe(bus, dev_addr, 0, &dev);
	if (ret)
		return ret;

	ret = i2c_set_chip_offset_len(dev, 2);
	if (ret)
		return ret;

	ret = dm_i2c_write(dev, BOOT_DEVICE_EEPROM_OFFSET, (const uint8_t *)&buffer, sizeof(buffer));

	return ret;
}


// Save spl boot device to eeprom
static int read_spl_boot_device_eeprom(u32 *boot_dev_u32)
{
	const int bus_addr = 0x00;
	const int dev_addr = 0x50;

	int ret;

	struct udevice *dev;
	struct udevice *bus;

	if (!boot_dev_u32)
		return 1;

	ret = uclass_get_device_by_seq(UCLASS_I2C, bus_addr, &bus);
	if (ret)
		return ret;

	ret = dm_i2c_probe(bus, dev_addr, 0, &dev);
	if (ret)
		return ret;

	ret = i2c_set_chip_offset_len(dev, 2);
	if (ret)
		return ret;

	ret = dm_i2c_read(dev, BOOT_DEVICE_EEPROM_OFFSET, (uint8_t *)boot_dev_u32, sizeof(u32));

	return ret;
}

static void setup_boot_linux_env(void)
{
	u32 boot_dev ;

	if(read_spl_boot_device_eeprom(&boot_dev))
		pr_err("%s: error reading boot device to EEPROM", __func__);

	if (boot_dev == BOOT_DEVICE_MMC1) {
		env_set("bootpart", "0:2");
		env_set("mmcdev", "0");
	} else if (boot_dev == BOOT_DEVICE_MMC2) {
		env_set("bootpart", "1:2");
		env_set("mmcdev", "1");
	}
}

static void set_prg_eth(int index, 
	uint8_t mac_addr_arr[AM6_EEPROM_HDR_NO_OF_MAC_ADDR][TI_EEPROM_HDR_ETH_ALEN])
{
	int i;
	uint8_t *mac_ptr;
	
	for (i = 0; i < 2; ++i) {			
		mac_ptr = &mac_addr_arr[index + i][0];

		if (!is_valid_ethaddr(mac_ptr)) {
			get_def_mac(index + i, mac_ptr);
		}

		eth_env_set_enetaddr_by_index("eth", i + index + 1, mac_ptr);
	}
}

static int setup_board_prg_eth(void) 
{
	const int i2c_bus = 0x00;
	const int i2c_addr = 0x50;

	uint8_t mac_addr_arr[AM6_EEPROM_HDR_NO_OF_MAC_ADDR][TI_EEPROM_HDR_ETH_ALEN];
	int ret;

	memset(&mac_addr_arr, 0xFF, AM6_EEPROM_HDR_NO_OF_MAC_ADDR * TI_EEPROM_HDR_ETH_ALEN);

	// Get MAC addresses array
	ret = read_mac_arr(i2c_bus, i2c_addr, 
		AM6_EEPROM_HDR_NO_OF_MAC_ADDR * TI_EEPROM_HDR_ETH_ALEN, mac_addr_arr);
		
	if (ret) {
		pr_err("%s: error read MAC addresses from EEPROM bus_addr=0x%02x dev_addr=0x%02x: %d\n",
			   __func__, i2c_bus, i2c_addr, ret);
	}

	// Check PRG_Etherenet_0
	ret = get_gpio_in_value(GPIO_PRG_0_ETH);
	if (ret < 0) {
		pr_err("%s: get gpio %s value error: %d\n", 
			__func__, PRG_0_ETHERNET_GPIO_NAME, ret);
		return ret;
	}

	if (ret == 1) {
		// Set MAC addresses foe ethernet1, ethernet2
		set_prg_eth(0, mac_addr_arr);
	}

	// Check PRG_Etherenet_1
	ret = get_gpio_in_value(GPIO_PRG_1_ETH);
	if (ret < 0) {
		pr_err("%s: get gpio %s value error: %d\n", 
			__func__, PRG_1_ETHERNET_GPIO_NAME, ret);
		return ret;
	}

	if (ret == 0) {
		// Set MAC addresses foe ethernet3, ethernet4
		set_prg_eth(2, mac_addr_arr);
	}

	// Set MAC addresses foe ethernet5, ethernet6
	set_prg_eth(4, mac_addr_arr);

	return 0;
}

static void setup_board_clock_synthesizer(void)
{
//config array for clock synthesizer
/*
out 0-1 100.0 MHz
out 2-3 disable
out 4, 5, 6, 7 = 25.0 MHz CMOS
*/
const uint32_t clock_config[334]={
//config from TICS Pro
//100 100 cml nc nc 25 25 25 25 (4x cmos +/ hiz -)
0x000010	,
0x00010B	,
0x000235	,
0x000300	,
0x000400	,
0x000500	,
0x000600	,
0x000700	,
0x000802	,
0x000AC8	,
0x000B00	,
0x000C3B	,
0x000D08	,
0x000E00	,
0x000F00	,
0x001000	,
0x00111D	,
0x0012FF	,
0x001300	,
0x001400	,
0x001500	,
0x001600	,
0x001755	,
0x001855	,
0x001900	,
0x001A00	,
0x001B00	,
0x001C01	,
0x001D11	,
0x001E40	,
0x002044	,
0x002300	,
0x002403	,
0x002500	,
0x002600	,
0x002702	,
0x002800	,
0x002900	,
0x002A11	,
0x002B82	,
0x002C01	,
0x002D03	,
0x002E11	,
0x002F07	,
0x003050	,
0x00314A	,
0x003200	,
0x003314	,
0x003414	,
0x003518	,
0x003600	,
0x003700	,
0x003818	,
0x00393C	,
0x003A63	,
0x003B3C	,
0x003C63	,
0x003D3C	,
0x003E63	,
0x003F3C	,
0x004000	,
0x004100	,
0x004200	,
0x004363	,
0x004408	,
0x004500	,
0x004600	,
0x004700	,
0x00483F	,
0x004900	,
0x004A00	,
0x004B00	,
0x004C00	,
0x004D0F	,
0x004E00	,
0x004F11	,
0x005080	,
0x00510A	,
0x005200	,
0x00530E	,
0x005466	,
0x005567	,
0x005600	,
0x00571E	,
0x005884	,
0x005981	,
0x005A00	,
0x005B14	,
0x005C00	,
0x005D0E	,
0x005E66	,
0x005F67	,
0x006000	,
0x00611E	,
0x006284	,
0x006381	,
0x006429	,
0x006503	,
0x006622	,
0x00670F	,
0x006818	,
0x006905	,
0x006A00	,
0x006B64	,
0x006C00	,
0x006D65	,
0x006EB9	,
0x006FAA	,
0x0070AA	,
0x0071AA	,
0x0072AB	,
0x007303	,
0x007401	,
0x007500	,
0x007600	,
0x007700	,
0x007800	,
0x007900	,
0x007A00	,
0x007B28	,
0x007C09	,
0x007DE5	,
0x007E35	,
0x007F2E	,
0x008000	,
0x008106	,
0x008200	,
0x008301	,
0x008401	,
0x008577	,
0x008600	,
0x008728	,
0x00884F	,
0x00898F	,
0x008A8A	,
0x008B03	,
0x008C02	,
0x008D00	,
0x008E01	,
0x008F01	,
0x009077	,
0x009101	,
0x009281	,
0x009320	,
0x00950D	,
0x009600	,
0x009701	,
0x00980D	,
0x009929	,
0x009A24	,
0x009B8B	,
0x009C01	,
0x009D00	,
0x009E00	,
0x009F00	,
0x00A0FC	,
0x00A100	,
0x00A200	,
0x00A400	,
0x00A500	,
0x00A701	,
0x00B200	,
0x00B400	,
0x00B500	,
0x00B600	,
0x00B700	,
0x00B800	,
0x00B905	,
0x00BA01	,
0x00BB00	,
0x00BC00	,
0x00BD00	,
0x00BE01	,
0x00BF00	,
0x00C050	,
0x00C12B	,
0x00C22B	,
0x00C300	,
0x00C400	,
0x00C51C	,
0x00C600	,
0x00C700	,
0x00C81C	,
0x00C900	,
0x00CA00	,
0x00CB00	,
0x00CC16	,
0x00CD00	,
0x00CE00	,
0x00CF16	,
0x00D000	,
0x00D108	,
0x00D200	,
0x00D30A	,
0x00D400	,
0x00D508	,
0x00D600	,
0x00D70A	,
0x00D80F	,
0x00D900	,
0x00DA00	,
0x00DB31	,
0x00DCAC	,
0x00DD00	,
0x00DE06	,
0x00DF1A	,
0x00E08B	,
0x00E100	,
0x00E200	,
0x00E331	,
0x00E4AC	,
0x00E500	,
0x00E606	,
0x00E71A	,
0x00E88B	,
0x00E90A	,
0x00EA0A	,
0x00EB00	,
0x00EC00	,
0x00ED00	,
0x00EE01	,
0x00EF00	,
0x00F000	,
0x00F100	,
0x00F201	,
0x00F300	,
0x00F400	,
0x00F921	,
0x00FA00	,
0x00FB22	,
0x00FC2D	,
0x00FD00	,
0x00FE00	,
0x00FF00	,
0x010000	,
0x01013F	,
0x010200	,
0x01033F	,
0x010402	,
0x010580	,
0x010600	,
0x010700	,
0x010800	,
0x010931	,
0x010A38	,
0x010BA0	,
0x010C0C	,
0x010D00	,
0x010E02	,
0x010FBB	,
0x011000	,
0x011100	,
0x011200	,
0x011310	,
0x01140E	,
0x011510	,
0x011607	,
0x011708	,
0x011807	,
0x011903	,
0x011A09	,
0x011B03	,
0x011C1E	,
0x011D1E	,
0x011E03	,
0x011F6D	,
0x012002	,
0x01216A	,
0x012203	,
0x0123E7	,
0x012409	,
0x012501	,
0x012600	,
0x01272C	,
0x012802	,
0x012906	,
0x012A02	,
0x012B01	,
0x012C00	,
0x012D1A	,
0x012E1F	,
0x012F00	,
0x013002	,
0x013100	,
0x013200	,
0x013303	,
0x013413	,
0x013580	,
0x013600	,
0x013700	,
0x013800	,
0x013900	,
0x013A00	,
0x013B00	,
0x013C00	,
0x013D00	,
0x013E00	,
0x013F03	,
0x014000	,
0x01410A	,
0x014200	,
0x014300	,
0x014494	,
0x0145D0	,
0x014600	,
0x014798	,
0x014897	,
0x014948	,
0x014A00	,
0x014B64	,
0x014C00	,
0x014D00	,
0x014E94	,
0x014FD0	,
0x015000	,
0x015198	,
0x015297	,
0x015348	,
0x015400	,
0x015500	,
0x015600	,
0x015700	,
0x015800	,
0x015900	,
0x015A00	,
0x015B00	,
0x015C00	,
0x015D00	,
0x015E00	,
0x015FB7	,
0x016000	,
0x016528	,
0x016F00	,
0x019B04	
};
	struct dm_i2c_chip *chip;
	struct i2c_msg msg;
	struct udevice *i2c_clock_synth = NULL;
	const int i2c_bus = 0x01;
	const int CLK_SYNTHESIZER_I2C_ADDR = 0x64;
	uint8_t buf[3];
	uint16_t reg_addr;
	int reg_index;
	int tmp = 0;

	tmp = i2c_get_chip_for_busnum(i2c_bus, CLK_SYNTHESIZER_I2C_ADDR, 1, &i2c_clock_synth);
	if (tmp) {
		printf("Failed to get device for synthesizer at address 0x%x\n", CLK_SYNTHESIZER_I2C_ADDR);
	}else{
			printf("Start config for clock synthesizer at address 0x%x\n", CLK_SYNTHESIZER_I2C_ADDR);
			chip = dev_get_parent_platdata(i2c_clock_synth);
			msg.addr = chip->chip_addr;
			msg.flags = 0;
				for (reg_index=0; reg_index<333; reg_index++){
				reg_addr= (clock_config[reg_index]>>8) & 0xFFFF;
				buf[0] = (reg_addr>>8) & 0xFF;
				buf[1] = reg_addr & 0xFF;
				buf[2] = clock_config[reg_index] & 0xFF;
				msg.buf = buf;
				msg.len = 3;
				dm_i2c_xfer(i2c_clock_synth, &msg, 1);
				}
			printf("Done!\n");
		}	
}


int board_late_init(void)
{
	const struct soc_device_attribute *match;
	const struct tqma65xx_rev_fdt_data *fdt_data;

	setup_board_eeprom_env();

	setup_board_prg_eth();

	// Configure u-boot env accroding spl boot source
	setup_boot_linux_env();

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
	add_mba_interfaces();
	setup_board_clock_synthesizer();

	return 0;
}



void spl_board_init(void)
{
	struct udevice *board;

	// Save spl boot device to eeprom to allow read boot device from u-boot
	if (save_spl_boot_device_eeprom(spl_boot_device()))
		pr_err("%s: error saving boot device to EEPROM", __func__);

	/* Check for and probe any plugged-in daughtercards */
	add_mba_interfaces();

	/* Probe the board driver and call its detection method*/
	if (!board_get(&board))
		board_detect(board);
}
