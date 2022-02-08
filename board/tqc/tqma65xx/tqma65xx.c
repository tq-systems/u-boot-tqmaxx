// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Module initialization for TQ-Systems TQMa65xx
 *
 * Copyright (C) 2017-2018 Texas Instruments Incorporated - http://www.ti.com/
 * Copyright (C) 2020-2022 TQ-Systems GmbH
 *
 */

#include <common.h>
#include <init.h>
#include <image.h>
#include <fdt_support.h>
#include <jffs2/load_kernel.h>
#include <mtd_node.h>
#include <spi_flash.h>
#include <spl.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>

#include "tqma65xx.h"
#include "../common/eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
#ifdef CONFIG_PHYS_64BIT
#  if defined(CONFIG_TQMA65XX_MODULE_VERSION_2G)
	gd->ram_size = 0x80000000;
#  elif defined(CONFIG_TQMA65XX_MODULE_VERSION_4G)
	gd->ram_size = 0x100000000;
#  else
#    error Unknown TQMa65xx variant
#  endif
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
	if (IS_ENABLED(CONFIG_SPL_BUILD))
		dram_init();

	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = 0x80000000;

#ifdef CONFIG_PHYS_64BIT
	if (gd->ram_size > 0x80000000) {
		gd->bd->bi_dram[1].start = CONFIG_SYS_SDRAM_BASE1;
		gd->bd->bi_dram[1].size = gd->ram_size - 0x80000000;
	}
#endif

	return 0;
}

#ifdef CONFIG_SPL_BUILD

int board_fit_config_name_match(const char *name)
{
	return 0;
}

int do_board_detect(void)
{
	return 0;
}

#else /* CONFIG_SPL_BUILD */

int board_init(void)
{
	return 0;
}

/* Bootmedia handling, copied from SPL code in am6_init.c */
static u32 get_backup_bootmedia(u32 devstat)
{
	u32 bkup_boot = (devstat & CTRLMMR_MAIN_DEVSTAT_BKUP_BOOTMODE_MASK) >>
			CTRLMMR_MAIN_DEVSTAT_BKUP_BOOTMODE_SHIFT;

	switch (bkup_boot) {
	case BACKUP_BOOT_DEVICE_USB:
		return BOOT_DEVICE_USB;
	case BACKUP_BOOT_DEVICE_UART:
		return BOOT_DEVICE_UART;
	case BACKUP_BOOT_DEVICE_ETHERNET:
		return BOOT_DEVICE_ETHERNET;
	case BACKUP_BOOT_DEVICE_MMC2:
	{
		u32 port =
			(devstat & CTRLMMR_MAIN_DEVSTAT_BKUP_MMC_PORT_MASK) >>
			CTRLMMR_MAIN_DEVSTAT_BKUP_MMC_PORT_SHIFT;
		if (port == 0x0)
			return BOOT_DEVICE_MMC1;
		return BOOT_DEVICE_MMC2;
	}
	case BACKUP_BOOT_DEVICE_SPI:
		return BOOT_DEVICE_SPI;
	case BACKUP_BOOT_DEVICE_HYPERFLASH:
		return BOOT_DEVICE_HYPERFLASH;
	case BACKUP_BOOT_DEVICE_I2C:
		return BOOT_DEVICE_I2C;
	};

	return BOOT_DEVICE_RAM;
}

static u32 get_primary_bootmedia(u32 devstat)
{
	u32 bootmode = (devstat & CTRLMMR_MAIN_DEVSTAT_BOOTMODE_MASK) >>
		CTRLMMR_MAIN_DEVSTAT_BOOTMODE_SHIFT;

	if (bootmode == BOOT_DEVICE_OSPI || bootmode ==	BOOT_DEVICE_QSPI)
		bootmode = BOOT_DEVICE_SPI;

	if (bootmode == BOOT_DEVICE_MMC2) {
		u32 port = (devstat & CTRLMMR_MAIN_DEVSTAT_MMC_PORT_MASK) >>
			CTRLMMR_MAIN_DEVSTAT_MMC_PORT_SHIFT;
		if (port == 0x0)
			bootmode = BOOT_DEVICE_MMC1;
	} else if (bootmode == BOOT_DEVICE_MMC1) {
		u32 port = (devstat & CTRLMMR_MAIN_DEVSTAT_EMMC_PORT_MASK) >>
			CTRLMMR_MAIN_DEVSTAT_EMMC_PORT_SHIFT;
		if (port == 0x1)
			bootmode = BOOT_DEVICE_MMC2;
	}

	return bootmode;
}

