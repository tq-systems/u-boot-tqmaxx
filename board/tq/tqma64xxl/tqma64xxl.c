// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Module initialization for TQ-Systems TQMa64xxL
 *
 * Copyright (c) 2017-2018 Texas Instruments Incorporated - http://www.ti.com/
 * Copyright (c) 2020-2023 TQ-Systems GmbH <u-boot@ew.tq-group.com>, D-82229 Seefeld, Germany.
 * Author: Matthias Schiffer
 */

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <display_options.h>
#include <dm/uclass.h>
#include <fdt_support.h>
#include <init.h>
#include <k3-ddrss.h>
#include <log.h>
#include <mtd_node.h>
#include <net.h>
#include <spi_flash.h>
#include <spl.h>
#include <sysinfo/tq_eeprom.h>

#include "tqma64xxl.h"

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	return fdtdec_setup_mem_size_base_lowest();
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

#ifdef CONFIG_SPL_BUILD

#define CTRLMMR_USB0_PHY_CTRL	(WKUP_CTRL_MMR0_BASE + 0x4008)
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

static void fixup_ddr_driver_for_ecc(struct spl_image_info *spl_image)
{
	struct udevice *dev;
	int ret;

	dram_init_banksize();

	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret)
		panic("Cannot get RAM device for ddr size fixup: %d\n", ret);

	ret = k3_ddrss_ddr_fdt_fixup(dev, spl_image->fdt_addr, gd->bd);
	if (ret)
		printf("Error fixing up ddr node for ECC use! %d\n", ret);
}

static void fixup_memory_node(struct spl_image_info *spl_image)
{
	u64 start[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];
	int bank;
	int ret;

	dram_init();
	dram_init_banksize();

	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		start[bank] = gd->bd->bi_dram[bank].start;
		size[bank] = gd->bd->bi_dram[bank].size;
	}

	ret = fdt_fixup_memory_banks(spl_image->fdt_addr, start, size,
				     CONFIG_NR_DRAM_BANKS);

	if (ret)
		printf("Error fixing up memory node! %d\n", ret);
}

void spl_perform_fixups(struct spl_image_info *spl_image)
{
	if (IS_ENABLED(CONFIG_K3_INLINE_ECC))
		fixup_ddr_driver_for_ecc(spl_image);
	else
		fixup_memory_node(spl_image);

	if (CONFIG_IS_ENABLED(USB_STORAGE))
		fixup_usb_boot(spl_image);
}

#else /* CONFIG_SPL_BUILD */

int board_init(void)
{
	return 0;
}

/*
 * Bootmedia handling, copied from SPL code in am642_init.c
 *
 * Differences from original code:
 * - Special value TQMA64XXL_BOOT_DEVICE_EMMC is returned for eMMC bootpart boot
 *   to distinguish from eMMC UDA
 */
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
		fallthrough;
	case BOOT_DEVICE_QSPI:
		fallthrough;
	case BOOT_DEVICE_XSPI:
		fallthrough;
	case BOOT_DEVICE_SPI:
		return BOOT_DEVICE_SPI;

	case BOOT_DEVICE_ETHERNET_RGMII:
		fallthrough;
	case BOOT_DEVICE_ETHERNET_RMII:
		return BOOT_DEVICE_ETHERNET;

	case BOOT_DEVICE_EMMC:
		return TQMA64XXL_BOOT_DEVICE_EMMC;

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

/* Imported from k3_has_icss() in arch/arm/mach-k3/am642_fdt.c */
static int tqma64xxl_has_icss(void)
{
	u32 full_devid = readl(CTRLMMR_WKUP_JTAG_DEVICE_ID);
	u32 feature_code = (full_devid & JTAG_DEV_FEATURES_MASK) >>
			    JTAG_DEV_FEATURES_SHIFT;

	switch (feature_code) {
	case JTAG_DEV_FEATURES_D:
	case JTAG_DEV_FEATURES_E:
	case JTAG_DEV_FEATURES_F:
		return true;
	default:
		return false;
	}
}

