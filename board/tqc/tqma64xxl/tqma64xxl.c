// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Module initialization for TQ-Systems TQMa64xxL
 *
 * Copyright (C) 2017-2018 Texas Instruments Incorporated - http://www.ti.com/
 * Copyright (c) 2020-2022 TQ-Systems GmbH <license@tq-group.com>, D-82229 Seefeld, Germany.
 *
 */

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <init.h>
#include <image.h>
#include <fdt_support.h>
#include <jffs2/load_kernel.h>
#include <mtd_node.h>
#include <net.h>
#include <spi_flash.h>
#include <spl.h>
#include <stdio.h>

#include "tqma64xxl.h"
#include "../common/eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	int ret;

	ret = fdtdec_setup_mem_size_base();
	if (ret)
		printf("Error setting up mem size and base: %d\n", ret);

	return ret;
}

int dram_init_banksize(void)
{
	int ret;

	ret = fdtdec_setup_memory_banksize();
	if (ret)
		printf("Error setting up memory banksize: %d\n", ret);

	return ret;
}

#ifdef CONFIG_CPU_V7R

static int fdt_variant = -1;

struct fdt_variant_info {
	phys_size_t ram_size;
	const char *fdt_name;
};

static const struct fdt_variant_info fdt_variants[] = {
	{ 1 * SZ_1G, "k3-am642-r5-tqma64xxl-1g-mbax4xxl" },
};

static int find_fdt_variant(phys_size_t ram_size)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(fdt_variants); i++) {
		const struct fdt_variant_info *var = &fdt_variants[i];

		if (var->ram_size == ram_size)
			return i;
	}

	return -1;
}

static void print_fdt_variants(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(fdt_variants); i++) {
		const struct fdt_variant_info *variant = &fdt_variants[i];

		printf("%d) %s (", i + 1, variant->fdt_name);
		print_size(variant->ram_size, " RAM)\n");
	}
}

static int choose_fdt_variant(void)
{
	int idx;

	puts("\nWarning: Failed to find a supported RAM size in EEPROM.\n");

	/* Only one supported configuration, so we can return it immediately */
	if (ARRAY_SIZE(fdt_variants) == 1)
		return 0;

	puts("Please enter the index of the configuration to use from the following list:\n\n");

	print_fdt_variants();

	/* Flush input */
	while (tstc())
		getchar();

	puts("\nSelection: ");

	while (true) {
		idx = getchar() - '1';

		if (idx >= 0 && idx < ARRAY_SIZE(fdt_variants))
			break;
	}

	printf("%d\n\n", idx + 1);

	return idx;
}

int do_board_detect(void)
{
	struct tq_eeprom_data eeprom_data;
	phys_size_t ram_size;
	int ret;

	ret = tq_read_eeprom(0, &eeprom_data);
	if (ret) {
		printf("EEPROM: err %d\n", ret);
		goto fallback;
	}

	if (!tq_vard_valid(&eeprom_data.tq_hw_data.vard)) {
		printf("EEPROM: VARD invalid\n");
		goto fallback;
	}

	ram_size = tq_vard_ramsize(&eeprom_data.tq_hw_data.vard);
	fdt_variant = find_fdt_variant(ram_size);

fallback:
	if (fdt_variant < 0)
		fdt_variant = choose_fdt_variant();

	if (fdt_variant < 0) {
		printf("No RAM configurations found\n");
		return -EINVAL;
	}

	printf("Selected configuration for ");
	print_size(fdt_variants[fdt_variant].ram_size, " RAM\n");

	return 0;
}

int board_fit_config_name_match(const char *name)
{
	/* Board detection has not run yet */
	if (fdt_variant < 0)
		return 0;

	if (!strcmp(name, fdt_variants[fdt_variant].fdt_name))
		return 0;

	return -1;
}

#else

int board_fit_config_name_match(const char *name)
{
	return 0;
}

#endif

#ifdef CONFIG_SPL_BUILD

#define CTRLMMR_USB0_PHY_CTRL	0x43004008
#define CORE_VOLTAGE		0x80000000

void spl_board_init(void)
{
	u32 val;
	/* Set USB PHY core voltage to 0.85V */
	val = readl(CTRLMMR_USB0_PHY_CTRL);
	val &= ~(CORE_VOLTAGE);
	writel(val, CTRLMMR_USB0_PHY_CTRL);
}

