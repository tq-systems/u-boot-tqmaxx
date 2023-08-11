// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Module initialization for TQ-Systems TQMa62xx
 *
 * Copyright (c) 2020-2022 Texas Instruments Incorporated - https://www.ti.com/
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

#include "tqma62xx.h"

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	u64 bank0_size, bank1_size = 0;
	struct udevice *sysinfo;
	int ret;

	if (!IS_ENABLED(CONFIG_CPU_V7R))
		return fdtdec_setup_memory_banksize();

	ret = sysinfo_get_and_detect(&sysinfo);
	if (ret) {
		printf("Failed to get sysinfo data: %d\n", ret);
		return ret;
	}

	ret = sysinfo_get_uint64(sysinfo, SYSINFO_ID_RAM_SIZE, &bank0_size);
	if (ret) {
		printf("Failed to get RAM size: %d\n", ret);
		return ret;
	}

	if (bank0_size > SZ_2G) {
		bank1_size = bank0_size - SZ_2G;
		bank0_size = SZ_2G;
	}

	gd->bd->bi_dram[0].start = CFG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = bank0_size;

	if (CONFIG_NR_DRAM_BANKS > 1) {
		gd->bd->bi_dram[1].start = CFG_SYS_SDRAM_BASE1;
		gd->bd->bi_dram[1].size = bank1_size;
	}

	return 0;
}

#ifdef CONFIG_SPL_BUILD

/*
 * - R5 SPL uses this to patch its own FDT
 * - Both SPLs patch the FDTs of subsequent stages
 */
static void fixup_usb_boot(void *fdt)
{
	const char *const value = "host";
	int ret;

	if (spl_boot_device() == BOOT_DEVICE_USB) {
		/*
		 * If the boot mode is host, fixup the dr_mode to host
		 * before dwc3 bind takes place
		 */
		ret = fdt_find_and_setprop(fdt,
					   "/bus@f0000/dwc3-usb@f900000/usb@31000000",
					   "dr_mode", value, strlen(value) + 1, 0);
		if (ret)
			printf("%s: fdt_find_and_setprop() failed: %d\n",
			       __func__, ret);
	}
}

#ifdef CONFIG_CPU_V7R
int fdtdec_board_setup(const void *fdt_blob)
{
	fixup_usb_boot((void *)fdt_blob);
	return 0;
}
#endif

#define CTRLMMR_USB0_PHY_CTRL	(WKUP_CTRL_MMR0_BASE + 0x4008)
#define CTRLMMR_USB1_PHY_CTRL	(WKUP_CTRL_MMR0_BASE + 0x4018)
#define CORE_VOLTAGE		0x80000000

#define WKUP_CTRLMMR_DBOUNCE_CFG1 0x04504084
#define WKUP_CTRLMMR_DBOUNCE_CFG2 0x04504088
#define WKUP_CTRLMMR_DBOUNCE_CFG3 0x0450408c
#define WKUP_CTRLMMR_DBOUNCE_CFG4 0x04504090
#define WKUP_CTRLMMR_DBOUNCE_CFG5 0x04504094
#define WKUP_CTRLMMR_DBOUNCE_CFG6 0x04504098

void spl_board_init(void)
{
	u32 val;

	/* Set USB0 PHY core voltage to 0.75V */
	val = readl(CTRLMMR_USB0_PHY_CTRL);
	val |= CORE_VOLTAGE;
	writel(val, CTRLMMR_USB0_PHY_CTRL);

	/* Set USB1 PHY core voltage to 0.75V */
	val = readl(CTRLMMR_USB1_PHY_CTRL);
	val |= CORE_VOLTAGE;
	writel(val, CTRLMMR_USB1_PHY_CTRL);

	/* We have 32k crystal, so lets enable it */
	val = readl(MCU_CTRL_LFXOSC_CTRL);
	val &= ~(MCU_CTRL_LFXOSC_32K_DISABLE_VAL);
	writel(val, MCU_CTRL_LFXOSC_CTRL);
	/* Add any TRIM needed for the crystal here.. */
	/* Make sure to mux up to take the SoC 32k from the crystal */
	writel(MCU_CTRL_DEVICE_CLKOUT_LFOSC_SELECT_VAL,
	       MCU_CTRL_DEVICE_CLKOUT_32K_CTRL);

	/* Setup debounce conf registers - arbitrary values. Times are approx */
	/* 1.9ms debounce @ 32k */
	writel(WKUP_CTRLMMR_DBOUNCE_CFG1, 0x1);
	/* 5ms debounce @ 32k */
	writel(WKUP_CTRLMMR_DBOUNCE_CFG2, 0x5);
	/* 20ms debounce @ 32k */
	writel(WKUP_CTRLMMR_DBOUNCE_CFG3, 0x14);
	/* 46ms debounce @ 32k */
	writel(WKUP_CTRLMMR_DBOUNCE_CFG4, 0x18);
	/* 100ms debounce @ 32k */
	writel(WKUP_CTRLMMR_DBOUNCE_CFG5, 0x1c);
	/* 156ms debounce @ 32k */
	writel(WKUP_CTRLMMR_DBOUNCE_CFG6, 0x1f);
}