static void tqma64xxl_set_macaddrs(u8 *macaddr)
{
	/*
	 * On variants with with ICSS/PRUs, up to 3 dual Ethernet controllers
	 * can exist, but only 5 of the 6 interfaces can be muxed at the same
	 * time. We reserve 4 additional MAC addresses, for a total of 5
	 * including the address assigned by TI.
	 *
	 * On TQMa64xxL variants without ICSS/PRUs, one dual Ethernet
	 * controller exists. We reserve 1 additional address, for a total of 2.
	 */
	const int num_addrs = tqma64xxl_has_icss() ? 4 : 1;
	int i;

	for (i = 1; ; i++) {
		eth_env_set_enetaddr_by_index("eth", i, macaddr);
		if (i >= num_addrs)
			break;

		if (++macaddr[5])
			continue;
		if (++macaddr[4])
			continue;
		if (++macaddr[3])
			continue;

		printf("Warning: End of MAC address block\n");
		break;
	}
}

#define CTRLMMR_WKUP_JTAG_USER_ID (WKUP_CTRL_MMR0_BASE + 0x18)

static const char *tqma64xxl_cpu_type(void)
{
	u32 device_id = readl(CTRLMMR_WKUP_JTAG_DEVICE_ID);

	switch ((device_id & JTAG_DEV_ID_MASK) >> JTAG_DEV_ID_SHIFT) {
	case JTAG_DEV_ID_AM6442:
		return "6442";
	case JTAG_DEV_ID_AM6441:
		return "6441";
	case JTAG_DEV_ID_AM6422:
		return "6422";
	case JTAG_DEV_ID_AM6421:
		return "6421";
	case JTAG_DEV_ID_AM6412:
		return "6412";
	case JTAG_DEV_ID_AM6411:
		return "6411";
	default:
		return "64??";
	}
}

int show_board_info(void)
{
	const char *model = fdt_getprop(gd->fdt_blob, 0, "model", NULL);

	printf("SoC variant: AM%s\n", tqma64xxl_cpu_type());
	if (model)
		printf("Model: %s\n", model);

	return 0;
}

void tqma64xxl_setup_sysinfo(void)
{
	struct udevice *sysinfo;
	char buf[80] = "", macaddr[ETH_ALEN];
	int ret;

	ret = sysinfo_get_and_detect(&sysinfo);
	if (ret) {
		pr_err("Failed to get sysinfo data: %d\n", ret);
		return;
	}

	if (!sysinfo_get_str(sysinfo, SYSINFO_ID_TQ_MODEL, sizeof(buf), buf))
		env_set_runtime("boardtype", buf);

	if (!sysinfo_get_str(sysinfo, SYSINFO_ID_TQ_SERIAL, sizeof(buf), buf))
		env_set_runtime("serial#", buf);

	if (!sysinfo_get_binary(sysinfo, SYSINFO_ID_TQ_MAC_ADDR, sizeof(macaddr), macaddr))
		tqma64xxl_set_macaddrs(macaddr);
}

#if IS_ENABLED(CONFIG_OF_LIBFDT) && IS_ENABLED(CONFIG_OF_BOARD_SETUP)
int tqma64xxl_ft_board_setup(void *blob, struct bd_info *bd)
{
#if IS_ENABLED(CONFIG_DM_SPI_FLASH)
	const struct node_info nodes[] = {
		{ "jedec,spi-nor", MTD_DEV_TYPE_NOR },
	};
	struct udevice *new;
	int ret;

	puts("Testing for SPI-NOR flash...\n");
	ret = spi_flash_probe_bus_cs(CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS, &new);
	if (!ret) {
		if (IS_ENABLED(CONFIG_FDT_FIXUP_PARTITIONS)) {
			puts("Updating MTD partitions...\n");
			/* Update MTD partition nodes using info from mtdparts env var */
			fdt_fixup_mtdparts(blob, nodes, ARRAY_SIZE(nodes));
		}
	} else {
		puts("No flash found.\n");
		fdt_status_disabled_by_pathf(blob,
					     "/bus@f4000/bus@fc00000/spi@fc40000/flash@0");
	}
#endif

	return 0;
}
#endif /* IS_ENABLED(CONFIG_OF_LIBFDT) && IS_ENABLED(CONFIG_OF_BOARD_SETUP) */

#endif /* !CONFIG_SPL_BUILD */