static void fixup_usb_boot(struct spl_image_info *spl_image)
{
	const char *const value = "host";
	int ret;

	if (spl_boot_device() == BOOT_DEVICE_USB) {
		/*
		 * If the boot mode is host, fixup the dr_mode to host
		 * before cdns3 bind takes place
		 */
		ret = fdt_find_and_setprop(spl_image->fdt_addr,
					   "/bus@f4000/cdns-usb@f900000/usb@f400000",
					   "dr_mode", value, strlen(value) + 1, 0);
		if (ret)
			printf("%s: fdt_find_and_setprop() failed: %d\n",
			       __func__, ret);
	}
}

static void fixup_memory_node(struct spl_image_info *spl_image)
{
	u64 start[CONFIG_NR_DRAM_BANKS], size[CONFIG_NR_DRAM_BANKS];
	int bank, ret;

	dram_init_banksize();

	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		start[bank] =  gd->bd->bi_dram[bank].start;
		size[bank] = gd->bd->bi_dram[bank].size;
	}

	ret = fdt_fixup_memory_banks(spl_image->fdt_addr, start, size,
				     CONFIG_NR_DRAM_BANKS);
	if (ret)
		printf("Error fixing up memory node! %d\n", ret);
}

void spl_perform_fixups(struct spl_image_info *spl_image)
{
	fixup_memory_node(spl_image);

	if (CONFIG_IS_ENABLED(USB_STORAGE))
		fixup_usb_boot(spl_image);
}

#else /* CONFIG_SPL_BUILD */

int board_init(void)
{
	return 0;
}

/* Bootmedia handling, copied from SPL code in am642_init.c */
static u32 get_backup_bootmedia(u32 main_devstat)
{
	u32 bkup_bootmode =
	    (main_devstat & MAIN_DEVSTAT_BACKUP_BOOTMODE_MASK) >>
	    MAIN_DEVSTAT_BACKUP_BOOTMODE_SHIFT;
	u32 bkup_bootmode_cfg =
	    (main_devstat & MAIN_DEVSTAT_BACKUP_BOOTMODE_CFG_MASK) >>
	    MAIN_DEVSTAT_BACKUP_BOOTMODE_CFG_SHIFT;

	switch (bkup_bootmode) {
	case BACKUP_BOOT_DEVICE_UART:
		return BOOT_DEVICE_UART;

	case BACKUP_BOOT_DEVICE_DFU:
		if (bkup_bootmode_cfg & MAIN_DEVSTAT_BACKUP_USB_MODE_MASK)
			return BOOT_DEVICE_USB;
		return BOOT_DEVICE_DFU;

	case BACKUP_BOOT_DEVICE_ETHERNET:
		return BOOT_DEVICE_ETHERNET;

	case BACKUP_BOOT_DEVICE_MMC:
		if (bkup_bootmode_cfg)
			return BOOT_DEVICE_MMC2;
		return BOOT_DEVICE_MMC1;

	case BACKUP_BOOT_DEVICE_SPI:
		return BOOT_DEVICE_SPI;

	case BACKUP_BOOT_DEVICE_I2C:
		return BOOT_DEVICE_I2C;
	};

	return BOOT_DEVICE_RAM;
}