#if defined(CONFIG_K3_AM64_DDRSS)
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
#else
static void fixup_memory_node(struct spl_image_info *spl_image)
{
	u64 start[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];
	int bank;
	int ret;

	dram_init();
	dram_init_banksize();

	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		start[bank] =  gd->bd->bi_dram[bank].start;
		size[bank] = gd->bd->bi_dram[bank].size;
	}

	/* dram_init functions use SPL fdt, and we must fixup u-boot fdt */
	ret = fdt_fixup_memory_banks(spl_image->fdt_addr, start, size,
				     CONFIG_NR_DRAM_BANKS);
	if (ret)
		printf("Error fixing up memory node! %d\n", ret);
}
#endif

void spl_perform_fixups(struct spl_image_info *spl_image)
{
#if defined(CONFIG_K3_AM64_DDRSS)
	fixup_ddr_driver_for_ecc(spl_image);
#else
	fixup_memory_node(spl_image);
#endif

	if (CONFIG_IS_ENABLED(USB_STORAGE))
		fixup_usb_boot(spl_image->fdt_addr);
}

#else /* CONFIG_SPL_BUILD */

int board_init(void)
{
	return 0;
}

/*
 * Bootmedia handling, copied from SPL code in am625_init.c
 *
 * Differences from original code:
 * - Special value TQMA62XX_BOOT_DEVICE_EMMC is returned for eMMC bootpart boot
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
		return TQMA62XX_BOOT_DEVICE_EMMC;

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

u32 tqma62xx_get_boot_device(void)
{
	u32 bootindex = *(u32 *)(CONFIG_SYS_K3_BOOT_PARAM_TABLE_INDEX);
	u32 devstat = readl(CTRLMMR_MAIN_DEVSTAT);

	if (bootindex == K3_PRIMARY_BOOTMODE)
		return get_primary_bootmedia(devstat);
	else
		return get_backup_bootmedia(devstat);
}

static const char *tqma62xx_cpu_type(void)
{
	u32 device_id = readl(CTRLMMR_WKUP_JTAG_DEVICE_ID);

	switch ((device_id & JTAG_DEV_ID_MASK) >> JTAG_DEV_ID_SHIFT) {
	case JTAG_DEV_ID_AM6254:
		return "6254";
	case JTAG_DEV_ID_AM6252:
		return "6252";
	case JTAG_DEV_ID_AM6251:
		return "6251";
	case JTAG_DEV_ID_AM6234:
		return "6234";
	case JTAG_DEV_ID_AM6232:
		return "6232";
	case JTAG_DEV_ID_AM6231:
		return "6231";
	default:
		return "62??";
	}
}

int checkboard(void)
{
	printf("SoC variant: AM%s\n", tqma62xx_cpu_type());
	return 0;
}

void tqma62xx_setup_sysinfo(void)
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
		eth_env_set_enetaddr_by_index("eth", 1, macaddr);
}

#if IS_ENABLED(CONFIG_OF_LIBFDT) && IS_ENABLED(CONFIG_OF_BOARD_SETUP)
int tqma62xx_ft_board_setup(void *blob, struct bd_info *bd)
{
#ifdef CONFIG_DM_SPI_FLASH
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
					     "/bus@f0000/bus@fc00000/spi@fc40000/flash@0");
	}
#endif

	return 0;
}
#endif /* IS_ENABLED(CONFIG_OF_LIBFDT) && IS_ENABLED(CONFIG_OF_BOARD_SETUP) */

#endif /* !CONFIG_SPL_BUILD */