u32 tqma65xx_get_boot_device(void)
{
	u32 bootindex = *(u32 *)(CONFIG_SYS_K3_BOOT_PARAM_TABLE_INDEX);
	u32 devstat = readl(CTRLMMR_MAIN_DEVSTAT);

	if (bootindex == K3_PRIMARY_BOOTMODE)
		return get_primary_bootmedia(devstat);
	else
		return get_backup_bootmedia(devstat);
}

static void tqma65xx_set_macaddrs(u8 *macaddr)
{
	int i;

	for (i = 1; ; i++) {
		eth_env_set_enetaddr_by_index("eth", i, macaddr);
		if (i >= 6)
			break;

		if (++macaddr[5])
			continue;
		if (++macaddr[4])
			continue;
		if (++macaddr[3])
			continue;

		printf("Warning: MAC address wrapped around to %pM\n", macaddr);
	}
}

static const char *tqma65xx_cpu_type(void)
{
	u32 device_id = readl(CTRLMMR_WKUP_JTAG_DEVICE_ID);
	switch (device_id >> 15) {
	case 0x1413:
		return "6526";
	case 0x1417:
		return "6546";
	case 0x140B:
		return "6528";
	case 0x140F:
		return "6548";
	default:
		return "65??";
	}
}

void tqma65xx_parse_eeprom(void)
{
	char id[TQ_ID_STRLEN], serial[TQ_SERIAL_STRLEN], id_prefix[9];
	struct tq_eeprom_data data;
	u8 macaddr[6];
	int ret;

	snprintf(id_prefix, sizeof(id_prefix), "TQMA%s", tqma65xx_cpu_type());

	ret = tq_read_eeprom(0, &data);
	if (ret) {
		printf("EEPROM: err %d\n", ret);
		return;
	}

	tq_show_eeprom(&data, id_prefix);

	if (tq_get_eeprom_id(&data, id))
		env_set_runtime("boardtype", id);

	if (tq_get_eeprom_serial(&data, serial))
		env_set_runtime("serial#", serial);

	if (tq_get_eeprom_mac(&data, macaddr))
		tqma65xx_set_macaddrs(macaddr);
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int tqma65xx_ft_board_setup(void *blob, bd_t *bd)
{
#if defined(CONFIG_DM_SPI_FLASH) && defined(CONFIG_FDT_FIXUP_PARTITIONS)
        const struct node_info nodes[] = {
                { "jedec,spi-nor", MTD_DEV_TYPE_NOR },
        };
	const unsigned int bus = CONFIG_SF_DEFAULT_BUS;
	const unsigned int cs = CONFIG_SF_DEFAULT_CS;
	const unsigned int speed = CONFIG_SF_DEFAULT_SPEED;
	const unsigned int mode = CONFIG_SF_DEFAULT_MODE;
	struct udevice *new;
#endif
	int ret;

	ret = fdt_fixup_msmc_ram(blob, "/bus@100000", "sram@70000000");
	if (ret) {
		printf("%s: fixing up msmc ram failed %d\n", __func__, ret);
		return ret;
	}

#if defined(CONFIG_TI_SECURE_DEVICE)
	/* Make HW RNG reserved for secure world use */
	ret = fdt_disable_node(blob, "/bus@100000/crypto@4e00000/trng@4e10000");
	if (ret)
		printf("%s: disabling TRGN failed %d\n", __func__, ret);
#endif

#if defined(CONFIG_DM_SPI_FLASH) && defined(CONFIG_FDT_FIXUP_PARTITIONS)
	/* Update MTD partition nodes using info from mtdparts env var */
	puts("Updating MTD partitions...\n");
	spi_flash_probe_bus_cs(bus, cs, speed, mode, &new);
	fdt_fixup_mtdparts(blob, nodes, ARRAY_SIZE(nodes));
#endif

	return 0;
}
#endif /* defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP) */

#endif /* !CONFIG_SPL_BUILD */