static u32 get_primary_bootmedia(u32 main_devstat)
{
	u32 bootmode = (main_devstat & MAIN_DEVSTAT_PRIMARY_BOOTMODE_MASK) >>
	    MAIN_DEVSTAT_PRIMARY_BOOTMODE_SHIFT;
	u32 bootmode_cfg =
	    (main_devstat & MAIN_DEVSTAT_PRIMARY_BOOTMODE_CFG_MASK) >>
	    MAIN_DEVSTAT_PRIMARY_BOOTMODE_CFG_SHIFT;

	switch (bootmode) {
	case BOOT_DEVICE_OSPI:
	case BOOT_DEVICE_QSPI:
	case BOOT_DEVICE_XSPI:
	case BOOT_DEVICE_SPI:
		return BOOT_DEVICE_SPI;

	case BOOT_DEVICE_ETHERNET_RGMII:
	case BOOT_DEVICE_ETHERNET_RMII:
		return BOOT_DEVICE_ETHERNET;

	case BOOT_DEVICE_EMMC:
		return BOOT_DEVICE_MMC1;

	case BOOT_DEVICE_NAND:
		return BOOT_DEVICE_NAND;

	case BOOT_DEVICE_MMC:
		if ((bootmode_cfg & MAIN_DEVSTAT_PRIMARY_MMC_PORT_MASK) >>
		     MAIN_DEVSTAT_PRIMARY_MMC_PORT_SHIFT)
			return BOOT_DEVICE_MMC2;
		return BOOT_DEVICE_MMC1;

	case BOOT_DEVICE_DFU:
		if ((bootmode_cfg & MAIN_DEVSTAT_PRIMARY_USB_MODE_MASK) >>
		    MAIN_DEVSTAT_PRIMARY_USB_MODE_SHIFT)
			return BOOT_DEVICE_USB;
		return BOOT_DEVICE_DFU;

	case BOOT_DEVICE_NOBOOT:
		return BOOT_DEVICE_RAM;
	}

	return bootmode;
}

u32 tqma64xxl_get_boot_device(void)
{
	u32 bootindex = *(u32 *)(CONFIG_SYS_K3_BOOT_PARAM_TABLE_INDEX);
	u32 devstat = readl(CTRLMMR_MAIN_DEVSTAT);

	if (bootindex == K3_PRIMARY_BOOTMODE)
		return get_primary_bootmedia(devstat);
	else
		return get_backup_bootmedia(devstat);
}

static void tqma64xxl_set_macaddrs(u8 *macaddr)
{
	int i;

	for (i = 1; ; i++) {
		eth_env_set_enetaddr_by_index("eth", i, macaddr);
		if (i >= 4)
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

#define CTRLMMR_WKUP_JTAG_DEVICE_ID 0x43000018

static const char *tqma64xxl_cpu_type(void)
{
	u32 device_id = readl(CTRLMMR_WKUP_JTAG_DEVICE_ID);

	switch (device_id >> 17) {
	case 0x1946:
		return "6442";
	case 0x1926:
		return "6441";
	case 0x1942:
		return "6422";
	case 0x1922:
		return "6421";
	case 0x1940:
		return "6412";
	case 0x1920:
		return "6411";
	default:
		return "64??";
	}
}

void tqma64xxl_parse_eeprom(void)
{
	char id[TQ_ID_STRLEN], serial[TQ_SERIAL_STRLEN], id_prefix[9];
	struct tq_eeprom_data data;
	u8 macaddr[6];
	int ret;

	snprintf(id_prefix, sizeof(id_prefix), "TQMa%sL", tqma64xxl_cpu_type());

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
		tqma64xxl_set_macaddrs(macaddr);

	tq_vard_show(&data.tq_hw_data.vard);
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int tqma64xxl_ft_board_setup(void *blob, struct bd_info *bd)
{
#ifdef CONFIG_DM_SPI_FLASH
#  ifdef CONFIG_FDT_FIXUP_PARTITIONS
	const struct node_info nodes[] = {
		{ "jedec,spi-nor", MTD_DEV_TYPE_NOR },
	};
#  endif
	const unsigned int bus = CONFIG_SF_DEFAULT_BUS;
	const unsigned int cs = CONFIG_SF_DEFAULT_CS;
	const unsigned int speed = CONFIG_SF_DEFAULT_SPEED;
	const unsigned int mode = CONFIG_SF_DEFAULT_MODE;
	struct udevice *new;
	int ret;

	puts("Testing for SPI-NOR flash...\n");
	ret = spi_flash_probe_bus_cs(bus, cs, speed, mode, &new);
	if (!ret) {
#  ifdef CONFIG_FDT_FIXUP_PARTITIONS
		puts("Updating MTD partitions...\n");
		/* Update MTD partition nodes using info from mtdparts env var */
		fdt_fixup_mtdparts(blob, nodes, ARRAY_SIZE(nodes));
#  endif
	} else {
		puts("No flash found.\n");
		fdt_disable_node(blob,
				 "/bus@100000/bus@28380000/fss@47000000/spi@47040000/flash@0");
	}
#endif

	return 0;
}
#endif /* defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP) */

#endif /* !CONFIG_SPL_BUILD */
